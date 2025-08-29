#ifndef __TFDSI_H__
#define __TFDSI_H__

#include <TObject.h>

#include <TClock.h>
#include <TPSPMT.h>
#include <TPIN.h>
#include <TPID.h>
#include <TClover.h>
#include <TVandle.h>

#include <TOFCorrector.h>

class TFDSi : public TObject{
  public:
    TFDSi();
    ~TFDSi();

    void Copy(TFDSi &other) const;

    void Reset();
    int EventType() const;

    double GetX() const { return fLowGain1.xpos; } 
    double GetY() const { return fLowGain1.ypos; } 

    bool DBox() const;

    static void SetTOFCorrector(std::string fname);

    double GetTOF() const;

  //private:
    TClock fClock;
    TPID   fPID;

    TPSPMT fHighGain1;
    TPSPMT fHighGain2;
    TPSPMT fLowGain1;
    TPSPMT fLowGain2;

    TPIN nPin1;  // MSX40
    TPIN nPin2;  // MSX100
    TPIN gPin1;
    TPIN gPin2;

    TPIN nVetoF;
    TPIN nVetoR;
    TPIN gVetoF;
    TPIN gVetoR;

    TPIN nSIPMT; // 2
    TPIN nSIPM;  // 4
    TPIN gSIPMT; //231 Cross Scint B2  (2)
    TPIN gSIPM;  //232 Cross Scint T2  (4)

    TPIN nSCLT;
    TPIN gSCLT;

    TClover fClover;
    TVandle fVandle;

    static TOFCorrector *fTOFCorrector;

    int fEventType;
    std::vector<int> fHitMap;

  ClassDef(TFDSi,1)
};

#endif
