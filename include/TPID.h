
#ifndef TPID_H
#define TPID_H

#include "TObject.h"

/*
 * PID class defintion
 */
class TPID: public TObject {
  public:
  
  TPID() { Reset(); } 
  TPID(const TPID &other) { other.Copy(*this); } 
  ~TPID() { } 

  // dynode energy
  double dyecal;

  // PIN dE
  double de1;
  double de2;
  // TOF from TACs
  double tof;

  // Implant tracking for correlation
  // Extendable if more information is desired
  double time;
  double xpos;
  double ypos;

  bool implanted;   // ?
  double dtLastIon; // ?

  bool hasGoodPosition;

  // For use in vectors we need a copy constructor
  TPID& operator=(const TPID& rhs)
    {
      dyecal = rhs.dyecal;
      de1 = rhs.de1;
      de2 = rhs.de2;
      tof = rhs.tof;
      time = rhs.time;
      xpos = rhs.xpos;
      ypos = rhs.ypos;
      implanted = rhs.implanted;
      dtLastIon = rhs.dtLastIon;
      hasGoodPosition = rhs.hasGoodPosition;
      return *this;
    }

  bool goodPosition() const { 
    return xpos>0 && xpos < 48.0 &&
           ypos>0 && ypos < 48.0;
  }

 // bool operator<(const TPID& rhs) const { return time<rhs.time; } //smallest to largest
  bool operator<(const TPID& rhs) const { return time>rhs.time; } //largest to smallest

  void Reset();
  void Print();

  void Copy(TPID &) const;

  ClassDef(TPID,2);
};

#endif
