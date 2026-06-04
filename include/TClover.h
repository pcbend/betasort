
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

class TAddbackHit : public TCloverHit {
  public:
    TAddbackHit();
    TAddbackHit(const TCloverHit& hit);
    ~TAddbackHit();

    void Add(const TCloverHit& hit);
    void Reset();

    bool CheckTime(const TCloverHit &other) const;

    int Mult() const { return fMult;}

  private:
    int fMult;

  ClassDef(TAddbackHit,0)
};

class TClover : public TObject {
  public:
    TClover();
    ~TClover();

    static void ReadCalFile(std::string name);
    void Copy(TClover &other) const;
    void Reset();
    void Unpack(const ddasHit &chanhit, int det);
    //void AddBack();
    void BuildAddback(); 


//  private:
    std::vector<TCloverHit> hits;
    std::vector<TAddbackHit> addbackHits; //!
    int hit;
    static std::map<int,std::vector<double> > fCals;




  ClassDef(TClover,4);
};

#endif
