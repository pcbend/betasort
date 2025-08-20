/*
 * TClock.h. Class definition for tracking event time.
 * Calibration methods and parameters for DDAS clock.
 * 
 * Author: A. Chester
 *
 */

#ifndef __TCLOCK_H__
#define __TCLOCK_H__

#include "TObject.h"

/*
 * DDAS clock class definition
 */
class TClock: public TObject
{
public:
  TClock();
  ~TClock();


  double  current;
  double  initial;

  void Reset();
  void Copy(TClock &other) const;

  ClassDef(TClock,2);
};

#endif
