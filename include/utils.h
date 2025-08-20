#ifndef __UTILS_H__
#define __UTILS_H__

#ifndef __CINT__
#ifndef __ROOTMACRO__

#include <cstdlib> 
#include <sys/stat.h> 
#include <iostream> 
#include <fstream> 
#include <sstream>
#include <limits.h>
#include <string>
#include <vector>

bool fileExists(const char *filename){
  //std::ifstream(filename);
  struct stat buffer;
  return (stat(filename,&buffer)==0);
}

std::vector<std::string> tokenizeString(std::string path,char delimiter='/') { 
  std::istringstream ss(path);
  std::string token;
  std::vector<std::string> parts;
  //printf("fullpath = %s\n",path.c_str());
  while(std::getline(ss,token,delimiter)) {
    //printf("token = %s\n",token.c_str());
    parts.push_back(token);
  }
  return parts;
}

void getRunNumber(std::string infile,int &run,int &subrun) {
  printf("I AM HERE\n");
  std::size_t one = infile.rfind(".root");
  std::size_t two = infile.rfind("-",one);
  std::size_t three = infile.rfind("-",two-1);
  
  std::string srun,ssubrun;
  if(three==-1) {
    //name is likely xxx####-##.root
    srun    = infile.substr(two-4,4);
    ssubrun = infile.substr(two+1,one-two-1);
  } else {
    //name is likely xxx-####-##.root
    srun    = infile.substr(three+1,two-three-1);
    ssubrun = infile.substr(two+1,one-two-1);
  }

  run = atoi(srun.c_str());
  subrun = atoi(ssubrun.c_str());
};

#ifdef __LINUX__
#include <unistd.h>
std::string programPath(){
  char buff[PATH_MAX+1];
  size_t len = readlink("/proc/self/exe", buff, sizeof(buff)-1);
  buff[len] = '\0';

  std::string exe_path = buff;
  return exe_path.substr(0, exe_path.find_last_of('/'));
}
#endif

#ifdef __DARWIN__ 
#include <mach-o/dyld.h>
std::string programPath(){
  char buff[PATH_MAX];
  uint32_t len = PATH_MAX;
  _NSGetExecutablePath(buff,&len);
  std::string exe_path = buff;
  return exe_path.substr(0, exe_path.find_last_of('/'));
}
#endif

#endif
#endif

#endif
