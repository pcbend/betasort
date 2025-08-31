#include "TVandle.h"
#include <ddasHit.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <TCanvas.h>
#include <TH2D.h>

TVandleHit::TVandleHit() { Reset(); } 

TVandleHit::~TVandleHit() {  } 

TVandleHit::TVandleHit(const TVandleHit &hit) {
  hit.Copy(*this);
} 

void TVandleHit::Copy(TVandleHit &other) const {
  other.fId          = fId;
  other.fTimeRight   = fTimeRight;
  other.fTimeLeft    = fTimeLeft;
  other.fEnergyRight = fEnergyRight;
  other.fEnergyLeft  = fEnergyLeft;
  other.fEcal        = fEcal;
  other.fTraceLeft   = fTraceLeft;
  other.fTraceRight  = fTraceRight;
}
 
void TVandleHit::Reset() {
  fId          = -1;
  fTimeRight   = -1;
  fTimeLeft    = -1;
  fEnergyRight = -1;
  fEnergyLeft  = -1;
  fEcal        = -1;   
  fTraceLeft.clear();
  fTraceRight.clear();
}

void TVandleHit::Print(Option_t *opt) const {
  printf("[%03i] EL[%.1f] ER[%.1f] QDC[%.1f]  @ TL[%.1f] TR[%.1f]\n",fId,fEnergyLeft,fEnergyRight,GetQDC(),fTimeLeft,fTimeRight);
}

void TVandleHit::DrawTrace() {
  TCanvas *c = 0;
  if(gPad)
    c = gPad->GetCanvas();
  if(!c) c = new TCanvas;
  c->Clear();
  c->Divide(1,2);
  TH2D hL("traceL","traceL",150,0,150,4000,0,4000);
  TH2D hR("traceR","traceR",150,0,150,4000,0,4000);
  for(size_t x=0;x<fTraceLeft.size();x++) {
    hL.Fill(x,fTraceLeft.at(x));
    hR.Fill(x,fTraceRight.at(x));
  }
  c->cd(1);
  hL.DrawCopy();
  c->cd(2);
  hR.DrawCopy();
  return;
}


double TVandleHit::GetQDC() const {
  if(fTraceLeft.size() < 80 || fTraceRight.size()<80) return 0.0;
  double bgL  =0;
  double sumL =0;
  double bgR  =0;
  double sumR =0;
  for(int i=0;i<80;i++) {
    if(i<40) {
      bgL+=fTraceLeft.at(i);
      bgR+=fTraceRight.at(i);
    } else {
      sumL+=fTraceLeft.at(i);
      sumR+=fTraceRight.at(i);
    }
  }
  return sumL+sumR - bgL - bgR;
} 



//
// Reset Vandle variables
//

TVandle::TVandle() { } 

TVandle::~TVandle() { } 

void TVandle::Reset() {
  fHits.clear();
}

void TVandle::Print(Option_t *opt) const {
  printf("Vandle (size = %lu)\n",fHits.size());
  for(size_t x=0;x<fHits.size();x++) {
    printf("\t"); fHits.at(x).Print();
  }
}


void TVandle::Unpack(ddasHit &ddashit, int det)
{

  // position derived from crystal number
  int id  = ddashit.GetId();
  int pos = id%2;
  int DET = (id-32)/2 + 1;   //  this should be 1 .. 104

  auto it = std::find_if(fHits.begin(),fHits.end(),[DET](const TVandleHit &hit) { 
    return hit.fId == DET;
  });
 
//  printf("trace.size  = %lu\n",ddashit.GetTrace().size());
//  printf("qdc.size    = %lu\n",ddashit.GetQDCSums().size());

  if(it != fHits.end()) {
    if(pos==0) {
      it->fTimeLeft    = ddashit.GetTime();
      it->fEnergyLeft  = ddashit.GetEnergy() + gRandom->Rndm();
      it->fTraceLeft   = ddashit.GetTrace();
    } else {
      it->fTimeRight    = ddashit.GetTime();
      it->fEnergyRight  = ddashit.GetEnergy() + gRandom->Rndm();
      it->fTraceRight   = ddashit.GetTrace();
    }
  } else {
    TVandleHit hit;
    if(pos==0) {
      hit.fTimeLeft    = ddashit.GetTime();
      hit.fEnergyLeft  = ddashit.GetEnergy() + gRandom->Rndm();
      hit.fTraceLeft    = ddashit.GetTrace();
    } else {
      hit.fTimeRight    = ddashit.GetTime();
      hit.fEnergyRight  = ddashit.GetEnergy() + gRandom->Rndm();
      hit.fTraceRight   = ddashit.GetTrace();
    }
    hit.fId      = DET;   //  this should be 1 .. 104
    hit.fEcal    = 0.0;
    fHits.push_back(hit);
  }
}

void TVandle::Copy(TVandle &other) const {
  other.Reset();
  for(size_t i=0;i<fHits.size();i++)
    other.fHits.push_back(TVandleHit(fHits[i]));

}





