#ifndef __GINT_H__
#define __GINT_H__

#ifndef __CINT__
#include <thread>
#endif

#include <vector>

#include <TRint.h>

#include <Gtypes.h>

class TFile;

class BetaInt : public TRint {
  private:
    BetaInt(int argc, char **argv);
    static BetaInt *fBetaInt; 
  public:
    static BetaInt *Get(int argc=0,char **argv=0);
    virtual ~BetaInt();
  
    int  TabCompletionHook(char* buf, int* pLoc, std::ostream& out) override;
    long ProcessLine(const char *line, bool sync=true,int *error=0) override;

    void Terminate(int status) override;

  public:
    void      LoadOptions(int argc, char **argv);
    kFileType DetermineFileType(const std::string& filename) const;
    //bool      FileAutoDetect(const std::string& filename);
    TFile*    OpenRootFile(const std::string& filename, Option_t *opt="");

    void      LoadStyle();    

  private:
    int fRootFilesOpened;
    bool fTabLock;

    std::vector<TFile*> fFilesToSort;

#ifndef __CINT__
    std::thread::id fMainThreadId;
#endif

  ClassDefOverride(BetaInt,0)
};

class LoopProgress {
  public: 
    LoopProgress();
    ~LoopProgress();
  
    void Show();

};


#endif

