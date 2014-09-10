#ifndef LOWPASSFILTER_H     // Prevent duplicate definition
#define LOWPASSFILTER_H 

// Include
#include "PredictiveFilterDefines.h"
#include <fftw3.h>
#include <stdio.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define MAX_NUM_REAL_POINTS (MAXNUMDATAPOINTS)
#define MAX_NUM_FFT_POINTS (MAX_NUM_REAL_POINTS/2 + 1)
#define MAX_NUM_REAL_POINTS_PADDED (MAX_NUM_REAL_POINTS*2)
#define MAX_NUM_FFT_POINTS_PADDED (MAX_NUM_FFT_POINTS*2)

#define REAL 0
#define IMAG 1

#define DEBUG_LPF 0
#define TIME_LPF 0
#define PLOT_PRINT_LPF 0

class LowPassFilter
{
	public:
	LowPassFilter();
	~LowPassFilter();
	void destroy();
	int configure(unsigned int numPointsToUse, unsigned flags=FFTW_EXHAUSTIVE);
	double filter(double cutoffFreq, unsigned long* timeStamps, double* inputSignal, double* outputSignal); // returns S/I ratio
	void printResults();

	private:
	void createHannFilter(double cutoffFreq, double sampleFreq);
	void applyHannFilter();

	unsigned int numRealPoints;
	unsigned int numFFTPoints;
	unsigned int paddedRealPoints;
	unsigned int paddedFFTPoints;
	unsigned int filterWidth;
	unsigned int fivePercentWidth;
	unsigned int filterStartBin;
	double sampleFrequency;
	double hannResult[MAX_NUM_FFT_POINTS_PADDED];
	double input_signal[MAX_NUM_REAL_POINTS_PADDED]; // for printing only
	double output_signal[MAX_NUM_REAL_POINTS_PADDED]; // for printing only
	double input_fft_signal[MAX_NUM_REAL_POINTS_PADDED];
	double output_fft_signal[MAX_NUM_REAL_POINTS_PADDED];
	double filter_array[MAX_NUM_FFT_POINTS_PADDED];
	unsigned long timestamps[MAX_NUM_REAL_POINTS_PADDED];
	fftw_complex power[MAX_NUM_FFT_POINTS_PADDED];
	fftw_complex filtered_power_copy[MAX_NUM_FFT_POINTS_PADDED];
	fftw_complex filtered_power[MAX_NUM_FFT_POINTS_PADDED];
	fftw_plan fft_handle;
	fftw_plan ifft_handle;

	#if TIME_LPF
	unsigned long prestart;
	unsigned long prestop;
	unsigned long start;
	unsigned long copystop;
	unsigned long fftstop;
	unsigned long filterstop;
	unsigned long ifftstop;
	unsigned long stop;
	#endif
};

#endif
