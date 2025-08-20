#ifndef __TANALYZER_H__
#define __TANALYZER_H__

#include <vector>
#include <queue>
#include <mutex>

#include<TObject.h>

#include <ddasHit.h>

class TFile;
class TChain;

class TAnalyzer : public TObject {

  private:
    TAnalyzer();
    static TAnalyzer *fAnalyzer;  
    static std::mutex fAnalyzerMutex;
  public:
    TAnalyzer(const TAnalyzer&) = delete;
    TAnalyzer& operator=(const TAnalyzer&) = delete;

    static void AddFile(TFile *file);
    static void AddFile(std::string file);

    static TAnalyzer *Get();
    ~TAnalyzer();

    void ClearData();
    void SetBranches();

    bool Next(std::vector<ddasHit> &hits);
    void Progress(int mod,bool newline=false);
    std::string Status() const;

    void Loop();
    bool LoopRunning() const { return fLoopRunning; }

    bool GetBackwards() const { return fLoopBackwards; }
    void SetBackwards();

    TChain *GetChain() { return gChain; }

  private:
    //tree data
    unsigned long long              evID;
    int                             multi;
    std::vector<int>*               id;
    std::vector<double>*            e;
    std::vector<double>*            e_t;
    std::vector<int>*               cfd;
    std::vector<int>*               pileup;
    std::vector<std::vector<int> >* qdc; 
    std::vector<int>*               traceLength;
    std::vector<std::vector<unsigned short> >* trace;

  private:
    TChain *gChain;
    std::string fFilename;
    size_t current;
    size_t entries;

    bool fLoopBackwards;

  private:
    std::queue<std::vector<ddasHit> > fQueue;
    std::mutex fQueueMutex; 
    bool fLoopRunning;

  public:
    std::queue<std::vector<ddasHit> > GetQ() { return fQueue; }
    size_t qsize() const { return fQueue.size(); }

    void push(std::vector<ddasHit> &hits);
    std::vector<ddasHit> pop(); 

  ClassDef(TAnalyzer,0)
};






#endif
