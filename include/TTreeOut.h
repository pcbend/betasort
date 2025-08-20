#ifndef __TTREEOUT_H__
#define __TTREEOUT_H__

#include <mutex>
#include <string>
#include <vector>

class TFDSi;
class TPID;

class TList;

class TTreeOut {
  
  private:
    TTreeOut();
    static TTreeOut *fTreeOut;  
    static std::mutex fTreeOutMutex;

  public:
    TTreeOut(const TTreeOut&) = delete;
    TTreeOut& operator=(const TTreeOut&) = delete;

    static TTreeOut *Get();
    ~TTreeOut();
    static void SetNoTree() { fNoTree = true; }

    void TreeLoop(); // output tree loop
    std::string Status() const;

    bool LoopRunning()  const { return fLoopRunning; }

    static void SetRun(int run,int subrun);


    void MakeHistograms(TFDSi&,std::vector<TPID>&) const;

  private:
    bool fLoopRunning;
    long fFillCounter;

    static int fRun;
    static int fSubRun;

    TList *fBlobs;
    TList *fTimes;


    static bool fNoTree;
};

#endif
