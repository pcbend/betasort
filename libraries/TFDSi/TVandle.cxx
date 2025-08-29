#include "TVandle.h"
#include <ddasHit.h>

#include <iostream>
#include <fstream>
#include <sstream>


TVandleHit::TVandleHit() {  } 

TVandleHit::~TVandleHit() {  } 

TVandleHit::TVandleHit(const TVandleHit &hit) {
  hit.Copy(*this);
} 

void TVandleHit::Copy(TVandleHit &other) const {
  other.fId      = fId;
  other.fTime    = fTime;
  other.fCfdTime = fCfdTime;
  other.fEnergy  = fEnergy;
  other.fEcal    = fEcal;
}

void TVandleHit::Reset() {
  fId       = -1;
  fTime     = -1;
  fCfdTime  = -1;
  fEnergy   = -1;
  fEcal     = -1;   
}


//
// Reset Vandle variables
//

TVandle::TVandle() { } 

TVandle::~TVandle() { } 

void TVandle::Reset() {
  fHits.clear();
}

void TVandle::Unpack(ddasHit &ddashit, int det)
{
  // position derived from crystal number
  int pos = det/4;

  TVandleHit hit;
  hit.fId      = det;
  hit.fTime    = ddashit.GetTime();
  hit.fCfdTime = ddashit.GetCFDTime();
  hit.fEnergy  = ddashit.GetEnergy() + gRandom->Rndm();
  hit.fEcal    = 0.0;
  //for(size_t i=0;i<fCals[det].size();i++) 
  //  hit.fEcal += pow(hit.fEnergy,i)*fCals[det].at(i);
  
  fHits.push_back(hit);
}


void TVandle::Copy(TVandle &other) const {
  other.Reset();
  for(size_t i=0;i<fHits.size();i++)
    other.fHits.push_back(TVandleHit(fHits[i]));

}

