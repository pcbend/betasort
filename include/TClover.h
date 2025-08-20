
#ifndef TCLOVER_H
#define TCLOVER_H

#include <ddasHit.h>

#include "TObject.h"
#include "TRandom3.h"

#include <map>

/*
 * Clover class defintion
 */

class TCloverHit : public TObject {
  public:
    TCloverHit();
    TCloverHit(const TCloverHit&);
    ~TCloverHit();

    void Copy(TCloverHit &other) const;
    void Reset();

  //private:
    int    fId;
    double fTime;
    double fCfdTime;
    double fEnergy;
    double fEcal;

  ClassDef(TCloverHit,4)
};


class TClover : public TObject {
  public:
    TClover();
    ~TClover();

    static void ReadCalFile(std::string name);
    void Copy(TClover &other) const;
    void Reset();
    void Unpack(ddasHit &chanhit, int det);
    //void AddBack();

//  private:
    std::vector<TCloverHit> hits;
    int hit;
    static std::map<int,std::vector<double> > fCals;

  ClassDef(TClover,4);
};

#endif
