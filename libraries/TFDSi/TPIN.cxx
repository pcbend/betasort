
#include <TPIN.h>
#include <ddasHit.h>

#include <TRandom.h>

ClassImp(TPIN);

//
// Reset PIN variables
//
void TPIN::Reset()
{
  fEnergy = 0.;
  fTime   = 0.;
  fEcal   = 0.;
  fHit    = 0;
}

TPIN::TPIN(ddasHit& ddashit) {
  Unpack(ddashit);
}

void TPIN::Unpack(ddasHit &ddashit) {

  fTime = ddashit.GetTime();
  fEnergy = ddashit.GetEnergy() + gRandom->Rndm();
  //double ec = cal.slope*energy + cal.intercept;
  fEcal = fEnergy *1.0 + 0.0;

  // threshold check
  //if(ec > cal.thresh && ec < cal.uld) {
  if(fEcal > 10 && fEcal < 16000) {
    fHit = 1;
  }
}

TPIN::TPIN() { Reset(); }

TPIN::~TPIN() { } 

void TPIN::Copy(TPIN &other) const {
  other.fEnergy = fEnergy;
  other.fEcal   = fEcal;
  other.fTime   = fTime;
  other.fHit    = fHit;

  return;
}


