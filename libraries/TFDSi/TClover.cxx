#include "TClover.h"
#include <ddasHit.h>

#include <iostream>
#include <fstream>
#include <sstream>


TCloverHit::TCloverHit() {  } 

TCloverHit::~TCloverHit() {  } 

TCloverHit::TCloverHit(const TCloverHit &hit) {
  hit.Copy(*this);
} 

void TCloverHit::Copy(TCloverHit &other) const {
  other.fId      = fId;
  other.fTime    = fTime;
  other.fCfdTime = fCfdTime;
  other.fEnergy  = fEnergy;
  other.fEcal    = fEcal;
}

void TCloverHit::Reset() {
  fId       = -1;
  fTime     = -1;
  fCfdTime  = -1;
  fEnergy   = -1;
  fEcal     = -1;   
}


TAddbackHit::TAddbackHit() :TCloverHit(),fMult(0) { } 

TAddbackHit::~TAddbackHit() { } 

bool TAddbackHit::CheckTime(const TCloverHit &other) const {
  return std::abs(other.fTime - fTime) < 200; // 100 is made up atm
}

TAddbackHit::TAddbackHit(const TCloverHit &hit) :TCloverHit(hit),fMult(1) { 
  fId =  hit.fId/4;
  fEnergy = fEcal;
} 

void TAddbackHit::Reset() {
  TCloverHit::Reset();
  fMult = 0;
}



void TAddbackHit::Add(const TCloverHit &hit) {
  fEcal += hit.fEcal;
  if(hit.fEcal > fEnergy) {
    fEnergy  = hit.fEcal;
    fTime    = hit.fTime;
    fCfdTime = hit.fCfdTime;
  }
  fMult++;
}





std::map<int,std::vector<double> > TClover::fCals;

void TClover::ReadCalFile(std::string name) {
  std::cout << "Reading Clover calibration from: " << name << std::endl;

  std::ifstream calfile(name.c_str());
  int linenum = 0;
  int location = -1;
  double value = 0;

  if(!calfile) {
    std::cout << "----> Unable to open file " << name << std::endl;
  } else {
    std::string line;
    while(std::getline(calfile,line)) {
      linenum++;
      std::stringstream ss(line);
      int id;
      double m,b;
      ss >> id;
      ss >> b;
      ss >> m;
      std::vector<double> params;
      params.push_back(b);
      params.push_back(m);
      fCals[id-1] = params;
    }
  }  
  calfile.close();
}

//
// Reset Clover variables
//

TClover::TClover() { } 

TClover::~TClover() { } 

void TClover::Reset() {
  hits.clear();
  addbackHits.clear();
  hit = 0;
}

void TClover::Unpack(const ddasHit &ddashit, int det)
{
  // position derived from crystal number
  int pos = det/4;

  TCloverHit hit;
  hit.fId      = det;
  hit.fTime    = ddashit.GetTime();
  hit.fCfdTime = ddashit.GetCFDTime();
  hit.fEnergy  = ddashit.GetEnergy() + gRandom->Rndm();
  hit.fEcal    = 0.0;
  for(size_t i=0;i<fCals[det].size();i++) 
    hit.fEcal += pow(hit.fEnergy,i)*fCals[det].at(i);
  if(hit.fEcal<10. || hit.fEcal>8192.) return; 
  hits.push_back(hit);
}


void TClover::Copy(TClover &other) const {
  other.Reset();
  other.hit = hit;
  for(size_t i=0;i<hits.size();i++)
    other.hits.push_back(TCloverHit(hits[i]));

}

void TClover::BuildAddback() { 
  addbackHits.clear();
  for(const auto& hit : hits) {
    const int addbackId = hit.fId/4;
    if(addbackId < 0 || addbackId >= 16) continue;
    bool added = false;
    
    for(auto& ab : addbackHits) {
      if(ab.fId != addbackId) continue;
      if(ab.CheckTime(hit))   continue;
      ab.Add(hit);
      added = true;
      break;
    }
    if(!added) addbackHits.emplace_back(hit);
  }
} 


