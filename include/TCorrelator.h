#ifndef __TCORRELATOR_H__
#define __TCORRELATOR_H__

#include <mutex>
#include <queue>
#include <vector>

#include <TFDSi.h>
#include <TPID.h>

class TCorrelator {

  private: 
    TCorrelator();
    static TCorrelator *fCorrelator;
    static std::mutex fCorrelatorMutex;

  public: 
    ~TCorrelator();
    static TCorrelator* Get();

    void Reset();
    void ClearImplants() { fCorrelatedImplants.clear(); }
    std::vector<TPID> GetImplants() { return fCorrelatedImplants; }

  private:
    std::queue<std::pair<TFDSi,std::vector<TPID> > > fQueue;
    bool fLoopRunning;
    long fIn;
    long fOut;

  public:
    bool LoopRunning() const { return fLoopRunning; }
    std::queue<std::pair<TFDSi,std::vector<TPID> > > GetQ() { return fQueue; }
    size_t qsize() const { return fQueue.size(); }

    void push(TFDSi &fdsi,std::vector<TPID> &corrImplants);
    std::pair<TFDSi,std::vector<TPID> > pop(); 

    void Correlate();  // Correlation loop

    std::string Status();

    void ShowImplantVector(int dx=-1,int dy=-1) const;

  private:
    TPID fImplanted[npspmt_utk][npspmt_utk];
    std::vector<TPID> fImplantVector[npspmt_utk][npspmt_utk];
    const int fCorrWin = 3; // Correlation window size in mm
    std::vector<TPID> fCorrelatedImplants;


  
};

#endif

