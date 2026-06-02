#ifndef __PIPELINE_H__
#define __PIPELINE_H__

#include <vector>
#include <iostream>

#include <BetaOptions.h>

class TFile;


struct StageStats {
  std::atomic<size_t> events{0};
  std::atomic<long long> ns{0};

  void Add(std::chrono::nanoseconds dt) {
    events++;
    ns += dt.count();
  }

  void Print(const std::string& name) const {
    auto e = events.load();
    auto t = ns.load();

    double sec = t / 1.0e9;
    double rate = sec > 0 ? e / sec : 0.0;

    std::cout << name
              << " events=" << e
              << " time=" << sec << " s"
              << " rate=" << rate << " events/s"
              << std::endl;
  }
};

struct PipelineStats {
  StageStats analyzer;
  StageStats unpacker;
  StageStats correlator;
  StageStats treeout;
  StageStats histograms;

  void Print() const {
    std::cout << "\n=== Pipeline timing ===\n";
    analyzer.Print("Analyzer");
    unpacker.Print("Unpacker");
    correlator.Print("Correlator");
    treeout.Print("TreeOut");
    histograms.Print("Histograms");
  }
};





class Pipeline {
  public:
    Pipeline(BetaOptions options);

    void AddFile(TFile *file);
    void Run();

  private:
    PipelineStats       fStats;
    BetaOptions         fOptions;
    std::vector<TFile*> fFilesToSort;

};


class LoopProgress {
  public: 
    LoopProgress();
    ~LoopProgress();
  
    void Show();
  
};






#endif
