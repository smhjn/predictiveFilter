#ifndef PREDICTIVEFILTER_H     // Prevent duplicate definition
#define PREDICTIVEFILTER_H 

// Include
// #include "./cdhlib/Timer.h"
#include "PredictiveFilterDefines.h"
#include "LowPassFilter.h"
#include "PolyFit.h"

using namespace std;

#define PREDICTIVE_FILTER_TIMING 0

class PredictiveFilter
{
	public:
	PredictiveFilter();
	~PredictiveFilter();
	void reset();
	int configure(unsigned int numPointsToFilter=MAXNUMDATAPOINTS, unsigned flags=FFTW_EXHAUSTIVE);
	int addData(double* timestamps, double* datapoints, unsigned int length=1);
	int filter(double cutoffFrequency, int order, unsigned int numPointsToFit=0);
	double getPrediction(double timestamp=0);

	// Get state functions:
	double* getData(){ return &data[numDataPointsToFilter-numDataPoints]; }
	double* getFilteredData(){ return filteredData; }
	double* getDataToFit(){ return dataToFit; }
	double* getTimeToFit(){ return timeToFit; }
	double* getTime(){ return &time[numDataPointsToFilter-numDataPoints]; }
	long double* getCoeffs(){ return Coeffs; }
	unsigned int getNumDataPoints(){ return numDataPoints; }
	unsigned int getNumDataPointsToFit(){ return numDataPointsToFit; }
	unsigned int getNumDataPointsToFilter(){ return numDataPointsToFilter; }
	double getStartTime(){ return startTime; }

	private:
	int calculateNumberOfPolyPoints(int polyOrder, double SIratio);

	// Limits and counters:
	unsigned int numDataPoints;
	unsigned int numDataPointsToFilter;
	unsigned int numDataPointsToFit;

	// Storage buffers:
	double data[MAXNUMDATAPOINTS];
	double filteredData[MAXNUMDATAPOINTS];
	double dataToFit[MAXNUMDATAPOINTS];
	double time[MAXNUMDATAPOINTS];
	double timeToFit[MAXNUMDATAPOINTS];

	// Low pass filter class:
	LowPassFilter lpf;

	// Protected State:
	// pthread_mutex_t lock;
    double startTime;
    long double Coeffs[MAXORDER+1];
    int Order;

  	#if PREDICTIVE_FILTER_TIMING
	StopWatch configureSW, resetSW, addSW, filterSW, numPtsSW, fitSW, mutexSW, fullpredictSW, prefitSW;
	void printTiming();
	#endif
};

#endif
