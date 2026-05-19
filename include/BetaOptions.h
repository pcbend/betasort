#ifndef __BETAOPTIONS_H__
#define __BETAOPTIONS_H__

#include <string>
#include <vector>

class BetaOptions {
  public:
    bool doHelp    = false;
    bool doVersion = false;
    bool noSort    = false;
    bool noTree    = false;
    bool doQuit    = false;

    std::vector<std::string> inputFiles;

    std::vector<std::string> rootFiles;
    std::vector<std::string> calFiles;
    std::vector<std::string> tofFiles;
    std::vector<std::string> macroFiles;
    std::vector<std::string> cutFiles;



};


#endif

