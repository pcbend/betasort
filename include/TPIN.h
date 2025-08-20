#ifndef TPIN_H
#define TPIN_H

//#include "DDASRootFitHit.h"

class ddasHit;

#include <TObject.h>

class TPIN: public TObject {
  public:
    TPIN();
    TPIN(ddasHit &hit);
    ~TPIN();

    void Reset();
    void Copy(TPIN &other) const;
    void Unpack(ddasHit &hit);
    

    double Energy() const { return fEnergy; }
    double Ecal()   const { return fEcal;   }
    double Time()   const { return fTime;   }
    int    Hit()    const { return fHit;    }

  private:
    double fEnergy;
    double fEcal;
    double fTime;
    int fHit;
  
  ClassDef(TPIN,4);
};

#endif
