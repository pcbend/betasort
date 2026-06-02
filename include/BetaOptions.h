#ifndef __BETAOPTIONS_H__
#define __BETAOPTIONS_H__

#include <string>
#include <vector>

enum class OutputLevel {
  Analyzer,
  Unpacker, 
  Correlator,
  Tree
};

inline std::istream& operator>>(std::istream& in, OutputLevel &level) {
  std::string value;
  in >> value;
  if(value == "analyzer")
    level = OutputLevel::Analyzer;
  else if(value == "unpacker")
    level = OutputLevel::Unpacker;
  else if(value == "correlator")
    level = OutputLevel::Correlator;
  else if(value == "tree" || value == "full")
    level = OutputLevel::Tree;
  else
    level = OutputLevel::Tree;
  return in;
}



class BetaOptions {
  public:
    bool doHelp    = false;
    bool doVersion = false;
    bool noSort    = false;
    bool noTree    = false;
    bool doQuit    = false;

    OutputLevel outputLevel = OutputLevel::Tree;

    std::vector<std::string> inputFiles;

    std::vector<std::string> rootFiles;
    std::vector<std::string> calFiles;
    std::vector<std::string> tofFiles;
    std::vector<std::string> macroFiles;
    std::vector<std::string> cutFiles;



};


#endif

