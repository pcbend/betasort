#include <TFDSi.h>

#include <Histogramer.h>
#include <globals.h>

#include <TFile.h>
#include <TDirectory.h>


TOFCorrector *TFDSi::fTOFCorrector = 0;

TFDSi::TFDSi() { Reset(); } 

TFDSi::~TFDSi() { }

void TFDSi::SetTOFCorrector(std::string fname) { 
  TDirectory *current = gDirectory;
  TFile *temp = TFile::Open(fname.c_str());
  TFDSi::fTOFCorrector = (TOFCorrector*)temp->Get("TOFCorrector");
  temp->Close();
  current->cd();
}

double TFDSi::GetTOF() const {
  if(!TFDSi::fTOFCorrector)
    return fPID.tof;
  double tof = TFDSi::fTOFCorrector->Correct(fPID.tof,fClock.initial/1e9);
  return tof;

}


void TFDSi::Copy(TFDSi &other) const {

  fClock.Copy(other.fClock);
  fPID.Copy(other.fPID);

  fHighGain1.Copy(other.fHighGain1);
  fHighGain2.Copy(other.fHighGain2);
  fLowGain1.Copy(other.fLowGain1);
  fLowGain2.Copy(other.fLowGain2);

  nPin1.Copy(other.nPin1);  // MSX40
  nPin2.Copy(other.nPin2);  // MSX100
  gPin1.Copy(other.gPin1);
  gPin2.Copy(other.gPin2);

  nVetoF.Copy(other.nVetoF);
  nVetoR.Copy(other.nVetoR);
  gVetoF.Copy(other.nVetoF);
  gVetoF.Copy(other.nVetoF);

  nSIPMT.Copy(other.nSIPMT);
  nSIPM.Copy(other.nSIPM);
  gSIPMT.Copy(other.gSIPMT);
  gSIPM.Copy(other.gSIPM);

  nSCLT.Copy(other.nSCLT);
  gSCLT.Copy(other.gSCLT);

  fClover.Copy(other.fClover);

  other.fEventType = fEventType;
  other.fHitMap    = fHitMap;

}

void TFDSi::Reset() {

  fClock.Reset();
  fPID.Reset();

  fHighGain1.Reset();
  fHighGain2.Reset();
  fLowGain1.Reset();
  fLowGain2.Reset();

  nPin1.Reset();  // MSX40
  nPin2.Reset();  // MSX100
  gPin1.Reset();
  gPin2.Reset();

  nVetoF.Reset();
  nVetoR.Reset();
  gVetoF.Reset();
  gVetoF.Reset();

  nSIPMT.Reset();
  nSIPM.Reset();
  gSIPMT.Reset();
  gSIPM.Reset();

  nSCLT.Reset();
  gSCLT.Reset();

  fClover.Reset();

  fEventType = -1;
  fHitMap.clear();

}


/****************************************                                                                             * Correlation flags:                   *
 * ==================================== *
 * 4  - implant good position           *
 * 8  - implant bad position            *
 * 12 - decay good position             *
 * 16 - decay bad position              *
 * 20 - light ion good position         *
 * 24 - light ion bad position
 * 28 - decay before ion (BAD!)         *
 * 32 - good correlation                *
 * 44 - no implants to correlate        *
 * 56 - Clover only     *
 * 96 - unknown event                   *
 * ==================================== *
 * 99 - Reset correlation aray          * 
 ****************************************/
