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
  hit = 0;
}

void TClover::Unpack(ddasHit &ddashit, int det)
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
  
  hits.push_back(hit);
}


void TClover::Copy(TClover &other) const {
  other.Reset();
  other.hit = hit;
  for(size_t i=0;i<hits.size();i++)
    other.hits.push_back(TCloverHit(hits[i]));



}

