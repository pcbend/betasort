

#include <iostream>
#include <thread>
#include <chrono>


#include <globals.h>
#include <TCorrelator.h>
#include <Unpacker.h>
#include <Histogramer.h>

#include<TString.h>


TCorrelator *TCorrelator::fCorrelator = 0;
std::mutex TCorrelator::fCorrelatorMutex;

TCorrelator::TCorrelator() :fIn(0), fOut(0) { Reset(); } 

TCorrelator::~TCorrelator() { } 

TCorrelator *TCorrelator::Get() { 
  if(!fCorrelator) {
    std::lock_guard<std::mutex> lock(fCorrelatorMutex);
    fCorrelator = new TCorrelator();
  }
  return fCorrelator;
}

void TCorrelator::Reset() {
  printf(DYELLOW " Correlator Is Being Reset!\n\n\n\n" RESET_COLOR);
  for(int i=0; i<npspmt_utk; i++) {
    for(int j=0; j<npspmt_utk; j++) {
      fImplanted[i][j].Reset(); 
      fImplantVector[i][j].clear();
    }    
  } 
  fCorrelatedImplants.clear();
}

std::mutex CorrelatorMtx; 

void TCorrelator::push(const TFDSi &fdsi,std::vector<TImplant> &corrImplants) {
  std::lock_guard<std::mutex> lock(CorrelatorMtx);
  fQueue.push(std::make_pair(fdsi, corrImplants)); 
  return;
}

std::pair<TFDSi,std::vector<TImplant> > TCorrelator::pop() {
  std::lock_guard<std::mutex> lock(CorrelatorMtx);
  if(fQueue.size()!=0) {
    //TFDSi fdsi = fQueue.front();
    std::pair<TFDSi,std::vector<TImplant> >temp = fQueue.front();
    fQueue.pop();
    return temp;
  }
  std::pair<TFDSi,std::vector<TImplant> > temp;
  return temp;
}


void TCorrelator::Correlate() {

  fLoopRunning = true;
  int implantsPushed  = 0;
  int implantsErased  = 0;
  int implantsMatched = 0;
  int decaysPushed    = 0;
  int decaysFinalized = 0;

  while(true) { 
    if(Unpacker::Get()->qsize()>0) { //event available to correlate
      TFDSi fdsi = Unpacker::Get()->pop();
      double current_time = fdsi.fClock.initial/1.e6;
      if(fdsi.fEventType==4) {         // 'good' implant
        StoreImplant(fdsi,current_time);
        implantsPushed++;
      } else if(fdsi.fEventType==12) { // 'good' decays
        StoreDecay(fdsi,current_time);
        decaysPushed++;
      } // end is decay.
      decaysFinalized += FinalizeDecays(current_time);
      implantsErased  += PruneImplants(current_time);
      continue;
    }

    if(!Unpacker::Get()->LoopRunning() &&  //finished upacking - still have decays pending 
       Unpacker::Get()->qsize()==0    &&
       !fPendingDecays.empty()) {
      decaysFinalized += FlushDecays();
      continue;
    }

    if(!Unpacker::Get()->LoopRunning() &&  //finished. 
       Unpacker::Get()->qsize()==0    &&
       fPendingDecays.empty()) {
      break;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1));

  }



  printf("\n\n\n\n");
  printf("\timplantsPushed:  %i\n",implantsPushed);
  printf("\timplantsErased:  %i\n",implantsErased);
  printf("\timplantsMatched: %i\n",implantsMatched);
  printf("\tdecaysPushed:    %i\n",decaysPushed);
  printf("\tdecaysFinalized: %i\n",decaysFinalized);
  printf("\n\n\n\n");

  fLoopRunning = false;

}



void TCorrelator::StoreImplant(const TFDSi& fdsi, double current_time) { 
  int xPos = int(fdsi.GetX());
  int yPos = int(fdsi.GetY());
  
  double timeDiffIon = 0;
  double lasttime = fImplanted[xPos][yPos].timestamp / 1.e6; // timestamp in ms. 
  if(lasttime> 0) {  // is there already a PID in the implant map?
    timeDiffIon = current_time - lasttime;
    if(timeDiffIon < 0) { 
      printf(RED "Ion's out of time order\n" RESET_COLOR);
    }
  } 
  
  fImplanted[xPos][yPos].timestamp = fdsi.fPID.timestamp; // / 1.e6; 
  fImplanted[xPos][yPos].dyecal    = fdsi.fLowGain1.dyecal;
  fImplanted[xPos][yPos].de1       = fdsi.fPID.de1;  
  fImplanted[xPos][yPos].de2       = fdsi.fPID.de2;  
  fImplanted[xPos][yPos].tof       = fdsi.GetTOF();//fdsi.fPID.tof;  
  fImplanted[xPos][yPos].xpos      = fdsi.fPID.xpos; 
  fImplanted[xPos][yPos].ypos      = fdsi.fPID.ypos; 
  fImplanted[xPos][yPos].dtLastIon = timeDiffIon;
  
  // copy the local implant map into the vector....
  //fImplantVector[xPos][yPos].push_back(fImplanted[xPos][yPos]);
  fImplantVector[xPos][yPos].emplace_back(fImplanted[xPos][yPos]);
  
  //dummy to get the fdsi implant info into the final tree....
  std::vector<TImplant> empty;
  push(fdsi,empty);
} 

