
#ifndef TPID_H
#define TPID_H

#include "TObject.h"

class TFDSi;

/*
 * PID class defintion
 */
class TPID {
  public:
    TPID() { Reset(); } 
    TPID(const TPID &other) { other.Copy(*this); } 
    ~TPID() { } 
    
    void Copy(TPID &) const;
    void Print(Option_t *opt="") const;
    void Reset();

    double mtime() const { return timestamp/1.e6; }

  //private:
    double timestamp;
    double dyecal; // dynode energy

    double de1;
    double de2;
    double tof;
  
    double xpos;
    double ypos;

    double dtLastIon;  //must be set when added to the vector map. 
  
    // For use in vectors we need a copy constructor
    TPID& operator=(const TPID& rhs) {
      if(this != &rhs)
        rhs.Copy(*this);
      return *this;
    }

    bool goodPosition() const { 
      return xpos>0 && xpos < 48.0 &&
             ypos>0 && ypos < 48.0;
    }

   // bool operator<(const TPID& rhs) const { return time<rhs.time; } //smallest to largest
    bool operator<(const TPID& rhs) const { return timestamp>rhs.timestamp; } //largest to smallest

  ClassDef(TPID,3);
};

class TImplant : public TPID { 
  public:
    TImplant(const TPID& imp,const TFDSi& decay);
    TImplant() { } 
    virtual ~TImplant() { }

  //private:

    // Implant tracking for correlation
    double dtime;
    double dx;
    double dy;
    double dr2;
    double fom;

};


#endif
