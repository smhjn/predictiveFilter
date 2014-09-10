
#ifndef POLYFIT_H     // Prevent duplicate definition
#define POLYFIT_H 

// Include
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>      /* need functions 'toupper' and tolower'   */
#include <float.h>      /* defines LDBL_MAX                        */
#include <math.h>
#include <time.h>       /* for date marking of output data files   */
#include <iostream>
#include "PredictiveFilterDefines.h"

#define MAXORDER 8      /* Can do higher, but who cares? -> NOT ME! */
#define MAXNUMFITPOINTS 50 // Optimize!
#define POLYFIT_TIMING 0
#if POLYFIT_TIMING
   	#include "./cdhlib/Timer.h"
#endif

using namespace std;

// Modified polyfit function by Ken Hough
// Found here: http://www.slscope.co.uk/software/polyfit/polyfit.html

// DATA MUST BE IN SORTED ORDER:
int polyfit(int order, double x[], double y[], int length, long double (&polycoefs)[MAXORDER+1]);
double polyval(int order, long double x, long double Coeffs[9]);
inline long double ldpow(long double n, unsigned p);

#endif