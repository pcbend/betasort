
#ifndef TVANDLE_H
#define TVANDLE_H

#include <ddasHit.h>

#include "TObject.h"
#include "TRandom3.h"

#include <map>

/*
 * Vandle class defintion
 */

class TVandleHit : public TObject {
  public:
    TVandleHit();
    TVandleHit(const TVandleHit&);
    ~TVandleHit();

    void Copy(TVandleHit &other) const;
    void Reset();

  //private:
    int    fId;
    double fTime;
    double fCfdTime;
    double fEnergy;
    double fEcal;

  ClassDef(TVandleHit,4)
};


class TVandle : public TObject {
  public:
    TVandle();
    ~TVandle();

    //static void ReadCalFile(std::string name);
    void Copy(TVandle &other) const;
    void Reset();
    void Unpack(ddasHit &chanhit, int det);
    //void AddBack();

//  private:
    std::vector<TVandleHit> fHits;

  ClassDef(TVandle,1);
};

#endif
