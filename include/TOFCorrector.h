#ifndef __TOFCORRECTOR_H__
#define __TOFCORRECTOR_H__

#include <TObject.h>

#include <string>

#define EXPECTED_PEAKS 4
#define TOF_WIDTH 30
#define PEAK1 -160.0
#define PEAK2 -155.0
#define PEAK3 -152.0
#define PEAK4 -150.0

class TSpline3;
class TH2;

class TOFCorrector : public TObject {
  public:
    TOFCorrector();
    ~TOFCorrector();

    void MakeCorrectionFile(std::string fname="",TH2 *tof_time=0); // 

    void Draw() const;

    double Correct(double tof,double time,int peak=0) const;


  private:
    int fNPeaks;
    std::vector<int>       fExpectedBin;
    std::vector<double>    fExpectedValue;
    //std::vector<double>    fCorrectedValue;
    std::vector<TSpline3*> fSplines;    

    TH2 *fTofTime;

  ClassDef(TOFCorrector,1)
};


#endif

