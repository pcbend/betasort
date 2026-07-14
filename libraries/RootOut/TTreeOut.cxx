
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

#include <globals.h>

TTreeOut* TTreeOut::fTreeOut = 0;  
std::mutex TTreeOut::fTreeOutMutex;

bool TTreeOut::fNoTree = false;

int TTreeOut::fRun = 0;
int TTreeOut::fSubRun = 0;

void TTreeOut::SetRun(int run,int subrun) {
  fRun = run;
  fSubRun = subrun;
}

TTreeOut::TTreeOut() : fIn(0),fOut(0), fFillCounter(0) { }

TTreeOut::~TTreeOut() { }

TTreeOut *TTreeOut::Get() { 
  if(!fTreeOut) {
    std::lock_guard<std::mutex> lock(fTreeOutMutex);
    fTreeOut = new TTreeOut();
  }
  return fTreeOut;
}

bool sortTime(TImplant one, TImplant two){return one.timestamp > two.timestamp;}

void TTreeOut::TreeLoop() {
  fLoopRunning = true;

  TFDSi *fdsi = 0;
  std::vector<TImplant> *implants =0;

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
    std::pair<TFDSi,std::vector<TImplant> > temp;
    if(!TCorrelator::Get()->pop(temp))  {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }
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


void TTreeOut::MakeHistograms(TFDSi& fdsi,std::vector<TImplant>& implants) const { 

  if(!Histogramer::Get()->GetBlobs()) {
    printf("CUTS:   %s\n\n\n\n\n\n",Form("%s/gates/myPid_de2.cuts",getenv("BSYS")));
    Histogramer::Get()->SetBlobGates(Form("%s/gates/myPid_de2.cuts",getenv("BSYS")));
    Histogramer::Get()->SetGammaPrompt(Form("%s/gates/gtime.cuts",getenv("BSYS")));
  }

  if(!neutron) {
    TFile *f = TFile::Open("gates/neutron.cuts");
    neutron = (TCutG*)f->Get("neutron");
  }

  Histogramer::fill("runtime",3600,0,3600000,fdsi.fClock.initial/1e6);

  if(fdsi.fEventType==4) { //implant


    Histogramer::fill("PID",500 ,-180,-130,fdsi.GetTOF(),
                            1500,1000,2500,fdsi.fPID.de2);

    Histogramer::fill("tof_time",720,   0, 7200,fdsi.fClock.initial/1e9,
                                 500,-180, -130,fdsi.GetTOF());   

    //Histogramer::fill("tof_time_uncorrected",720,0,7200,fdsi.fClock.initial/1e9,
    //                                         500,-180,-130,fdsi.fPID.tof);   

    //Histogramer::fill("de2_time",720,0,7200,fdsi.fClock.initial/1e9,
    //                             500,-180,-130,fdsi.fPID.de2);   

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
      Histogramer::fill("decayEvent","gtime",500,-2000,2000,fdsi.fLowGain1.dytime - hit.fTime,
                                             1000,0,4000,hit.fEcal);

      Histogramer::fill("decayEvent","gsummary",8000,0,4000,hit.fEcal,
                                                70,0,70,hit.fId);
    }

    int nmult =0;
    for(int y=0;y<fdsi.fVandle.fHits.size();y++) {
      TVandleHit hit = fdsi.fVandle.fHits.at(y);
      //Histogramer::fill("vsummaryRight",4000,0,4000,hit.fEnergyRight,
      //                                 200,0,200,hit.fId);
      //Histogramer::fill("vsummaryLeft",4000,0,4000,hit.fEnergyLeft,
      //                                 200,0,200,hit.fId);
      //Histogramer::fill("vTDiff",400,-2000,2000,hit.fTimeLeft - hit.fTimeRight,
      //                                 200,0,200,hit.fId);
      //Histogramer::fill("vTOF",400,0,3200,hit.fTimeRight - fdsi.fLowGain1.dytime,
      //                         4000,0,4000,hit.fEnergyLeft + hit.fEnergyRight);
      //
      //Histogramer::fill("vTOFR",400,0,3200,hit.fTimeRight - fdsi.fLowGain1.dytime,
      //                         4000,0,4000,hit.GetQDC());
      //Histogramer::fill("vTOFL",400,0,3200,hit.fTimeLeft - fdsi.fLowGain1.dytime,
      //                         4000,0,4000,hit.GetQDC());
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


    //testing.
    //******************************//
    //******************************//
    printf("decay:\n");
    for(int z=0;z<int(implants.size());z++) {
      double dtime = (fdsi.fClock.initial/1.e6) - implants.at(z).mtime();
      TIter iter(Histogramer::Get()->GetBlobs());
      std::string name = "";
      while(TCutG* blob = (TCutG*)iter.Next()) {
        if(blob->IsInside(implants.at(z).tof,implants.at(z).de2)) { 
          name = blob->GetName();
          break;
        } 
      }
      //print time, dr2, fom, name.
      if(implants.at(z).fom<4.0) { printf(GREEN); } // ~ 3 pixel radius 
      printf("\t%i\t%.1f\t%.1f\t%.1f\t%s\n",z,dtime,implants.at(z).dr2,implants.at(z).fom,name.c_str()); 
      printf(RESET_COLOR);

    }
    //******************************//
    //******************************//

    //loop over blobs.
    TIter iter(Histogramer::Get()->GetBlobs());
    while(TCutG* blob = (TCutG*)iter.Next()) {
      //for each blob, check if the any implant in the implant list matches. 
      for(int z=0;z<int(implants.size());z++) {
        bool first = true;
        if(blob->IsInside(implants.at(z).tof,implants.at(z).de2)) { 
          double dtime = (fdsi.fClock.initial/1.e6) - implants.at(z).mtime();
          Histogramer::fill(blob->GetName(),"dtimeOnly",2000,-1000,5000,dtime);
          //Histogramer::fill(blob->GetName(),"dtimefom",6000,-1000,5000,dtime,
          //                                                   1000,0,1000,implants.at(z).fom);
          for(const auto &hit : fdsi.fClover.hits) {
            double gdt = fdsi.fLowGain1.dytime - hit.fTime;
            Histogramer::fill(blob->GetName(),"gtime",500,-2000,2000,gdt, //fdsi.fLowGain1.dytime - hit.fTime,
                1000,0,4000,hit.fEcal);
            if(!Histogramer::Get()->GetGammaPrompt()->IsInside(gdt,hit.fEcal)) continue;
            Histogramer::fill(blob->GetName(),"gsummary",16000,0,8000,hit.fEcal,
                70,0,70,hit.fId);

            if( (dtime>0 && dtime<100) || (dtime>900 && dtime<1000) ) { 
              for(const auto &hit1 : fdsi.fClover.hits) {
                if(&hit == &hit1) continue;
                //if(std::abs(hit.fTime - hit1.fTime)>200) continue; // 100 is made up atm
                double gdt1 = fdsi.fLowGain1.dytime - hit1.fTime;
                if(!Histogramer::Get()->GetGammaPrompt()->IsInside(gdt1,hit1.fEcal)) continue;
                if(dtime>0 && dtime<100) 
                  Histogramer::fill(blob->GetName(),"gg_0_100",4000,0,4000,hit.fEcal,
                                                               4000,0,4000,hit1.fEcal);
                if(dtime>900 && dtime<1000) 
                  Histogramer::fill(blob->GetName(),"gg_900_1000",4000,0,4000,hit.fEcal,
                                                                  4000,0,4000,hit1.fEcal);
              }
            }
            Histogramer::fill(blob->GetName(),"gdtime",6000,-1000,5000,dtime,
                                                              8000,0,8000,hit.fEcal);
            //if(nmult==1) 
            //  Histogramer::fill(blob->GetName(),"gdtime1N",6000,-1000,5000,dtime,
            //                                                  8000,0,8000,hit.fEcal);
            //if(nmult==2) 
            //  Histogramer::fill(blob->GetName(),"gdtime2N",6000,-1000,5000,dtime,
            //                                                  8000,0,8000,hit.fEcal);
            if(nmult>0) 
              Histogramer::fill(blob->GetName(),"gdtimeAN",6000,-1000,5000,dtime,
                                                              8000,0,8000,hit.fEcal);

            //if(first) 
            //   Histogramer::fill(blob->GetName(),"gdtime_0",6000,-1000,5000,dtime,
            //                                                     8000,0,8000,hit.fEcal);

          }

          for(const auto &hit : fdsi.fClover.addbackHits) {
            Histogramer::fill(blob->GetName(),"atime",500,-2000,2000,fdsi.fLowGain1.dytime - hit.fTime,
                1000,0,4000,hit.fEcal);

            Histogramer::fill(blob->GetName(),"asummary",16000,0,8000,hit.fEcal,
                20,0,20,hit.fId);

            if( (dtime>0 && dtime<100) || (dtime>900 && dtime<1000) ) { 
              for(const auto &hit1 : fdsi.fClover.addbackHits) {
                if(&hit == &hit1) continue;
                if(std::abs(hit.fTime - hit1.fTime)>200) continue; // 100 is made up atm
                if(dtime>0 && dtime<100) 
                  Histogramer::fill(blob->GetName(),"aa_0_100",4000,0,4000,hit.fEcal,
                                                               4000,0,4000,hit1.fEcal);
                if(dtime>900 && dtime<1000) 
                  Histogramer::fill(blob->GetName(),"aa_900_1000",4000,0,4000,hit.fEcal,
                                                                  4000,0,4000,hit1.fEcal);
              }
            }
            Histogramer::fill(blob->GetName(),"adtime",6000,-1000,5000,dtime,
                                                              8000,0,8000,hit.fEcal);
            //if(nmult==1) 
            //  Histogramer::fill(blob->GetName(),"adtime1N",6000,-1000,5000,dtime,
            //                                                  8000,0,8000,hit.fEcal);
            //if(nmult==2) 
            //  Histogramer::fill(blob->GetName(),"adtime2N",6000,-1000,5000,dtime,
            //                                                  8000,0,8000,hit.fEcal);
            if(nmult>0) 
              Histogramer::fill(blob->GetName(),"adtimeAN",6000,-1000,5000,dtime,
                                                              8000,0,8000,hit.fEcal);

            //if(first) 
            //   Histogramer::fill(blob->GetName(),"adtime_0",6000,-1000,5000,dtime,
            //                                                     8000,0,8000,hit.fEcal);

          }




          first=false;
        }
      }
    }

  }

}


