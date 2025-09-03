
#include <TTreeOut.h>

#include <TCorrelator.h>
#include <Histogramer.h>

#include <TFile.h>
#include <TTree.h>
#include <TCutG.h>

#include <TFDSi.h>
#include <TPID.h>

#include <chrono>
#include <thread>

TTreeOut* TTreeOut::fTreeOut = 0;  
std::mutex TTreeOut::fTreeOutMutex;

bool TTreeOut::fNoTree = false;

int TTreeOut::fRun = 0;
int TTreeOut::fSubRun = 0;

void TTreeOut::SetRun(int run,int subrun) {
  fRun = run;
  fSubRun = subrun;
}

TTreeOut::TTreeOut() : fFillCounter(0) { }

TTreeOut::~TTreeOut() { }

TTreeOut *TTreeOut::Get() { 
  if(!fTreeOut) {
    std::lock_guard<std::mutex> lock(fTreeOutMutex);
    fTreeOut = new TTreeOut();
  }
  return fTreeOut;
}

bool sortTime(TPID one, TPID two){return one.time > two.time;}

void TTreeOut::TreeLoop() {
  fLoopRunning = true;

  TFDSi *fdsi = 0;
  std::vector<TPID> *implants =0;

  TDirectory *current = gDirectory;
  TFile *outfile = 0;
  TTree *tree    = 0;

  if(!fNoTree) {
    outfile = new TFile(Form("beta%04i-%02i.root",fRun,fSubRun),"recreate");
    tree    = new TTree("beta","beta-tree");

    tree->Branch("TFDSi",&fdsi);
    tree->Branch("Implants",&implants);
  }

  while(true) { 
    if(!TCorrelator::Get()->LoopRunning() &&  TCorrelator::Get()->qsize()==0) 
      break;
    std::pair<TFDSi,std::vector<TPID> > temp = TCorrelator::Get()->pop();
    fIn++;

    if(temp.first.fEventType<0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      continue;
    }
    //std::sort(implants->begin(),implants->end(),sortTime);
    std::sort(temp.second.begin(),temp.second.end(),sortTime);

    MakeHistograms(temp.first,temp.second);

    if(!fNoTree) {
      std::lock_guard<std::mutex> lock(fTreeOutMutex);
      TFDSi tempFDSi = temp.first;
      tempFDSi.Copy(*fdsi);  //copies into fdsi

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
  std::string s = Form("TreeOut: in[%lu]  out[%lu]  filled[%lu]",fIn,fOut,fFillCounter);
  return s;
}

TCutG *neutron = 0;


void TTreeOut::MakeHistograms(TFDSi& fdsi,std::vector<TPID>& implants) const { 

  if(!Histogramer::Get()->GetBlobs()) {
    printf("CUTS:   %s\n\n\n\n\n\n",Form("%s/gates/myPid_de2.cuts",getenv("BSYS")));
    Histogramer::Get()->SetBlobGates(Form("%s/gates/myPid_de2.cuts",getenv("BSYS")));
  }

  if(!neutron) {
    TFile *f = TFile::Open("gates/neutron.cuts");
    neutron = (TCutG*)f->Get("neutron");
  }

  Histogramer::fill("runtime",3600,0,3600000,fdsi.fClock.initial/1e6);

  if(fdsi.fEventType==4) { //implant


    Histogramer::fill("PID",500 ,-180,-130,fdsi.GetTOF(),
        1500,1000,2500,fdsi.fPID.de2);

    Histogramer::fill("tof_time",720,0,7200,fdsi.fClock.initial/1e9,
        500,-180,-130,fdsi.GetTOF());   

    Histogramer::fill("tof_time_uncorrected",720,0,7200,fdsi.fClock.initial/1e9,
        500,-180,-130,fdsi.fPID.tof);   


    Histogramer::fill("de2_time",720,0,7200,fdsi.fClock.initial/1e9,
        500,-180,-130,fdsi.fPID.de2);   

    double dxpos = (fdsi.fLowGain1.xpos);
    double dypos = (fdsi.fLowGain1.ypos);

    Histogramer::fill("implantX",2000,0,48,fdsi.fLowGain1.xpos,4000,0,64000,fdsi.fLowGain1.dyenergy);
    Histogramer::fill("implantY",2000,0,48,fdsi.fLowGain1.ypos,4000,0,64000,fdsi.fLowGain1.dyenergy);

  } else if(fdsi.fEventType==12) { //decay
    double dxpos = (fdsi.fLowGain1.xpos);
    double dypos = (fdsi.fLowGain1.ypos);

    Histogramer::fill("decayX",2000,0,48,fdsi.fLowGain1.xpos,4000,0,64000,fdsi.fLowGain1.dyenergy);
    Histogramer::fill("decayY",2000,0,48,fdsi.fLowGain1.ypos,4000,0,64000,fdsi.fLowGain1.dyenergy);

    for(int y=0;y<fdsi.fClover.hits.size();y++) {
      TCloverHit hit = fdsi.fClover.hits.at(y);
      Histogramer::fill("gtime",500,-2000,2000,fdsi.fLowGain1.dytime - hit.fTime,
          1000,0,4000,hit.fEcal);

      Histogramer::fill("gsummary",8000,0,4000,hit.fEcal,
          70,0,70,hit.fId);
    }

    int nmult =0;

    for(int y=0;y<fdsi.fVandle.fHits.size();y++) {
      TVandleHit hit = fdsi.fVandle.fHits.at(y);
      Histogramer::fill("vsummaryRight",4000,0,4000,hit.fEnergyRight,
                                       200,0,200,hit.fId);
      Histogramer::fill("vsummaryLeft",4000,0,4000,hit.fEnergyLeft,
                                       200,0,200,hit.fId);
      Histogramer::fill("vTDiff",400,-2000,2000,hit.fTimeLeft - hit.fTimeRight,
                                       200,0,200,hit.fId);
      Histogramer::fill("vTOF",400,0,3200,hit.fTimeRight - fdsi.fLowGain1.dytime,
                               4000,0,4000,hit.fEnergyLeft + hit.fEnergyRight);
      
      Histogramer::fill("vTOFR",400,0,3200,hit.fTimeRight - fdsi.fLowGain1.dytime,
                               4000,0,4000,hit.GetQDC());
      Histogramer::fill("vTOFL",400,0,3200,hit.fTimeLeft - fdsi.fLowGain1.dytime,
                               4000,0,4000,hit.GetQDC());


      if(neutron && neutron->IsInside(hit.fTimeLeft - fdsi.fLowGain1.dytime,hit.GetQDC())) {
        nmult++;
      }
/*
      double dt = fdsi.fLowGain1.dytime - hit.fTimeRight;
      for(size_t z=0;z<hit.fTrace.size();z++) {
        Histogramer::fill("vTrace",150,0,150,z,
                                   4000,0,0,hit.fTrace.at(z));
        if(dt<20 && dt>-20)
          Histogramer::fill("vTrace_g",150,0,150,z,
                                     4000,0,0,hit.fTrace.at(z));
        if(dt<-50)
          Histogramer::fill("vTrace_n",150,0,150,z,
                                     4000,0,0,hit.fTrace.at(z));
        if(dt>50)
          Histogramer::fill("vTrace_bg",150,0,150,z,
                                     4000,0,0,hit.fTrace.at(z));
      }
*/      
    }

    Histogramer::fill("nmult",100,0,100,nmult);

    TIter iter(Histogramer::Get()->GetBlobs());
    while(TCutG* blob = (TCutG*)iter.Next()) {
      for(int z=0;z<int(implants.size());z++) {
        bool first = true;
        if(blob->IsInside(implants.at(z).tof,implants.at(z).de2)) { 
          double dtime = (fdsi.fClock.initial/1.e6) - implants.at(z).time;
          Histogramer::fill(blob->GetName(),"dtimeOnly",2000,-1000,1000,dtime);
          for(int y=0;y<fdsi.fClover.hits.size();y++) {
            TCloverHit hit = fdsi.fClover.hits.at(y);
            Histogramer::fill(blob->GetName(),"gtime",500,-2000,2000,fdsi.fLowGain1.dytime - hit.fTime,
                1000,0,4000,hit.fEcal);

            Histogramer::fill(blob->GetName(),"gsummary",8000,0,4000,hit.fEcal,
                70,0,70,hit.fId);
            Histogramer::fill(blob->GetName(),"dtime",2000,-1000,1000,dtime,
                                                              4000,0,4000,hit.fEcal);
            if(nmult==1) 
              Histogramer::fill(blob->GetName(),"dtime1N",2000,-1000,1000,dtime,
                                                              4000,0,4000,hit.fEcal);
            if(nmult==2) 
              Histogramer::fill(blob->GetName(),"dtime2N",2000,-1000,1000,dtime,
                                                              4000,0,4000,hit.fEcal);
            if(nmult>0) 
              Histogramer::fill(blob->GetName(),"dtimeAN",2000,-1000,1000,dtime,
                                                              4000,0,4000,hit.fEcal);

            if(first) 
               Histogramer::fill(blob->GetName(),"dtime_0",2000,-1000,1000,dtime,
                                                                 4000,0,4000,hit.fEcal);

          }

          first=false;
        }
      }
    }

  }

}


