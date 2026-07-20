#include <TTreeOut.h>

#include <TCorrelator.h>
#include <Histogramer.h>

#include <TFile.h>
#include <TTree.h>
#include <TCutG.h>

#include <TFDSi.h>
#include <TPID.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <thread>
#include <vector>

#include <globals.h>

TTreeOut* TTreeOut::fTreeOut = 0;
std::mutex TTreeOut::fTreeOutMutex;

bool TTreeOut::fNoTree = false;

int TTreeOut::fRun = 0;
int TTreeOut::fSubRun = 0;

void TTreeOut::SetRun(int run, int subrun) {
  fRun = run;
  fSubRun = subrun;
}

TTreeOut::TTreeOut() : fIn(0), fOut(0), fFillCounter(0) { }

TTreeOut::~TTreeOut() { }

TTreeOut* TTreeOut::Get() {
  if(!fTreeOut) {
    std::lock_guard<std::mutex> lock(fTreeOutMutex);
    fTreeOut = new TTreeOut();
  }
  return fTreeOut;
}

bool sortTime(TImplant one, TImplant two) {
  return one.timestamp > two.timestamp;
}

void TTreeOut::TreeLoop() {
  fLoopRunning = true;

  TFDSi* fdsi = 0;
  std::vector<TImplant>* implants = 0;

  TDirectory* current = gDirectory;
  TFile* outfile = 0;
  TTree* tree = 0;

  if(!fNoTree) {
    outfile = new TFile(Form("beta%04i-%02i.root", fRun, fSubRun), "recreate");
    tree = new TTree("beta", "beta-tree");

    tree->Branch("TFDSi", &fdsi);
    tree->Branch("Implants", &implants);
  }

  while(true) {
    if(!TCorrelator::Get()->LoopRunning() && TCorrelator::Get()->qsize() == 0)
      break;

    std::pair<TFDSi, std::vector<TImplant>> temp;
    if(!TCorrelator::Get()->pop(temp)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }
    fIn++;

    if(temp.first.fEventType < 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      continue;
    }

    std::sort(temp.second.begin(), temp.second.end(), sortTime);

    MakeHistograms(temp.first, temp.second);

    if(!fNoTree) {
      std::lock_guard<std::mutex> lock(fTreeOutMutex);
      TFDSi tempFDSi = temp.first;
      tempFDSi.Copy(*fdsi);

      *implants = temp.second;

      tree->Fill();
      fFillCounter++;
    }
    fOut++;
  }

  if(!fNoTree) {
    tree->Write();
    outfile->Close();
    current->cd();
  }
  fLoopRunning = false;
}

std::string TTreeOut::Status() {
  std::lock_guard<std::mutex> lock(fTreeOutMutex);
  std::string s = Form("TreeOut: in[%lu]  out[%lu]  filled[%lu]", fIn, fOut, fFillCounter);
  return s;
}

TCutG* neutron = 0;

