

#include <Pipeline.h>

#include <cstdio>
#include <thread>

#include <TFile.h>

//loops
#include <TAnalyzer.h>
#include <Unpacker.h> 
#include <TCorrelator.h>
#include <TTreeOut.h>   
                        
#include <Histogramer.h>
                        
#include <utils.h>   
#include <globals.h>

namespace {
  bool RunAnalyzer(OutputLevel level) {
    return level == OutputLevel::Analyzer ||
           level == OutputLevel::Unpacker ||
           level == OutputLevel::Correlator ||
           level == OutputLevel::Tree; 
  }
  
  bool RunUnpacker(OutputLevel level) {
    return level == OutputLevel::Unpacker ||
           level == OutputLevel::Correlator ||
           level == OutputLevel::Tree; 
  }

  bool RunCorrelator(OutputLevel level) {
    return level == OutputLevel::Correlator ||
           level == OutputLevel::Tree; 
  }
  
  bool RunTreeOut(OutputLevel level) {
    return level == OutputLevel::Tree; 
  }

}



Pipeline::Pipeline(BetaOptions options) {
  fOptions = options;
}

void Pipeline::AddFile(TFile *file)  {
  if(!file) return;
  TObject *obj = file->Get("tree");                                                                                     
  if(obj && obj->InheritsFrom("TTree")) {
    fFilesToSort.push_back(file);
  }
}

