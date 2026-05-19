
#include<cstdio>

#include<BetaInt.h>
#include<utils.h>

#include<TEnv.h>
//#include<TStyle.h>
//#include<TROOT.h>

void loadEnv() {
  setenv("BSYS", (programPath()+"/..").c_str(), 0);

  // Load $BSYS/.betarc
  std::string beta_path = Form("%s/.betarc",getenv("BSYS"));
  gEnv->ReadFile(beta_path.c_str(),kEnvChange);

  // Load $HOME/.betarc
  beta_path = Form("%s/.betarc",getenv("HOME"));
  if(fileExists(beta_path.c_str())){
    gEnv->ReadFile(beta_path.c_str(),kEnvChange);
  }
  
  beta_path = Form("%s/.betarc",getenv("PWD"));
  if(fileExists(beta_path.c_str())){
    gEnv->ReadFile(beta_path.c_str(),kEnvChange);
  }
}


int main(int argc, char **argv) {

  //loadStyle();
  loadEnv();
  BetaInt::Get(argc,argv)->Run(true);
 
  return 0;
}
