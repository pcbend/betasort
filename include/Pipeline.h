#ifndef __PIPELINE_H__
#define __PIPELINE_H__

#include <vector>

#include <BetaOptions.h>

class TFile;

class Pipeline {
  public:
    Pipeline(BetaOptions options);

    void AddFile(TFile *file);
    void Run();


  private:
    BetaOptions fOptions;
    std::vector<TFile*> fFilesToSort;

};


class LoopProgress {
  public: 
    LoopProgress();
    ~LoopProgress();
  
    void Show();
  
};






#endif
