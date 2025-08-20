/*
 * TPID.cpp. Class methods PID variables which are 
 * derived from dE measurements in an upstream PIN
 * detector and some measure of TOF throught the 
 * fragment separator. 
 * 
 * Author: A. Chester
 *
 */

#include "TPID.h"

ClassImp(TPID);


//
// Reset PID variables
//
void TPID::Reset()
{
  dyecal = 0;
  // PIN dE
  de1 = 0.;
  de2 = 0.;
  // TOF
  tof = 0.;

  // Correlation variables
  time = 0.;
  xpos = -1.;
  ypos = -1.;

  implanted = false;
  dtLastIon = -1.;

  hasGoodPosition = false;
}

void TPID::Print() {
  printf("ion@[%f]: e(%.1f) p(%.1f,%.1f)\n",time,de2,xpos,ypos);
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

  other.time          = time;  
  other.xpos          = xpos;
  other.ypos          = ypos;
  
  other.implanted     = implanted;
  other.dtLastIon     = dtLastIon;

  other.hasGoodPosition = hasGoodPosition;
}






