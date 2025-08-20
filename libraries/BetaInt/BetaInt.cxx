#include <iostream>
#include <thread>
#include <chrono>


#include <TFile.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TEnv.h>
#include <TSystem.h>

#include <BetaInt.h>
#include <Gtypes.h>
#include <argParser.h>

//loops
#include <TAnalyzer.h>
#include <Unpacker.h>
#include <TCorrelator.h>
#include <TTreeOut.h>

#include <Histogramer.h>

#include <utils.h>
#include <globals.h>
//#include <GGlobals.h>





BetaInt *BetaInt::fBetaInt = 0;

//BetaInt::BetaInt(int argc, char **argv) : TRint("gint",&argc,argv,0,0,true,false) {
BetaInt::BetaInt(int argc, char **argv) : TRint("gint",0,0,0,0,true,false), 
  fRootFilesOpened(0), fTabLock(false), fMainThreadId(std::this_thread::get_id())  {

  LoadOptions(argc,argv);
  LoadStyle();
  SetPrompt("beta [%d] ");

}

BetaInt *BetaInt::Get(int argc,char **argv) {
  if(!fBetaInt)
    fBetaInt = new BetaInt(argc,argv);
  return fBetaInt;
}

BetaInt::~BetaInt() { }

void BetaInt::Terminate(int status) {
  printf("\nbye,bye\n\n");
  SetPrompt("");
  TRint::Terminate(status);

}

void BetaInt::LoadStyle() {
  // Load the ROOT style file
  //gStyle->SetPalette(kVisibleSpectrum);
  gStyle->SetPalette(gEnv->GetValue("BetaInt.Style",kVisibleSpectrum));
  gStyle->SetHistLineWidth(2);
  gStyle->SetHistFillStyle(0);
 
  gStyle->SetFrameBorderMode(1);
  //gStyle->SetFrameFillColor(1);
  gStyle->SetPadBorderMode(1);
  gStyle->SetPadBorderSize(5);
  gStyle->SetPadColor(2);

//canvas.SetBorderSize(6);
//canvas.SetHighLightColor(kBlue); //or whatever color


  //Hists.Stats: "nemri"
  gStyle->SetOptStat(gEnv->GetValue("Hists.Stats","nemri"));

  //gEnv->Print();


  gROOT->ForceStyle();
}


void BetaInt::LoadOptions(int argc, char **argv) {
  //check the grutrc file for set preset optrions....

  argParser parser;

  std::vector<std::string> input_files;
  bool doHelp,doVersion,noSort,doQuit,doLoopBackwards,noTree;

  parser.default_option(&input_files)
    .description("Input file(s)");
  parser.option("h help ?",&doHelp)
    .description("Show this help Message")
    .default_value(false);
  parser.option("v version",&doVersion)
    .description("Show version")
    .default_value(false);
  parser.option("t notree",&noTree)
    .description("no fdsi tree  is made")
    .default_value(false);
  parser.option("b back",&doLoopBackwards)
    .description("Loop from largest to smallest entry")
    .default_value(false);
  parser.option("n no-sort",&noSort)
    .description("Do not attemp to sort input 'trees'")
    .default_value(false);
  parser.option("q quit",&doQuit)
    .description("quit the program after sorting")
    .default_value(false);


  // Do the parsing...
  try{
    parser.parse(argc, argv);
  } catch (ParseError& e){
    std::cerr << "ERROR: " << e.what() << "\n"
              << parser << std::endl;
    //fShouldExit = true;
  }

 

  // Print help if requested.
  if(doHelp){
    //Version();
    std::cout << parser << std::endl;
    //fShouldExit = true;
  }
  if(doVersion) {
    //Version();
    printf("version not available.\n");
    //fShouldExit = true;
  }


  for(auto& file : input_files){
    switch(DetermineFileType(file)){
      case kFileType::CALIBRATION:
        break;
      case kFileType::ROOTFILE:
        {
          TFile *rfile = OpenRootFile(file);
          //if(rfile && doGui && gHistomatic) 
          //  gHistomatic->AddRootFile(rfile);
        }
        break;
      case kFileType::TOF: {
          TFDSi::SetTOFCorrector(file); 
        }
        break;
      case kFileType::MACRO:
        break;
      case kFileType::CUTS:
        break;
      default:
        printf("\tDiscarding unknown file: %s\n",file.c_str());
      break;
    };
  }

  //
  //  The Loop organization. Ideally, each loop would take a pointer to the previous one....
  //  - Each loop holds a queue of an object which inherits from a TObject....
  //
  if(fFilesToSort.size() && !noSort) {
    //we are going to sort, lets read some calibrations in.
    TClover::ReadCalFile("cals/CloverInit_1343.txt");

    printf("here 1\n");
    for(auto file : fFilesToSort) { //bad things will currently happen if ever more then one file, tanalyzer is a singleton.
    printf("here 2\n");
      TAnalyzer::AddFile(file);
    }

    TFile *file =fFilesToSort[0];
      int run,subrun;
      getRunNumber(file->GetName(),run,subrun);
      Histogramer::Get()->SetRun(run,subrun);


      if(doLoopBackwards)
        TAnalyzer::Get()->SetBackwards();
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
  
      if(noTree)
        TTreeOut::SetNoTree();
      TTreeOut::Get()->SetRun(run,subrun);
      std::thread CorrFDSi2Tree(&TTreeOut::TreeLoop,TTreeOut::Get());
      CorrFDSi2Tree.detach();
      
      progress.join();

      Histogramer::Get()->Close();

      if(doQuit)
        this->Terminate(0);
    
  }


}