void TTreeOut::MakeHistograms(TFDSi& fdsi,
                              std::vector<TImplant>& implants) const {

  if(!Histogramer::Get()->GetBlobs()) {
    printf("CUTS:   %s\n\n\n\n\n\n",
           Form("%s/gates/myPid_de2.cuts", getenv("BSYS")));
    Histogramer::Get()->SetBlobGates(
        Form("%s/gates/myPid_de2.cuts", getenv("BSYS")));
    Histogramer::Get()->SetGammaPrompt(
        Form("%s/gates/gtime.cuts", getenv("BSYS")));
  }

  if(!neutron) {
    TFile* f = TFile::Open("gates/neutron.cuts");
    neutron = static_cast<TCutG*>(f->Get("neutron"));
  }

  Histogramer::fill("runtime", 3600, 0, 3600000,
                    fdsi.fClock.initial / 1.e6);

  if(fdsi.fEventType == 4) { // implant

    Histogramer::fill("PID",
                      500, -180, -130, fdsi.GetTOF(),
                      1500, 1000, 2500, fdsi.fPID.de2);

    Histogramer::fill("tof_time",
                      720, 0, 7200, fdsi.fClock.initial / 1.e9,
                      500, -180, -130, fdsi.GetTOF());

    Histogramer::fill("implantX",
                      2000, 0, 48, fdsi.fLowGain1.xpos,
                      4000, 0, 64000, fdsi.fLowGain1.dyenergy);

    Histogramer::fill("implantY",
                      2000, 0, 48, fdsi.fLowGain1.ypos,
                      4000, 0, 64000, fdsi.fLowGain1.dyenergy);

    return;
  }

  if(fdsi.fEventType != 12) // not a decay
    return;

  Histogramer::fill("decayX",
                    2000, 0, 48, fdsi.fLowGain1.xpos,
                    4000, 0, 64000, fdsi.fLowGain1.dyenergy);

  Histogramer::fill("decayY",
                    2000, 0, 48, fdsi.fLowGain1.ypos,
                    4000, 0, 64000, fdsi.fLowGain1.dyenergy);

  // Ungated decay-event gamma-ray spectra. These are filled once per decay.
  for(const auto& hit : fdsi.fClover.hits) {
    const double gdt = fdsi.fLowGain1.dytime - hit.fTime;

    Histogramer::fill("decayEvent", "gtime",
                      500, -2000, 2000, gdt,
                      1000, 0, 4000, hit.fEcal);

    Histogramer::fill("decayEvent", "gsummary",
                      8000, 0, 4000, hit.fEcal,
                      70, 0, 70, hit.fId);
  }

  int nmult = 0;
  for(const auto& hit : fdsi.fVandle.fHits) {
    if(neutron &&
       neutron->IsInside(hit.fTimeLeft - fdsi.fLowGain1.dytime,
                         hit.GetQDC())) {
      nmult++;
    }
  }
  Histogramer::fill("nmult", 100, 0, 100, nmult);

  /*
   * For each PID gate:
   *
   *   1. Build the list of acceptable correlated implants in that gate.
   *   2. Fill gamma/addback singles and timing spectra once per decay.
   *   3. Fill decay-time-dependent spectra once for every possible
   *      implant-decay pair.
   *
   * This prevents gsummary/asummary/gtime/atime from being multiplied when
   * more than one correlated implant passes the same PID gate.
   */
  TIter iter(Histogramer::Get()->GetBlobs());
  while(TCutG* blob = static_cast<TCutG*>(iter.Next())) {
    std::vector<const TImplant*> matchingImplants;
    matchingImplants.reserve(implants.size());

    for(const auto& implant : implants) {
      if(implant.fom > 4.01)
        continue;

      if(blob->IsInside(implant.tof, implant.de2))
        matchingImplants.push_back(&implant);
    }

    if(matchingImplants.empty())
      continue;

    const char* gateName = blob->GetName();

    // Diagnostic: number of acceptable implants for this decay and PID gate.
    Histogramer::fill(gateName, "implantMultiplicity",
                      20, 0, 20, matchingImplants.size());

    /*
     * Event-level crystal spectra.
     * These quantities do not depend on which implant is selected, so each
     * gamma ray is filled only once for this decay and PID gate.
     */
    for(const auto& hit : fdsi.fClover.hits) {
      const double gdt = fdsi.fLowGain1.dytime - hit.fTime;

      Histogramer::fill(gateName, "gtime",
                        500, -2000, 2000, gdt,
                        1000, 0, 4000, hit.fEcal);

      if(gdt < 200.0 || gdt > 500.0)
        continue;

      Histogramer::fill(gateName, "gsummary",
                        16000, 0, 8000, hit.fEcal,
                        70, 0, 70, hit.fId);
    }

    /*
     * Event-level addback spectra.
     * These are also filled only once for this decay and PID gate.
     */
    for(const auto& hit : fdsi.fClover.addbackHits) {
      const double gdt = fdsi.fLowGain1.dytime - hit.fTime;

      Histogramer::fill(gateName, "atime",
                        500, -2000, 2000, gdt,
                        1000, 0, 4000, hit.fEcal);

      if(gdt < 200.0 || gdt > 500.0)
        continue;

      Histogramer::fill(gateName, "asummary",
                        16000, 0, 8000, hit.fEcal,
                        20, 0, 20, hit.fId);
    }

    /*
     * Pair-level spectra.
     * Each candidate implant has a different decay time, so retain every
     * acceptable implant-decay pair here.
     */
    bool first = true;
    for(const TImplant* implant : matchingImplants) {
      const double dtime =
          fdsi.fClock.initial / 1.e6 - implant->mtime();

      Histogramer::fill(gateName, "dtimeOnly",
                        2000, -1000, 5000, dtime);
      if(first) 
        Histogramer::fill(gateName, "dtimeOnlyi_first",
                                     2000, -1000, 5000, dtime);


      for(const auto& hit : fdsi.fClover.hits) {
        const double gdt = fdsi.fLowGain1.dytime - hit.fTime;

        if(gdt < 200.0 || gdt > 500.0)
          continue;

        Histogramer::fill(gateName, "gdtime",
                          6000, -1000, 5000, dtime,
                          8000, 0, 8000, hit.fEcal);
        if(first)
          Histogramer::fill(gateName, "gdtime_first",
                                      6000, -1000, 5000, dtime,
                                      8000, 0, 8000, hit.fEcal);

        if(nmult > 0) {
          Histogramer::fill(gateName, "gdtimeAN",
                            6000, -1000, 5000, dtime,
                            8000, 0, 8000, hit.fEcal);
          if(first)
          Histogramer::fill(gateName, "gdtimeAN_first",
                                        6000, -1000, 5000, dtime,
                                        8000, 0, 8000, hit.fEcal);
        }

        if((dtime > 0.0 && dtime < 250.0) ||
           (dtime > 2000.0 && dtime < 2500.0)) {
          for(const auto& hit1 : fdsi.fClover.hits) {
            if(&hit == &hit1)
              continue;

            const double gdt1 = fdsi.fLowGain1.dytime - hit1.fTime;
            if(!Histogramer::Get()->GetGammaPrompt()->IsInside(gdt1,
                                                                hit1.fEcal)) {
              continue;
            }

            if(dtime > 0.0 && dtime < 250.0) {
              Histogramer::fill(gateName, "gg_0_250",
                                8000, 0, 8000, hit.fEcal,
                                8000, 0, 8000, hit1.fEcal);
              if(first)
              Histogramer::fill(gateName, "gg_0_250_first",
                                8000, 0, 8000, hit.fEcal,
                                8000, 0, 8000, hit1.fEcal);
            }

            if(dtime > 2000.0 && dtime < 2500.0) {
              Histogramer::fill(gateName, "gg_2000_2500",
                                8000, 0, 8000, hit.fEcal,
                                8000, 0, 8000, hit1.fEcal);
              if(first)
              Histogramer::fill(gateName, "gg_2000_2500",
                                8000, 0, 8000, hit.fEcal,
                                8000, 0, 8000, hit1.fEcal);
            }
          }
        }
      }

      for(const auto& hit : fdsi.fClover.addbackHits) {
        const double gdt = fdsi.fLowGain1.dytime - hit.fTime;

        if(gdt < 200.0 || gdt > 500.0)
          continue;

        Histogramer::fill(gateName, "adtime",
                          6000, -1000, 5000, dtime,
                          8000, 0, 8000, hit.fEcal);
        if(first)
        Histogramer::fill(gateName, "adtime_first",
                          6000, -1000, 5000, dtime,
                          8000, 0, 8000, hit.fEcal);

        if(nmult > 0) {
          Histogramer::fill(gateName, "adtimeAN",
                            6000, -1000, 5000, dtime,
                            8000, 0, 8000, hit.fEcal);
          Histogramer::fill(gateName, "adtimeAN_first",
                            6000, -1000, 5000, dtime,
                            8000, 0, 8000, hit.fEcal);
        }

        if((dtime > 0.0 && dtime < 250.0) ||
           (dtime > 2000.0 && dtime < 2500.0)) {
          for(const auto& hit1 : fdsi.fClover.addbackHits) {
            if(&hit == &hit1)
              continue;

            if(std::abs(hit.fTime - hit1.fTime) > 200.0)
              continue;

            if(dtime > 0.0 && dtime < 250.0) {
              Histogramer::fill(gateName, "aa_0_250",
                                8000, 0, 8000, hit.fEcal,
                                8000, 0, 8000, hit1.fEcal);
              if(first)
              Histogramer::fill(gateName, "aa_0_250",
                                8000, 0, 8000, hit.fEcal,
                                8000, 0, 8000, hit1.fEcal);
            }

            if(dtime > 2000.0 && dtime < 2500.0) {
              Histogramer::fill(gateName, "aa_2000_2500",
                                8000, 0, 8000, hit.fEcal,
                                8000, 0, 8000, hit1.fEcal);
              if(first)
              Histogramer::fill(gateName, "aa_2000_2500",
                                8000, 0, 8000, hit.fEcal,
                                8000, 0, 8000, hit1.fEcal);
            }
          }
        }
      }
      first = false;
    }
  }
}


