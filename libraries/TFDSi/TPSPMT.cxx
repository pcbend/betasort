
#include <TPSPMT.h>
#include <ddasHit.h>

#include <Histogramer.h>

#include <TRandom.h>

#include <globals.h>

// anode position fitting results
#define ERR_MINV     -100 // Matrix for linear equation solver is not invertable
#define ERR_BADPOS    -99 // Position fit outside of anode pixel range [0,nanodes)
#define ERR_CONCDOWN  -98 // Quadratic fit is concave down (no minimum)
#define ERR_QDCMULT   -97 // Not all QDC sums present for the event

TPSPMT::TPSPMT() { }

TPSPMT::~TPSPMT() { } 


void TPSPMT::Reset() { 
  // dynode
  dyenergy = 0;        //unpacked
  dyecal   = 0;           //unpacked
  dytime   = 0;           

  // Anode
  for(int i=0;i<NANODES;i++) { 
    aenergy[i]     = 0.;
    aecal[i]       = 0.;
    aenergy_qdc[i] = 0.;
    atime[i]       = 0.;
    ahit[i]        = 0;
  }

  asum     = 0;
  asum_qdc = 0;

  baseline = 0;
  area     = 0;

  // Anode position
  xpos = -1;
  ypos = -1;;

  // hit flags
  dyhit = 0;
  dyhihit   = 0; // dynode and at least one anode
}


void TPSPMT::Copy(TPSPMT &other) const {

  other.dyenergy = dyenergy; 
  other.dyecal   = dyecal;   
  other.dytime   = dytime;   

  // Anode
  for(int i=0;i<NANODES;i++) { 
    other.aenergy[i]     = aenergy[i]    ;
    other.aecal[i]       = aecal[i]      ;
    other.aenergy_qdc[i] = aenergy_qdc[i];
    other.atime[i]       = atime[i]      ;
    other.ahit[i]        = ahit[i]       ; 
  }

  other.asum     = asum;
  other.asum_qdc = asum_qdc;

  other.baseline = baseline;
  other.area     = area;

  // Anode position
  other.xpos = xpos;
  other.ypos = ypos;

  // hit flags
  other.dyhit   = dyhit;
  other.dyhihit = dyhihit; // dynode and at least one anode
  
  return;
}


void TPSPMT::UnpackDynode(ddasHit& ddashit) {
  // extract hit information
  // put hit information into detector class
  // time
  dytime = ddashit.GetTime();      // this takes into account the cfd...
  //dytimecfd = ddashit.GetCFDTime();
  
  // energy (from Pixie)
  dyenergy = ddashit.GetEnergy() + gRandom->Rndm();
  double ec = 1.0*dyenergy + 0.0;

  if(ddashit.GetTrace().size()) {
    baseline=0;
    area=0;
    for(size_t i=0;i<ddashit.GetTrace().size();i++) {
      if(i<10)
        baseline += ddashit.GetTrace().at(i);
      area += ddashit.GetTrace().at(i);
    }
    baseline /= 10.;
    area -= ddashit.GetTrace().size()*baseline;  

  }

  //if(ec > 10 && ec < 16000) {
//  if(ec > 10 && ec < 70000) {
    if(!dyhit) dyhit = 1;
    dyecal = ec;
//  }
}

void TPSPMT::UnpackAnode(ddasHit& ddashit,int pix)
{
  // extract hit information
  //amultraw++; // raw multiplicty
  
  // put hit information into detector class
  // trace analysis

  // time
  double time = ddashit.GetTime();
  
  // Due to some odd events where the anodes have times but no energies, lets make sure that the
  // anode energy is reasonable as well before keeping any anode data.
  double energy = ddashit.GetEnergy() + gRandom->Rndm();
  aenergy[pix] = energy;
  double ec = 1.0*energy + 0.0;
  
  // the conditon for a good anode event is:
  // |t_anode-t_dynode| < dycoincwindow AND athresh < e_anode < auld.

  //TODO: histogram time -dytime.
  Histogramer::fill("timeDytime",2000,-10000,10000,time-dytime);
  Histogramer::fill("anodeEnergy",8000,0,64000,ec,500,0,500,ddashit.GetId());

  //if(std::abs(time - dytime) < 10000.) {  // HERE - the dytime MUST be set first to use this....
  if(std::abs(time - dytime) < 150.) {  // HERE - the dytime MUST be set first to use this....
    if(ec > 10 && ec < 70000) {
      // in the case of multiple anode hits
      // save only the first anode hit info
      if(!ahit[pix]) {	
	      ahit[pix] = 1;
        if(dyhit)
          dyhihit   = 1; // dynode and at least one anode
	      // get QDC sums
	      std::vector<int> qdcSums = ddashit.GetQDCSums();
        if(qdcSums.size() > 0) { // if the QDC sum vector isn't empty, set the energy
          aenergy_qdc[pix] = qdcSums[0] - qdcSums[2];
	        asum_qdc += aenergy_qdc[pix];
        }
	      aecal[pix] = ec;
	      asum += ec;
	      atime[pix] = time;

        //amult++;
      }
      //pixelmult[pix]++; // but do increment pixel mult
    } // end threshold and uld check
    
    // reset overflows to have a high energy
  } // end coincidence check
}


void TPSPMT::FitAnodePosition()
{
  // check to see if all the QDC sums are set
  static int bad =0;
  static int calls=0;
  calls++;
  bool check = true;
  for(int i=0; i<NANODES; i++) { // really hacky check to see if all the energies are good
    if(aenergy_qdc[i] > 0) {
      check &= true;
    } else {
      check &= false;
    }
  }

  // if all the QDC sums are present calculate the position
  // position set to error values if out of range or not all
  // QDC sums are present for the current event
  if(check == true) {
    // QDC 2 not used
    double xa = aenergy_qdc[0];
    double xb = aenergy_qdc[1];
    double ya = aenergy_qdc[2];
    double yb = aenergy_qdc[3];

      xpos = npspmt_utk*(yb + xa)/asum_qdc;
      ypos = npspmt_utk*(xa + xb)/asum_qdc;

    if(area>0) {
      //printf("OLD    %.2f , %.2f    \n",xpos,ypos);
      xpos = 8*((6*(ya + xb)/asum_qdc)-3)/(1-0.45*pow(area/1e6, 0.6))+24;
      ypos = 8*((6*(xa + xb)/asum_qdc)-3)/(1-0.2*pow(area/1e6, 0.6))+24;
      //printf("NEW    %.2f , %.2f    \n\n",xpos,ypos);
    } //} else {
    //}
//xpos =((6*(ya+xb)/asum_qdc)-3)/(1-0.45*pow(area/1e6,0.6));

    if(xpos < 0 || xpos > npspmt_utk)
      xpos = ERR_BADPOS;
    if(ypos < 0 || ypos > npspmt_utk)
      ypos = ERR_BADPOS;
  } else {
    xpos = ERR_QDCMULT;
    ypos = ERR_QDCMULT;
  }
  
  if(xpos == ERR_QDCMULT ||  //if the qdc aren't there, true the energy filter...
     ypos == ERR_QDCMULT) {

    double xa = aenergy[0];
    double xb = aenergy[1];
    double yb = aenergy[3];
    xpos = npspmt_utk*(yb + xa)/asum;
    ypos = npspmt_utk*(xa + xb)/asum;
  }

  if(xpos == ERR_BADPOS || 
     ypos == ERR_BADPOS ) { 
      bad++;
      printf(RED "BAD POSITION!!\t %i / %i  %.2f \n" RESET_COLOR,bad,calls,(float(bad)/(float)calls)); 
      fflush(stdout);
  }

}




