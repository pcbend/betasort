#ifndef __DDASHIT_H__
#define __DDASHIT_H__

#include<vector>
#include<climits>

#include<TObject.h>

class ddasHit : public TObject {
  public:
    ddasHit();
    ~ddasHit();

   void setEvId(const unsigned long long EVID) { evId   = EVID; }  // evID
   void setId(const int ID)                    { id    = ID; }
   void setEnergy(const double E)              { energy = E; }
   void setTime(const double T)                { time   = T; }
   void setCFD(const int CFD)                  { cfd = CFD; }
   void setQDC(const std::vector<int> QDC)     { qdc = QDC; }
   void setTraceLength(const int TL)           { traceLength = TL; }
   void setTrace(const std::vector<unsigned short> TRACE)  { trace = TRACE; }

   void Clear();
   void Copy(ddasHit& lhs) const;

   ddasHit operator=(ddasHit const& rhs);
   bool    operator==(ddasHit const& rhs);
   bool    operator<(ddasHit const& rhs) const;

  public:
    double GetEnergy()  const { return energy; }
    double GetTime()    const { return time + cfd/16384.; }
    int    GetCFDTime() const { return cfd; }
    int    GetId()      const { return id;  }

    std::vector<unsigned short> GetTrace() const { return trace; }
    std::vector<int> GetQDCSums() const { return qdc; }

    void print() const;

  private:
    unsigned long long evId;
    int id;
    double energy;
    double time;
    int cfd;
    //int qdc[8];
    std::vector<int> qdc;

    int traceLength;
    std::vector<unsigned short> trace;

  ClassDef(ddasHit,1);
};


#endif
