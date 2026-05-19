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

#include <Pipline.h>
#include <BetaOptions.h>

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

void BetaInt::ParseOptions(int argc, char **argv) {

  //TODO check the betarc file for preset options....
  
  argParser parser;
  BetaOptions options;

  bool callExit = false;

  parser.default_option(&options.inputFiles)
    .description("Input file(s)");
  parser.option("h help ?",&options.doHelp)
    .description("Show this help Message")
    .default_value(false);
  parser.option("v version",&options.doVersion)
    .description("Show version")
    .default_value(false);
  parser.option("t notree",&options.noTree)
    .description("no fdsi tree  is made")
    .default_value(false);
  parser.option("n no-sort",&options.noSort)
    .description("Do not attemp to sort input 'trees'")
    .default_value(false);
  parser.option("q quit",&options.doQuit)
    .description("quit the program after sorting")
    .default_value(false);


  try{
    parser.parse(argc, argv);
  } catch (ParseError& e){
    std::cerr << "ERROR: " << e.what() << "\n"
              << parser << std::endl;
    callExit = true;
  }
  
  if(doHelp){
    //Version();
    std::cout << parser << std::endl;
    callExit = true;
  }
  if(doVersion) {
    //Version();
    printf("version not available.\n");
    callExit = true;
  }

  if(callExit) exit(0);

  for(auto& file : input_files){
    switch(DetermineFileType(file)){
      case kFileType::CALIBRATIONi:
        calFiles.push_back(file);
        break;
      case kFileType::ROOTFILE:
        rootFiles.push_back(file);
        break;
      case kFileType::TOF: 
        tofFiles.push_back(file);
        break;
      case kFileType::MACRO:
        macroFiles.push_back(file);
        break;
      case kFileType::CUTS:
        cutFiles.push_back(file);
        break;
      default:
        printf("\tDiscarding unknown file: %s\n",file.c_str());
      break;
    };
  }

  return options;
}


void BetaInt::LoadOptions(int argc, char **argv) {

  BetaOptions options = PasrseOptions(argc,argv);
  Pipeline   pipeline(options);

  for(auto& file : options.rootFiles) {
    pipeline.AddFile(OpenRootFile(file));
  }

  if(!options.noSort) {
    pipeline.Run(0;
  }

  if(options.doQuit) {
    Terminate(0);
  }
};

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

