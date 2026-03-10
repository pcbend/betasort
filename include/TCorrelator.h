#ifndef __TCORRELATOR_H__
#define __TCORRELATOR_H__

#include <mutex>
#include <queue>
#include <vector>

#include <TFDSi.h>
#include <TPID.h>

struct PendingDecay {
  TFDSi  fdsi;
  double time;
};


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
    std::vector<TImplant> GetImplants() { return fCorrelatedImplants; }

  private:
    std::queue<std::pair<TFDSi,std::vector<TImplant> > > fQueue;
    bool fLoopRunning;
    long fIn;
    long fOut;

  public:
    bool LoopRunning() const { return fLoopRunning; }
    std::queue<std::pair<TFDSi,std::vector<TImplant> > > GetQ() { return fQueue; }
    size_t qsize() const { return fQueue.size(); }

    void push(const TFDSi &fdsi,std::vector<TImplant> &corrImplants);
    std::pair<TFDSi,std::vector<TImplant> > pop(); 

    void Correlate();  // Correlation loop

    std::string Status();

    void ShowImplantVector(int dx=-1,int dy=-1) const;

  private:
    TPID fImplanted[npspmt_utk][npspmt_utk];
    std::vector<TPID> fImplantVector[npspmt_utk][npspmt_utk];  //TPID before correlation, TImplant after...
    
    double fBGWindow     = 1000.0;
    double fDecayWindow  = 5000.0;
    int fCorrWin = 3; // Correlation window size in mm
    
    std::vector<TImplant> fCorrelatedImplants;
    std::deque<PendingDecay> fPendingDecays;  

    void StoreImplant(const TFDSi& fdsi, double t);
    void StoreDecay(const TFDSi& fdsi, double t);

    void CorrelateOneDecay(const TFDSi &decay,double tdecay);

    int  PruneImplants(double current_time);
    int  FinalizeDecays(double current_time);
    int  FlushDecays();


};

#endif

