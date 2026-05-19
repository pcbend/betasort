

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





Pipeline::Pipeline(BetaOptions options) {
  fOptions = options;
}

void Pipeline::AddFile(TFile *file)  {
  fFilesToSort.push_back(file);
}

void Pipeline::Run() {
  printf(" i am sorting now...\n");

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


  //if(doLoopBackwards)
  //  TAnalyzer::Get()->SetBackwards();
  std::thread root2DDAS(&TAnalyzer::Loop,TAnalyzer::Get());



  LoopProgress loopprogress;
  std::thread progress(&LoopProgress::Show,loopprogress);
  //progress.detach();

  root2DDAS.detach();

  std::this_thread::sleep_for(std::chrono::seconds(1));

  std::thread ddas2FDSi(&Unpacker::Unpack,Unpacker::Get());
  ddas2FDSi.detach();

  std::thread FDSi2CorrFDSi(&TCorrelator::Correlate,TCorrelator::Get());
  FDSi2CorrFDSi.detach();

  if(fOptions.noTree)
    TTreeOut::SetNoTree();
  TTreeOut::Get()->SetRun(run,subrun);
  std::thread CorrFDSi2Tree(&TTreeOut::TreeLoop,TTreeOut::Get());
  CorrFDSi2Tree.detach();

  progress.join();

  Histogramer::Get()->Close();

}

LoopProgress::LoopProgress() { }  
 
LoopProgress::~LoopProgress() { }  
 
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
