void Pipeline::Run() {
  if(fFilesToSort.empty()) return;

  /// setup ////
  //////////////
  TClover::ReadCalFile("cals/CloverInit_1343.txt");
  if(fOptions.tofFiles.size()) 
    TFDSi::SetTOFCorrector(fOptions.tofFiles.at(0));

    for(auto file : fFilesToSort) { //bad things will currently happen if ever more then one file, tanalyzer is a singleton.
      TAnalyzer::AddFile(file);
    }            

  TFile *file =fFilesToSort[0];
  int run,subrun;
  getRunNumber(file->GetName(),run,subrun);
  Histogramer::Get()->SetRun(run,subrun);




  /// loops ////
  //////////////
  //TAnalyzer::Get(&fStats);


  std::thread root2DDAS;
  std::thread ddas2FDSi;
  std::thread FDSi2CorrFDSi;
  std::thread CorrFDSi2Tree;

  if(RunAnalyzer(fOptions.outputLevel)) {
    if(fOptions.outputLevel>=OutputLevel::Unpacker)
      TAnalyzer::Get()->SetForwardToNext(true);
    root2DDAS = std::thread(&TAnalyzer::Loop,TAnalyzer::Get());
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  if(RunUnpacker(fOptions.outputLevel)) {
    if(fOptions.outputLevel>=OutputLevel::Correlator)
      Unpacker::Get()->SetForwardToNext(true);
    ddas2FDSi    = std::thread(&Unpacker::Unpack,Unpacker::Get());
  }

  if(RunCorrelator(fOptions.outputLevel)) {
    if(fOptions.outputLevel>=OutputLevel::Tree)
      TCorrelator::Get()->SetForwardToNext(true);
    FDSi2CorrFDSi = std::thread(&TCorrelator::Correlate,TCorrelator::Get());
  }

  if(RunTreeOut(fOptions.outputLevel)) {
    if(fOptions.noTree)
      TTreeOut::SetNoTree();
  
    TTreeOut::Get()->SetRun(run,subrun);
    CorrFDSi2Tree = std::thread(&TTreeOut::TreeLoop,TTreeOut::Get());
  }

  LoopProgress loopprogress(fOptions.outputLevel);
  std::thread progress(&LoopProgress::Show,&loopprogress);

  if(root2DDAS.joinable())     root2DDAS.join();
  if(ddas2FDSi.joinable())     ddas2FDSi.join();
  if(FDSi2CorrFDSi.joinable()) FDSi2CorrFDSi.join();
  if(CorrFDSi2Tree.joinable()) CorrFDSi2Tree.join();
  progress.join();

  Histogramer::Get()->Close();
  fStats.Print();
}

LoopProgress::LoopProgress(OutputLevel level) : fOutputLevel(level) { }  
 
LoopProgress::~LoopProgress() { }  
 

/* 
void LoopProgress::Show() {  
  printf("\n\n"); //make two cleared lines. 
  while(true) {  
    printf(CLEAR_LINE); 
    printf(CURSOR_UP); printf(CLEAR_LINE); 
    printf(CURSOR_UP); printf(CLEAR_LINE); 
    printf(CURSOR_UP); printf(CLEAR_LINE); 
    printf(CURSOR_UP); printf(CLEAR_LINE); 
     
    printf("%s\n",TAnalyzer::Get()->Status().c_str());    //TAnalyzer (reads from file) 
    printf("%s\n",Unpacker::Get()->Status().c_str());     //Unpacker  (ddasHit -> FDSi)  //defines EventType 
    printf("%s\n",TCorrelator::Get()->Status().c_str());  //Correlator(FDSi -> Correlated FDSi)  
    printf("%s\n",TTreeOut::Get()->Status().c_str()); 
 
    std::this_thread::sleep_for(std::chrono::seconds(1)); // delay 1 seconds 
    if(!TAnalyzer::Get()->LoopRunning() && TAnalyzer::Get()->qsize()==0  &&  
       !Unpacker::Get()->LoopRunning() && Unpacker::Get()->qsize()==0    && 
       !TCorrelator::Get()->LoopRunning() && TCorrelator::Get()->qsize()==0    && 
       !TTreeOut::Get()->LoopRunning() ) 
      break; 
  } 
    printf(CLEAR_LINE); 
    printf(CURSOR_UP); printf(CLEAR_LINE); 
    printf(CURSOR_UP); printf(CLEAR_LINE); 
    printf(CURSOR_UP); printf(CLEAR_LINE); 
    printf(CURSOR_UP); printf(CLEAR_LINE); 
     
    printf("%s\n",TAnalyzer::Get()->Status().c_str());    //TAnalyzer (reads from file) 
    printf("%s\n",Unpacker::Get()->Status().c_str());     //Unpacker  (ddasHit -> FDSi)  //defines EventType 
    printf("%s\n",TCorrelator::Get()->Status().c_str());  //Correlator(FDSi -> Correlated FDSi)  
    printf("%s\n",TTreeOut::Get()->Status().c_str()); 
    printf("\n"); 
  return; 
} 
*/

/*
void LoopProgress::Show() {
  printf("\n\n");

  auto done = [this]() {
    bool analyzerDone =
      !TAnalyzer::Get()->LoopRunning();

    bool unpackerDone =
      fOutputLevel == OutputLevel::Analyzer ||
      (!Unpacker::Get()->LoopRunning() && TAnalyzer::Get()->qsize() == 0);

    bool correlatorDone =
      fOutputLevel == OutputLevel::Analyzer ||
      fOutputLevel == OutputLevel::Unpacker ||
      (!TCorrelator::Get()->LoopRunning() && Unpacker::Get()->qsize() == 0);

    bool treeDone =
      fOutputLevel != OutputLevel::Tree ||
      !TTreeOut::Get()->LoopRunning();

    return analyzerDone && unpackerDone && correlatorDone && treeDone;
  };

  while(true) {
    printf(CLEAR_LINE);
    printf(CURSOR_UP); printf(CLEAR_LINE);
    printf(CURSOR_UP); printf(CLEAR_LINE);
    printf(CURSOR_UP); printf(CLEAR_LINE);
    printf(CURSOR_UP); printf(CLEAR_LINE);

    printf("%s\n", TAnalyzer::Get()->Status().c_str());

    if(fOutputLevel != OutputLevel::Analyzer)
      printf("%s\n", Unpacker::Get()->Status().c_str());

    if(fOutputLevel == OutputLevel::Correlator ||
       fOutputLevel == OutputLevel::Tree)
      printf("%s\n", TCorrelator::Get()->Status().c_str());

    if(fOutputLevel == OutputLevel::Tree)
      printf("%s\n", TTreeOut::Get()->Status().c_str());

    std::this_thread::sleep_for(std::chrono::seconds(1));

    if(done())
      break;
  }

  printf("\n");
}
*/

void LoopProgress::Show() {
  auto num_lines = [this]() {
    switch(fOutputLevel) {
      case OutputLevel::Analyzer:   return 1;
      case OutputLevel::Unpacker:   return 2;
      case OutputLevel::Correlator: return 3;
      case OutputLevel::Tree:       return 4;
    }
    return 4;
  };

  auto done = [this]() {
    bool analyzerDone =
      !TAnalyzer::Get()->LoopRunning();

    bool unpackerDone =
      fOutputLevel == OutputLevel::Analyzer ||
      (!Unpacker::Get()->LoopRunning() &&
       TAnalyzer::Get()->qsize() == 0);

    bool correlatorDone =
      fOutputLevel == OutputLevel::Analyzer ||
      fOutputLevel == OutputLevel::Unpacker ||
      (!TCorrelator::Get()->LoopRunning() &&
       Unpacker::Get()->qsize() == 0);

    bool treeDone =
      fOutputLevel != OutputLevel::Tree ||
      !TTreeOut::Get()->LoopRunning();

    return analyzerDone && unpackerDone && correlatorDone && treeDone;
  };

  auto print_status = [this]() {
    printf("%s\n", TAnalyzer::Get()->Status().c_str());

    if(fOutputLevel != OutputLevel::Analyzer)
      printf("%s\n", Unpacker::Get()->Status().c_str());

    if(fOutputLevel == OutputLevel::Correlator ||
       fOutputLevel == OutputLevel::Tree)
      printf("%s\n", TCorrelator::Get()->Status().c_str());

    if(fOutputLevel == OutputLevel::Tree)
      printf("%s\n", TTreeOut::Get()->Status().c_str());
  };

  auto clear_status = [&num_lines]() {
    int n = num_lines();

    for(int i = 0; i < n; ++i) {
      printf(CURSOR_UP);
      printf(CLEAR_LINE);
    }
  };

  printf("\n");

  print_status();
  fflush(stdout);

  while(true) {
    std::this_thread::sleep_for(std::chrono::seconds(1));

    clear_status();
    print_status();
    fflush(stdout);

    if(done())
      break;
  }

  printf("\n");
}





