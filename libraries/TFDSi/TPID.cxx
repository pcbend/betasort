
#include "TPID.h"
#include "TFDSi.h"
//
void TPID::Reset()
{
  timestamp = 0.;
  dyecal = 0;
  de1    = 0;
  de2    = 0;
  tof    = 0;
  xpos = -1.;
  ypos = -1.;
  dtLastIon = 0;
}

void TPID::Print(Option_t *opt) const {
  printf("ion@[%f]: e(%.1f) p(%.1f,%.1f)\n",timestamp/1e6,de2,xpos,ypos);
 // printf("\tde2:   %.1f\n",de2);
 // printf("\txpos:  %.1f\n",xpos);
 // printf("\typos:  %.1f\n",ypos);
 // printf("\ttime:  %.1f\n",time); 
}

void  TPID::Copy(TPID &other) const { 
  other.dyecal        = dyecal;
  other.de1           = de1;
  other.de2           = de2;
  other.tof           = tof;
  other.timestamp     = timestamp;  
  other.xpos          = xpos;
  other.ypos          = ypos;
  other.dtLastIon     = dtLastIon;
}


TImplant::TImplant(const TPID& imp,const TFDSi& decay) : TPID(imp) {
  dtime = (decay.fClock.initial - imp.timestamp) / 1.e6;
  dx    = imp.xpos - decay.GetX();
  dy    = imp.ypos - decay.GetY();
  dr2   = dx*dx + dy*dy;
  
  const double sigmaR = 1.5;    // pixels
  const double sigmaT = 200.0;  // ms

  if(dtime>0) 
    fom = dr2/(sigmaR*sigmaR) + (dtime*dtime)/(sigmaT*sigmaT);
  else 
    fom = -1;
}




