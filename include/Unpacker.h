#ifndef __UNPACKER_H__
#define __UNPACKER_H__

#include <queue>

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
    bool fLoopRunning;

  public:
    bool LoopRunning() const { return fLoopRunning; }
    std::queue<TFDSi> GetQ() { return fQueue; }
    size_t qsize() const { return fQueue.size(); }

    void push(TFDSi &fdsi);
    TFDSi pop(); 

    void Unpack();  // unpacking loop

    std::string Status() const;

};

#endif