kFileType BetaInt::DetermineFileType(const std::string& filename) const {
  size_t dot = filename.find_last_of('.');
  std::string ext = filename.substr(dot+1);

  if((ext=="gz") || (ext=="bz2") || (ext=="zip")) {
    std::string remaining = filename.substr(0,dot);
    ext = remaining.substr(remaining.find_last_of('.')+1);
  }
  
  if(ext == "cal") {
    return kFileType::CALIBRATION;
  } else if(ext == "tof") {
    return kFileType::TOF;
  } else if(ext == "root") {
    return kFileType::ROOTFILE;
  } else if((ext=="c") || (ext=="C") 
            || (ext=="c+") || (ext=="C+") 
            || (ext=="c++") || (ext=="C++")) {
    return kFileType::MACRO;
  } else if(ext == "cuts") {
    return kFileType::CUTS;
  } else {
    return kFileType::UNKNOWN;
  }
};

/*
bool BetaInt::FileAutoDetect(const std::string& filename) {
  switch(DetermineFileType(filename)){
    case kFileType::CALIBRATION:
      break;
    case kFileType::ROOTFILE:
      OpenRootFile(filename);
      break;
    case kFileType::MACRO:
      break;
    case kFileType::CUTS:
    default:
      printf("\tDiscarding unknown file: %s\n",filename.c_str());
      return false;    
    break;
  };
  return true;
}
*/

TFile *BetaInt::OpenRootFile(const std::string& filename, Option_t* opt) {
  TString sopt(opt);
  sopt.ToLower();
  TFile *file = NULL;

  if(sopt.Contains("recreate") || sopt.Contains("new")) {
    file = new TFile(filename.c_str(),"recreate");
    if(!file->IsOpen()) file = NULL;
    if(file) {
      const char* command = Form("TFile* _file%i = (TFile*)%luL",
                                 fRootFilesOpened,
                                 (unsigned long)file);
      TRint::ProcessLine(command);
      fRootFilesOpened++;
    } else {
      std::cout << "Could not create " << filename << std::endl;
    }
  } else {
    //file = TFile::Open(filename.c_str(),opt);
    file = new TFile(filename.c_str(),opt);
    if(!file->IsOpen()) file = NULL;
    if(file) {
      const char* command = Form("TFile* _file%i = (TFile*)%luL",
                                 fRootFilesOpened,
                                 (unsigned long)file);
      TRint::ProcessLine(command);
      std::cout << "\tfile " << BLUE << file->GetName() << RESET_COLOR
                <<  " opened as " << BLUE <<  "_file" << fRootFilesOpened << RESET_COLOR <<  std::endl;
      fRootFilesOpened++;
      TObject *obj = file->Get("tree"); 
      if(obj && obj->InheritsFrom("TTree")) {
        fFilesToSort.push_back(file);
      }
    } else {
      std::cout << "Could not create " << filename << std::endl;
    }
  }

  return file;
}

int BetaInt::TabCompletionHook(char* buf, int* pLoc, std::ostream& out) {
  fTabLock = true;
  int result = TRint::TabCompletionHook(buf,pLoc,out);
  fTabLock = false;
  return result;
}


long BetaInt::ProcessLine(const char* line, bool sync, int* error) {
  long retval = 0;
  if(fTabLock) {
    return TRint::ProcessLine(line,sync,error);
  }
  TString sline(line);
  if(!sline.Length()) {
    return 0;
  }

  if(std::this_thread::get_id() != fMainThreadId){
    printf("Not the main thread...  ");
    fflush(stdout);
  }           
  
  if(!sline.CompareTo("clear")) {
    retval = TRint::ProcessLine(".! clear");
  } else {
    retval = TRint::ProcessLine(sline.Data(),sync,error);
  }

  if(retval < 0) {
    //std::cerr << "Error processing line: " << line << std::endl;
  }
  return retval;
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

