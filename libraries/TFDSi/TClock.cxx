/*
 * TClock.cpp. Class methods for tracking event time.
 * Calibration methods and parameters for DDAS clock.
 * 
 * Author: A. Chester
 *
 */

#include "TClock.h"

TClock::TClock() { Reset(); } 

TClock::~TClock() { } 

/*
 * Reset clock variables
 */
void TClock::Reset()
{
  current = 0.;
  initial = -1;
}

void TClock::Copy(TClock &other) const {
  other.current = current;
  other.initial = initial;
}

