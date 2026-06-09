#ifndef __UNPACKER_H__
#define __UNPACKER_H__

#include <queue>
#include <atomic>

#include <TFDSi.h>

class Unpacker {
  private: 
    Unpacker();
    static Unpacker *fUnpacker;
  public: 
    static Unpacker *Get();
    ~Unpacker();

  private:
    std::queue<TFDSi> fQueue;
    std::atomic<bool> fLoopRunning{false};
    
    long fIn;
    long fOut;
    bool fForwardToNext=true;

    size_t fMaxQueueSize = 100000;

  public:
    bool LoopRunning() const { return fLoopRunning; }
    std::queue<TFDSi> GetQ() { return fQueue; }
    size_t qsize(); // const { return fQueue.size(); }
    bool SetForwardToNext(bool forward) { fForwardToNext = forward; return fForwardToNext; }

    void push(TFDSi &&fdsi);
    //TFDSi pop(); 
    bool pop(TFDSi &fdsi);

    void Unpack();  // unpacking loop
    
    void FillHistograms(const TFDSi &fdsi);

    std::string Status();

};

#endif
