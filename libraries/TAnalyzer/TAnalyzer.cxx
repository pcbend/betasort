
#include <TAnalyzer.h>
#include <Histogramer.h>

#include <cstdio>
#include <string>

#include <TFile.h>
#include <TChain.h>

TAnalyzer *TAnalyzer::fAnalyzer = 0;
std::mutex TAnalyzer::fAnalyzerMutex;




TAnalyzer *TAnalyzer::Get() {
  if(!fAnalyzer) { 
    std::lock_guard<std::mutex> lock(fAnalyzerMutex);
    fAnalyzer = new TAnalyzer();
  }
  return fAnalyzer;
}


void TAnalyzer::AddFile(TFile *file) { 
  printf("TFile: %p\n",file);
  AddFile(file->GetName());
}

void TAnalyzer::AddFile(std::string file) { 
  printf("adding file %s to chain.\n\n\n\n",file.c_str());
  Get()->GetChain()->Add(file.c_str());
}


TAnalyzer::TAnalyzer() {
  //printf("\nFILE: %p\n\n",file); fflush(stdout); 
  //fFilename = file->GetName();
  //printf("TAnalyzer creator: %s\n",fFilename.c_str());
  gChain = new TChain("tree");;

  fLoopRunning = false;

  fLoopBackwards   = false;

  current = 0;

  id          = 0;
  e           = 0;
  e_t         = 0;
  cfd         = 0;
  pileup      = 0;
  qdc         = 0; 
  traceLength = 0;
  trace       = 0;
  
  //disable automatic directory association
  //TH1::AddDirectory(false);

}

void TAnalyzer::SetBackwards() {
  fLoopBackwards = true;
  current = entries-1;
}


TAnalyzer::~TAnalyzer() { } 

void TAnalyzer::ClearData() {
  if(id)          id->clear();         
  if(e)           e->clear();          
  if(e_t)         e_t->clear();        
  if(cfd)         cfd->clear();        
  if(pileup)      pileup->clear();     
  if(qdc)         qdc->clear();        
  if(traceLength) traceLength->clear();
  if(trace)       trace->clear();      
}

void TAnalyzer::SetBranches() {
  
  gChain->SetBranchAddress("evID",&evID);
  gChain->SetBranchAddress("multi",&multi);                                                                                                                                            
  gChain->SetBranchAddress("id",&id);
  gChain->SetBranchAddress("e",&e);
  gChain->SetBranchAddress("e_t",&e_t);
  gChain->SetBranchAddress("cfd",&cfd);
  gChain->SetBranchAddress("pileup",&pileup);
  gChain->SetBranchAddress("qdc",&qdc);
  
  gChain->SetBranchAddress("traceLength",&traceLength);
  gChain->SetBranchAddress("trace",&trace);

  //printf("after branch set:\n");
  //printf("\tid          = %p\n",id);         
  //printf("\te           = %p\n",e);          
  //printf("\te_t         = %p\n",e_t);        
  //printf("\tcfd         = %p\n",cfd);       
  //printf("\tpileup      = %p\n",pileup);   
  //printf("\tqdc         = %p\n",qdc);        
  //printf("\ttraceLength = %p\n",traceLength);
  //printf("\ttrace       = %p\n",trace);      
}

bool TAnalyzer::Next(std::vector<ddasHit> &hits) {
  
  hits.clear();
  
  //Progress(15000);
  //if(current==entries)
  //  Progress(true);

  ClearData();

  if(current<entries && current>=0) {
    if(fLoopBackwards)
      gChain->GetEntry(current--);
    else
      gChain->GetEntry(current++);
    for(int i=0;i<multi;i++) {
      ddasHit hit;
      hit.setEvId(evID);
      hit.setId(id->at(i));
      hit.setEnergy(e->at(i));
      hit.setTime(e_t->at(i));
      hit.setCFD(cfd->at(i));
      hit.setQDC(qdc->at(i));
      hit.setTrace(trace->at(i));
      hits.push_back(hit);
    }
    return true;
  }
  return false;
}



void TAnalyzer::Progress(int mod,bool newline) {
  if((current%mod)==0 || newline) {
    printf(" on entry  %lu / %lu       \r",current,entries);
  }
  if(newline)
    printf("\n");
  fflush(stdout);
}

std::string TAnalyzer::Status() const { 
  std::string s = Form("TAnalyzer:  on entry  %lu / %lu q[%lu]",current,entries,qsize());
  return  s;
}

void TAnalyzer::Loop() {
  entries = gChain->GetEntries();
  SetBranches();

 std::vector<ddasHit> hits;
 fLoopRunning = true;
 while(Next(hits)) {
    //fQueue.push(hits);
    push(hits);
  }
  fLoopRunning = false;
}


void TAnalyzer::push(std::vector<ddasHit> &hits) {
  std::lock_guard<std::mutex> lock(fQueueMutex);
  fQueue.push(hits); 
  return;
}
    
std::vector<ddasHit> TAnalyzer::pop() { 
  std::lock_guard<std::mutex> lock(fQueueMutex);
  if(fQueue.size()!=0) {
    std::vector<ddasHit> hits = fQueue.front();
    fQueue.pop();
    return hits;
  }
  std::vector<ddasHit> hits;
  return hits;
}



