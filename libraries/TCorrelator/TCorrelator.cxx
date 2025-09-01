

#include <iostream>

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

void TCorrelator::push(TFDSi &fdsi,std::vector<TPID> &corrImplants) {
  std::lock_guard<std::mutex> lock(CorrelatorMtx);
  fQueue.push(std::make_pair(fdsi, corrImplants)); 
  return;
}

std::pair<TFDSi,std::vector<TPID> > TCorrelator::pop() {
  std::lock_guard<std::mutex> lock(CorrelatorMtx);
  if(fQueue.size()!=0) {
    //TFDSi fdsi = fQueue.front();
    std::pair<TFDSi,std::vector<TPID> >temp = fQueue.front();
    fQueue.pop();
    return temp;
  }
  std::pair<TFDSi,std::vector<TPID> > temp;
  return temp;
}


void TCorrelator::Correlate() {

  fLoopRunning = true;
  int implantsPushed = 0;
  int implantsErased = 0;
  int implantsMatched = 0;
  while(true) { 
    if(!Unpacker::Get()->LoopRunning() &&  Unpacker::Get()->qsize()==0) 
      break;

    TFDSi fdsi = Unpacker::Get()->pop();
    double current_time = fdsi.fClock.initial/1.e6;

    if(fdsi.fEventType==4) {         // 'good' implant

      int xPos = int(fdsi.GetX());
      int yPos = int(fdsi.GetY());

      double timeDiffIon = -1.;
      if(fImplanted[xPos][yPos].time > 0) {  // is there already a PID in the implant map?
        timeDiffIon = current_time - fImplanted[xPos][yPos].time;
        if(timeDiffIon < 0) { 
          printf(RED "Ion's out of time order\n" RESET_COLOR);
        }
        fImplanted[xPos][yPos].dtLastIon = timeDiffIon;
      } else {
        fImplanted[xPos][yPos].implanted = true;
      }

      fImplanted[xPos][yPos].time = fdsi.fPID.time / 1.e6; 
      fImplanted[xPos][yPos].dyecal = fdsi.fLowGain1.dyecal;
      fImplanted[xPos][yPos].de1  = fdsi.fPID.de1;  
      fImplanted[xPos][yPos].de2  = fdsi.fPID.de2;  
      fImplanted[xPos][yPos].tof  = fdsi.GetTOF();//fdsi.fPID.tof;  
      fImplanted[xPos][yPos].xpos = fdsi.fPID.xpos; 
      fImplanted[xPos][yPos].ypos = fdsi.fPID.ypos; 

      // copy the local implant map into the vector....
      fImplantVector[xPos][yPos].push_back(fImplanted[xPos][yPos]);
      implantsPushed++;

      //dummy to get the fdsi implant info into the final tree....
      std::vector<TPID> empty;
      push(fdsi,empty);

    } else if(fdsi.fEventType==12) { // 'good' decays

      int xPos = int(fdsi.GetX());
      int yPos = int(fdsi.GetY());

      // fCorrWin = 5; //Correlation window size in mm set in header file.
      int corrGridLimit = (fCorrWin - 1)/2; // for fCorrWin = 3, corrGridLimit = 1

      //search the local grid for implants. 
      for(int i=(-corrGridLimit); i<(corrGridLimit+1); i++) {
        for(int j=(-corrGridLimit); j<(corrGridLimit+1); j++) {
          if((xPos+i) >= 0 && (xPos+i) < npspmt_utk &&  // check the xpos & ypos are in bounds. 
              (yPos+j) >= 0 && (yPos+j) < npspmt_utk) {

            int it = 0;
            for(auto & ion : fImplantVector[xPos+i][yPos+j]) { //empty??
              double dtime = current_time - ion.time; 

              if(abs(dtime) > 1000) {
                //                printf(BLUE); printf("\t abs(%.1f - %.1f) = %.1f\n",current_time,ion.time, current_time-ion.time); printf(RESET_COLOR); 
                fImplantVector[xPos+i][yPos+j].erase(fImplantVector[xPos+i][yPos+j].begin()+it);
                implantsErased++;
              } else {
                it++;
                TPID implantStore;
                implantStore.Reset();
                ion.Copy(implantStore);
                //printf(RED); printf("[%i][%i]\t",xPos+i,yPos+j); ion.Print(); printf(RESET_COLOR);
                //                printf(GREEN); printf("\t abs(%.1f - %.1f) = %.1f\n",current_time,ion.time, current_time-ion.time); printf(RESET_COLOR); 

                //double x1 = fdsi.GetX();
                //double y1 = fdsi.GetY();
                //double x2 = ion.xpos;
                //double y2 = ion.ypos;

                //double distance = std::sqrt(std::pow((x2-x1),2.0) + std::pow((y2-y1),2.0)); 
                //printf("dist: (%.1f,%.1f)\t(%.1f,%.1f)  \n",x1,y1,x2,y2);
                //printf("corr: %i\t%.02f\n",corrGridLimit,distance);
                //Histogramer::Get()->fill("distance",500,0,50,distance);
                //if(distance < corrGridLimit) {
                  fCorrelatedImplants.push_back(implantStore);
                  implantsMatched++;
                //}
              }
            }
          }
        }
      }

      push(fdsi,fCorrelatedImplants);
      ClearImplants();
    } // end is decay.
  }



  printf("\n\n\n\n");
  printf("\timplantsPushed:  %i\n",implantsPushed);
  printf("\timplantsErased:  %i\n",implantsErased);
  printf("\timplantsMatched: %i\n",implantsMatched);
  printf("\n\n\n\n");

  fLoopRunning = false;

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






