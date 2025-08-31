
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
    void Print(Option_t *opt="") const;

  //private:

    int    fId;
    double fTimeRight;
    double fTimeLeft;
    double fEnergyRight;
    double fEnergyLeft;
    double fEcal;

    std::vector<unsigned short> fTraceRight;
    std::vector<unsigned short> fTraceLeft;

    void DrawTrace(); 
    double GetQDC() const;

  ClassDef(TVandleHit,6)
};


class TVandle : public TObject {
  public:
    TVandle();
    ~TVandle();

    //static void ReadCalFile(std::string name);
    void Copy(TVandle &other) const;
    void Reset();
    void Print(Option_t *opt="") const;
    void Unpack(ddasHit &chanhit, int det=-1);
    //void AddBack();

//  private:
    std::vector<TVandleHit> fHits;

  ClassDef(TVandle,1);
};

#endif