int TFDSi::EventType() const { 
  int condition = 96;

  // signal flags
  bool hasPSPMT = false;
  bool hasPSPMTLow = false;      // PSPMT low gain (ions)
  bool hasPSPMTHigh = false;     // PSPMT high gain (decays)
  bool hasPin = false;           // any PIN hit
  bool hasPin01 = false;
  bool hasPin02 = false;
  bool hasPinImplant = false;    // implant-like events above implant threshold
  bool hasPin01Implant = false;
  bool hasPin02Implant = false;
  bool hasPinLightIon = false;   // light ion events below implant threshold
  bool hasPin01LightIon = false;
  bool hasPin02LightIon = false;
  bool hasClover = false;
  bool hasGoodPosition = false;
  //... and others (clover, LaBr3, etc.)
 
  // event identification
  bool isImplant = false;
  bool isDecay = false;
  bool isCloverOnly = false;
  bool isLightIon = false;
  bool isUnidentified = false;
  //... and others (clover only, LaBr3 only etc.)
 
  // PSPMT positions
  //int xPos = -1;
  //int yPos = -1;

  //ions
  if(fLowGain1.Hit()) 
    hasPSPMTLow = true;

  //decays
  if(fHighGain1.Hit())
    hasPSPMTHigh = true;

  //pin 1 implant vs light ion.
  Histogramer::fill("gPin1",60000,0,60000,gPin1.Ecal(),
                            2,0,2,gPin1.Hit());
  if(gPin1.Hit()) {
    hasPin01 = true;
    if(gPin1.Ecal() > 500) // implant threshold on pin1
      hasPin01Implant = true;
    else 
      hasPin01LightIon = true;
  }

  //pin 2 implant vs light ion.
  Histogramer::fill("gPin2",60000,0,60000,gPin2.Ecal(),
                            2,0,2,gPin2.Hit());
  if(gPin2.Hit()) {
    hasPin02 = true;
    if(gPin2.Ecal() > 500) // implant threshold on pin1
      hasPin02Implant = true;
    else 
      hasPin02LightIon = true;
  }

  //if(fClover.Hit()) 
  //  hasClover = true;
  
  if(hasPin01 || hasPin02)                 {hasPin = true;}         // any PIN is hit    
  if(hasPSPMTLow || hasPSPMTHigh)          {hasPSPMT = true;}       // any PSPMT
  if(hasPin01Implant && hasPin02Implant)   {hasPinImplant = true;}  // implants only
  if(hasPin01LightIon && hasPin02LightIon) {hasPinLightIon = true;} // light ions only


  if(fLowGain1.xpos>0 && fLowGain1.xpos < npspmt_utk &&
     fLowGain1.ypos>0 && fLowGain1.ypos < npspmt_utk) {
    hasGoodPosition = true;
  }

  if(hasPSPMTLow  && hasPinImplant && !nVetoR.Hit()) {
    isImplant = true;
  }
                                                                     // here nSIPM (is cross scint.)
  //if(hasPSPMT && (hasPinLightIon || nVetoF.Hit() || nVetoR.Hit() || nSIPM.Hit()) )
  //if(hasPSPMT && (hasPinLightIon || (nVetoF.Hit() && nVetoR.Hit()))) // || nSIPMT.Hit()) )
  //if(hasPSPMT && (hasPinLightIon || nVetoF.Hit() || nVetoR.Hit() || gSIPM.Hit()) )
    //(hasPinLightIon || nVetoF.Hit() || nVetoR.Hit() || gSIPM.Hit()) )
  //if(hasPSPMT && ((nVetoF.Hit() || nVetoR.Hit()) || hasPinLightIon || (!hasPin && gSIPM.Hit())))
  //  isLightIon = true;                                                       //232.

  if(hasPSPMTHigh && !hasPin && !isLightIon) { 
    isDecay = true;
  }

  if(hasPinLightIon)
    isLightIon = true;

  if(gVetoF.Hit() && gVetoR.Hit())
    isLightIon = true;

  if(isDecay && (gSIPM.Hit() || nSIPM.Hit()) )
    isLightIon = true;

  if(!hasPin && !hasPSPMT && hasClover)
    isCloverOnly = true;

  if(!isImplant && !isDecay && !isLightIon && !isCloverOnly) 
    isUnidentified = true;



  if(isImplant) {
    // * 4  - implant good position           *
    // * 8  - implant bad position            *
    if(hasGoodPosition) {
      condition = 4;
    } else {
      condition = 8;
    }
  }
  if(isDecay) {
    // * 12 - decay good position             *
    // * 16 - decay bad position              *
    //if(fHighGain1.xpos>0 && fHighGain1.xpos < npspmt_utk &&
    //   fHighGain1.ypos>0 && fHighGain1.ypos < npspmt_utk) {
    if(hasGoodPosition) {
      condition = 12;
    } else {
      condition = 16;
    }
  }
  if(isLightIon) {
    if(hasGoodPosition) {
      condition = 20;
    } else {
      condition = 24;
    }
  }
  if(isCloverOnly) 
    condition = 56;

  //printf("isImplant : isDecay : isLightIon    %i %i %i  Condition = %i\n",isImplant?1:0,isDecay?1:0,isLightIon?1:0,condition);
  //if(condition==) { 
/*
    if(condition==4) printf(RED);
    if(condition==12) printf(BLUE);
    if(condition==20 || condition==24) printf(DYELLOW);

    printf("condition         = %i\n", condition);
    printf("hasPSPMT          = %i\n",hasPSPMT         ? 1:0);
    printf("hasPSPMTLow       = %i\n",hasPSPMTLow      ? 1:0);  //  PSPMT         low     gain    (ions)
    printf("hasPSPMTHigh      = %i\n",hasPSPMTHigh     ? 1:0);  //  PSPMT         high    gain    (decays)
    printf("hasPin            = %i\n",hasPin           ? 1:0);  //  any           PIN     hit
    printf("hasPin01          = %i\n",hasPin01         ? 1:0);
    printf("hasPin02          = %i\n",hasPin02         ? 1:0);
    printf("hasPinImplant     = %i\n",hasPinImplant    ? 1:0);  //  implant-like  events  above   implant   threshold
    printf("hasPin01Implant   = %i\n",hasPin01Implant  ? 1:0);
    printf("hasPin02Implant   = %i\n",hasPin02Implant  ? 1:0);
    printf("hasPinLightIon    = %i\n",hasPinLightIon   ? 1:0);  //  light         ion     events  below     implant    threshold
    printf("hasPin01LightIon  = %i\n",hasPin01LightIon ? 1:0);
    printf("hasPin02LightIon  = %i\n",hasPin02LightIon ? 1:0);
    printf("hasClover         = %i\n",hasClover        ? 1:0);
    printf("hasGoodPosition   = %i\n",hasGoodPosition  ? 1:0);
    printf("isImplant         = %i\n",isImplant        ? 1:0);
    printf("isDecay           = %i\n",isDecay          ? 1:0);
    printf("isCloverOnly      = %i\n",isCloverOnly     ? 1:0);
    printf("isLightIon        = %i\n",isLightIon       ? 1:0);
    printf("isUnidentified    = %i\n",isUnidentified   ? 1:0);

    printf("nVetoF            = %i\n",nVetoF.Hit());
    printf("gVetoF            = %i\n",gVetoF.Hit());
    printf("nVetoR            = %i\n",nVetoR.Hit());
    printf("gVetoR            = %i\n",gVetoR.Hit());

    printf("nSIPM             = %i\n",nSIPM.Hit());
    printf("nSIPMT            = %i\n",nSIPMT.Hit());
    printf("gSIPM             = %i\n",gSIPM.Hit());
    printf("gSIPMT            = %i\n",gSIPMT.Hit());

    printf(RESET_COLOR);

    printf("\n\n");
  //}

*/
  return condition;
}

bool TFDSi::DBox() const { 
  bool dBoxFired = false;
  for(auto id : fHitMap) {
    if(id>=240 && id <=255) { //diag boxes from seperator
      dBoxFired = true;
      break;
    }
    if(id>=231 && id<=233) { //cross scint 
      dBoxFired = true;
      break;
    }
  }
  return dBoxFired;
}