void TCorrelator::StoreDecay(const TFDSi& fdsi, double t) { 
  PendingDecay pd;
  pd.fdsi = fdsi;
  pd.time = t;
  fPendingDecays.push_back(pd);
} 

void TCorrelator::CorrelateOneDecay(const TFDSi &fdsi,double tdecay) {
  
  fCorrelatedImplants.clear();

  int xPos = int(fdsi.GetX());
  int yPos = int(fdsi.GetY());
 
  if(xPos<0 || xPos >=npspmt_utk ||
     yPos<0 || yPos >=npspmt_utk) return;

  const int fSearchRadius = 4;   

  for(int ix = std::max(0, xPos - fSearchRadius);
          ix <= std::min(npspmt_utk-1, xPos + fSearchRadius); ++ix) {
    for(int iy = std::max(0, yPos - fSearchRadius);
            iy <= std::min(npspmt_utk-1, yPos + fSearchRadius); ++iy) {
      
      const auto& vec = fImplantVector[ix][iy];
      for(const auto&ion : vec) { 
        double dtime = tdecay - ion.mtime();
        if(dtime < -fBGWindow)    continue;
        if(dtime >  fDecayWindow) continue;

        TImplant match(ion,fdsi);
        fCorrelatedImplants.push_back(match);

      }
    }
  }
  push(fdsi,fCorrelatedImplants);
  ClearImplants();
}

int TCorrelator::PruneImplants(double current_time) {
  const double tmin_keep = current_time - (fBGWindow + fDecayWindow);
  int erased = 0;
  for(int x=0;x<npspmt_utk;x++) {
    for(int y=0;y<npspmt_utk;y++) {
      auto &vec = fImplantVector[x][y];
      if(vec.empty()) continue;

      auto first_keep = std::find_if(vec.begin(),vec.end(),
        [tmin_keep] (const TPID &ion) { return ion.mtime() >= tmin_keep;});
      erased += std::distance(vec.begin(),first_keep);
      vec.erase(vec.begin(),first_keep);
    }
  }
  return erased;
}

int TCorrelator::FinalizeDecays(double current_time) { 
  int decaysHandled = 0;
  while(!fPendingDecays.empty()) {
    const PendingDecay& pd = fPendingDecays.front();

    if(current_time < (pd.time + fBGWindow))
      break;
    CorrelateOneDecay(pd.fdsi,pd.time);
    fPendingDecays.pop_front();
    decaysHandled++;
  }
  return decaysHandled;
}


int TCorrelator::FlushDecays() { 
  int decaysHandled = 0;
  while(!fPendingDecays.empty()) {
    const PendingDecay& pd = fPendingDecays.front();
    CorrelateOneDecay(pd.fdsi,pd.time);
    fPendingDecays.pop_front();
    decaysHandled++;
  }
  return decaysHandled;
}

std::string TCorrelator::Status()  {
  std::lock_guard<std::mutex> lock(CorrelatorMtx);
  std::string s = Form("TCorrelator in[%lu]  out[%lu]  q[%lu]",fIn,fOut,qsize());
  return s;
}


void TCorrelator::ShowImplantVector(int dx,int dy) const {
  size_t size=0;
  printf(" \t0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47\n");  
  int corrGridLimit = 3;
  for(int x=0;x<npspmt_utk; x++) {
    for(int y=0;y<npspmt_utk; y++) {
      if(y==0) printf("%i\t",x);
      size = fImplantVector[x][y].size();

      bool reset = true;
      if( x >= (dx-corrGridLimit) && x < (dx+corrGridLimit+1) &&
          y >= (dy-corrGridLimit) && y < (dy+corrGridLimit+1)) {
        printf(DYELLOW); 
        reset =true;
      }

      //printf("%lu  ",size);
      if(size) { printf(RED); reset = true; }
      if(x==dx && y==dy) { printf(BLUE); reset=true; } 

      printf("%lu  ",size);
      if(reset) printf(RESET_COLOR);
      if(y==(npspmt_utk-1)) printf("\n"); 
    }
  }

}






