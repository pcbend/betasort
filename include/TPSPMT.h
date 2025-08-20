#ifndef __TPSPMT_H__
#define __TPSPMT_H__


// implantation detectors
//const int nanodes = 256;
//const int npspmt = 16;
//const int NANODES = 4;
//const int npspmt_utk = 48;
//const int ndssd = 40;
//const int nsssd = 16;

#define NANODES 4
#define npspmt_utk 48

#include <TObject.h>

class ddasHit;

class TPSPMT : public TObject{
  public:
    TPSPMT();
    ~TPSPMT();

    void Reset();
    void UnpackDynode(ddasHit& ddashit);
    void UnpackAnode(ddasHit& ddashit,int pix);
    void FitAnodePosition();

    int Hit() const { return hit; }

    //void Print() const;
    void Copy(TPSPMT &other) const;

  //private:
    // dynode
    double dyenergy;         //unpacked
    double dyecal;           //unpacked
    double dytime;           
    //double dytimecfd;

    // Anode
    double aenergy[NANODES];
    double aecal[NANODES];
    double aenergy_qdc[NANODES];
    double asum;
    double asum_qdc;
    double atime[NANODES];
    //double atimecal[NANODES];
    double baseline;
    double area;

    // Anode position
    double xpos;
    double ypos;

    // hit flags
    int dyhit;
    int ahit[NANODES];
    int dyhihit   = 0; // dynode and at least one anode
    int hit; // dynode and at least one anode

  ClassDef(TPSPMT,3)
};

#endif
