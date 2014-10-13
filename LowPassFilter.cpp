#include "LowPassFilter.h"

#if TIME_LPF
#include <time.h>
unsigned long rt_time()
{
	timespec timeSpec;
	clock_gettime(CLOCK_REALTIME, &timeSpec);
	return (timeSpec.tv_sec*1000000000 + timeSpec.tv_nsec);
}
#endif

LowPassFilter::LowPassFilter()
{
	fft_handle = NULL;
	ifft_handle = NULL;
	filterStartBin = 0;
}

LowPassFilter::~LowPassFilter()
{
	destroy();
}

void LowPassFilter::destroy()
{
	if( fft_handle != NULL )
	{
		fftw_destroy_plan(fft_handle);
		fft_handle = NULL;
	}

	if( ifft_handle != NULL )
	{
		fftw_destroy_plan(ifft_handle);
		ifft_handle = NULL;
	}

	// WE CAN'T CLEAN UP BECAUSE IT WILL DESTROY FFTW WHEN ANOTHER LPF 
	// OBJECT MAY STILL BE USING FFTW!
	// fftw_cleanup();
}

int LowPassFilter::configure(unsigned int numPointsToUse, unsigned flags)
{
	#if TIME_LPF
	prestart = rt_time();
	#endif

	// Check inputs:
	if( numPointsToUse > MAX_NUM_REAL_POINTS )
		return -1;

	if( numPointsToUse%2 != 0 || numPointsToUse < 8 )
		return -1;

	// Clean-up any previous instance first:
	destroy();

	// Set sizes:
	numRealPoints = numPointsToUse;
	// numFFTPoints = numRealPoints/2 + 1;
	//paddedRealPoints = numRealPoints*2;
	paddedRealPoints = numRealPoints*2;
	paddedFFTPoints = paddedRealPoints/2+1;

	// Set filter width to be 10% of spread of the FFT:
	filterWidth = (unsigned int) (((double)paddedFFTPoints)*0.10+0.5);

	// Set the end point averaging width to be 5% of end points:
	fivePercentWidth = (unsigned int) (((double)numRealPoints)*0.05 + 0.5);
	if( fivePercentWidth < 4 )
		fivePercentWidth = 4;

	//
	// Planning-rigor flags
	//
	// FFTW_ESTIMATE specifies that, instead of actual measurements of different algorithms, a simple heuristic is used to pick a (probably sub-optimal) plan quickly. With this flag, the input/output arrays are not overwritten during planning.
	// FFTW_MEASURE tells FFTW to find an optimized plan by actually computing several FFTs and measuring their execution time. Depending on your machine, this can take some time (often a few seconds). FFTW_MEASURE is the default planning option.
	// FFTW_PATIENT is like FFTW_MEASURE, but considers a wider range of algorithms and often produces a “more optimal” plan (especially for large transforms), but at the expense of several times longer planning time (especially for large transforms).
	// FFTW_EXHAUSTIVE is like FFTW_PATIENT, but considers an even wider range of algorithms, including many that we think are unlikely to be fast, to produce the most optimal plan but with a substantially increased planning time.
	// FFTW_WISDOM_ONLY is a special planning mode in which the plan is only created if wisdom is available for the given problem, and otherwise a NULL plan is returned. This can be combined with other flags, e.g. ‘FFTW_WISDOM_ONLY | FFTW_PATIENT’ creates a plan only if wisdom is available that was created in FFTW_PATIENT or FFTW_EXHAUSTIVE mode. The FFTW_WISDOM_ONLY flag is intended for users who need to detect whether wisdom is available; for example, if wisdom is not available one may wish to allocate new arrays for planning so that user data is not overwritten.
	//
	// printf("FFTW_ESTIMATE: %d\n" , FFTW_ESTIMATE);   //64
	// printf("FFTW_MEASURE: %d\n", FFTW_MEASURE);		//0
	// printf("FFTW_PATIENT: %d\n", FFTW_PATIENT);		//32
	// printf("FFTW_EXHAUSTIVE: %d\n",FFTW_EXHAUSTIVE); //8
	// printf("FFTW_WISDOM_ONLY: %d\n",FFTW_WISDOM_ONLY); //2097152
	// Configure fft handles:
	fft_handle = fftw_plan_dft_r2c_1d(paddedRealPoints, input_fft_signal, power, flags);
	ifft_handle = fftw_plan_dft_c2r_1d(paddedRealPoints, filtered_power, output_fft_signal, flags);

	#if TIME_LPF
	prestop = rt_time();
	#endif

	return 0;
}

double LowPassFilter::filter(double cutoffFrequency, double* timeStamps, double* inputSignal, double* outputSignal)
{
	unsigned int k;
	int ind;

	#if TIME_LPF
	start = rt_time();
	#endif

	// Clear everything:
	bzero(input_fft_signal, paddedRealPoints*sizeof(double));
	bzero(output_fft_signal, paddedRealPoints*sizeof(double));

	// Copy input signal into working array:
	memcpy(&input_signal, inputSignal, numRealPoints*sizeof(double)); // For printing only
	memcpy(&timestamps, timeStamps, numRealPoints*sizeof(double)); // For printing only
	memcpy(&input_fft_signal, inputSignal, numRealPoints*sizeof(double));

	// Mirror half of the signal on each side:
	// int ind1 = paddedRealPoints/4;
	// int ind2 = paddedRealPoints/4 - 1;
	// int ind3 = paddedRealPoints/2 + paddedRealPoints/4 - 1;
	// int ind4 = paddedRealPoints/2 + paddedRealPoints/4;
	// double firstValDoubled = input_fft_signal[ind1]*2;
	// double lastValDoubled = input_fft_signal[ind3]*2;
	// for(k = 0; k < paddedRealPoints/4; k++)
	// {
	// 	input_fft_signal[ind2] = firstValDoubled-input_fft_signal[ind1];
	// 	input_fft_signal[ind4] = lastValDoubled-input_fft_signal[ind3];
	// 	ind1++;
	// 	ind2--;
	// 	ind3--;
	// 	ind4++;
	// }

	// Mirror input signal to help with FFT:
	ind = numRealPoints-1;
	double lastValueDoubled = input_fft_signal[numRealPoints-1];
	lastValueDoubled += lastValueDoubled;
	k = numRealPoints;
	while(ind >= 0)
	{
		// Repeat the signal in reverse:
		//input_fft_signal[k] = input_fft_signal[ind];
		// Repeat and mirror the signal in reverse around the last point:
		input_fft_signal[k] = lastValueDoubled - input_fft_signal[ind];
		ind--;
		k++;
	}


	// Average the first a last 5% of data to get robust endpoints:
	double startPoint = 0.0;
	double endPoint = 0.0;
	ind = paddedRealPoints-1;
	for(k = 0; k < fivePercentWidth; k++)
	{
		startPoint += input_fft_signal[k];
		endPoint += input_fft_signal[ind];
		ind--;
	}
	startPoint /= fivePercentWidth;
	endPoint /= fivePercentWidth;

	// Fit line to input signal endpoints:
	double slope = (endPoint - startPoint)/(paddedRealPoints);

	// Apply linear correction to data to make endpoints equal:
	for(k = 0; k < paddedRealPoints; k++)
	{
		input_fft_signal[k] -= slope*k;
	}

	// // Add in mean pad:
	// ind = numRealPoints*3-1;
	// for(k = 0; k < numRealPoints; k++)
	// {
	// 	input_fft_signal[k] = input_fft_signal[numRealPoints-1];
	// 	input_fft_signal[ind] = input_fft_signal[numRealPoints-1];
	// 	ind--;
	// }

	// ind = numRealPoints*2-1;
	// while(ind >= 0)
	// {
	// 	// Repeat the signal in reverse:
	// 	input_fft_signal[k] = input_fft_signal[ind];
	// 	//input_fft_signal[k] = lastValueDoubled - input_fft_signal[ind];
	// 	ind--;
	// 	k++;
	// }

	#if TIME_LPF
	copystop = rt_time();
	#endif

	#if DEBUG_LPF
	unsigned int i = 0;
	printf("--------------\n");
	for(i = 0; i < paddedRealPoints; i++ )
		printf("input[%d]: %f\n", i, input_fft_signal[i]);
	#endif

	// Compute the FFT (void):
	fftw_execute(fft_handle);
    
    #if TIME_LPF
	fftstop = rt_time();
	#endif

    #if DEBUG_LPF
    printf("--------------\n");
    for(i = 0; i < paddedFFTPoints; i++ )
		printf("power[%d]: %f\n", i, sqrt(power[i][REAL] * power[i][REAL] + power[i][IMAG] * power[i][IMAG]));
	#endif

	// Calculate sample frequency:
	double avedt = (timeStamps[numRealPoints-1] - timeStamps[0]) / numRealPoints;
	sampleFrequency = 1.0/avedt;

	// Create the hann filter:
	createHannFilter(cutoffFrequency, sampleFrequency);

    // Filter the result:
    applyHannFilter();

    #if TIME_LPF
	filterstop = rt_time();
	#endif

    #if DEBUG_LPF
    printf("--------------\n");
    for(i = 0; i < paddedFFTPoints; i++ )
		printf("filtered[%d]: %f\n", i, sqrt(filtered_power[i][REAL] * filtered_power[i][REAL] + filtered_power[i][IMAG] * filtered_power[i][IMAG]));
	#endif

	// Copy input power spectrum for printing:
 	memcpy(&filtered_power_copy, &filtered_power, paddedFFTPoints*sizeof(fftw_complex)); // For printing only

    // Compute the inverse FFT (void):
    fftw_execute(ifft_handle);

    #if TIME_LPF
	ifftstop = rt_time();
	#endif

    #if DEBUG_LPF
    printf("--------------\n");
    for(i = 0; i < paddedRealPoints; i++ )
		printf("output[%d]: %f\n", i, output_fft_signal[i]);
	#endif

	// Adjust the output signal's power:
	for(k = 0; k < paddedRealPoints; k++)
	{
		output_fft_signal[k] /= paddedRealPoints;
	}

 	// Copy working array into output signal:
	memcpy(outputSignal, &output_fft_signal, numRealPoints*sizeof(double));

	// Apply linear correction to output signal:
	for(k = 0; k < numRealPoints; k++)
	{
		outputSignal[k] += slope*(k);
	}

	// Copy output array for printing:
	memcpy(&output_signal, outputSignal, numRealPoints*sizeof(double)); // For printing only


	#if TIME_LPF
	stop = rt_time();
	#endif

	#if PLOT_PRINT_LPF
	printResults();
	#endif

	#if TIME_LPF
	printf("#Pre time: %f ms\n",(prestop-prestart)/1000.0/1000.0);
	printf("#Total time: %f ms\n",(stop-start)/1000.0/1000.0);
	printf("#\t copy input: %f ms\n",(copystop-start)/1000.0/1000.0);
	printf("#\t        fft: %f ms\n",(fftstop-copystop)/1000.0/1000.0);
	printf("#\t     filter: %f ms\n",(filterstop-fftstop)/1000.0/1000.0);
	printf("#\t       ifft: %f ms\n",(ifftstop-filterstop)/1000.0/1000.0);
	printf("#\tcopy output: %f ms\n",(stop-ifftstop)/1000.0/1000.0);
	#endif

	return (sampleFrequency/cutoffFrequency);
}

void LowPassFilter::createHannFilter(double cutoffFreq, double sampleFreq)
{
    // At start freq, decrease from 1 to 0 over filtWidth points
    unsigned int targetBin = (unsigned int)((( cutoffFreq * ((double)paddedRealPoints) ) / sampleFreq) + 1); // Round up always

    // Make sure targetBin is within range:
    if( targetBin >= paddedFFTPoints )
    	targetBin = paddedFFTPoints-1;

    // Recalculate the filter if the frequency of the control law changes:
    if( targetBin != filterStartBin )
    {
    	filterStartBin = targetBin;

    	unsigned int i = 0;
	    for(; i<filterStartBin; i++)
	    {
	        filter_array[i] = 1;
	    }

	    int x = 0;
	    unsigned int iterateTill = filterStartBin+filterWidth;
	    if( iterateTill > paddedFFTPoints )
	    	iterateTill = paddedFFTPoints;
	    for(; i<iterateTill; i++)
	    {
	        double evalPt = M_PI * ((double)x)/((double)filterWidth);     
	        filter_array[i] = 0.5 + 0.5*cos(evalPt);
	        x++;        
	    }

	    for(; i < paddedFFTPoints; i++)
	    {
	        filter_array[i] = 0;
	    }
    }

    #if DEBUG_LPF
    printf("--------------\n");
    printf("length of fft: %d\n", paddedFFTPoints);
    printf("sample freq: %f cutoffFreq: %f\n", sampleFreq, cutoffFreq);
    printf("filterStartBin: %d (%f Hz)\n", filterStartBin, targetBin*sampleFreq/paddedRealPoints);
    printf("filterWidth: %d\n", filterWidth);
    for(unsigned int i = 0; i < paddedFFTPoints; i++ )
		printf("filter[%d]: %f\n", i, filter_array[i]);
	#endif
}

void LowPassFilter::applyHannFilter()
{
    for(unsigned int n = 0; n < paddedFFTPoints; n++)
    {
        filtered_power[n][REAL] = power[n][REAL]*filter_array[n];
        filtered_power[n][IMAG] = power[n][IMAG]*filter_array[n];
    }
}

void LowPassFilter::printResults()
{
	// Print results for plotting:
	unsigned int j = 0;
	for(; j < numRealPoints; j++ )
		printf("%d, %lu, %05.6f, %05.6f, %05.6f, %05.6f, %05.6f, %05.6f, %05.6f\n", j, timestamps[j], input_signal[j], output_signal[j], input_fft_signal[j], output_fft_signal[j], 
			j*sampleFrequency/paddedRealPoints, 
			sqrt(power[j][REAL] * power[j][REAL] + power[j][IMAG] * power[j][IMAG]),
			sqrt(filtered_power_copy[j][REAL] * filtered_power_copy[j][REAL] + filtered_power_copy[j][IMAG] * filtered_power_copy[j][IMAG]));
	for(; j < paddedFFTPoints; j++ )
		printf("%d, %lu, %05.6f, %05.6f, %05.6f, %05.6f, %05.6f, %05.6f, %05.6f\n", j, 0l, 0.0, 0.0, input_fft_signal[j], output_fft_signal[j], 
			j*sampleFrequency/paddedRealPoints, 
			sqrt(power[j][REAL] * power[j][REAL] + power[j][IMAG] * power[j][IMAG]),
			sqrt(filtered_power_copy[j][REAL] * filtered_power_copy[j][REAL] + filtered_power_copy[j][IMAG] * filtered_power_copy[j][IMAG]));
	for(; j < paddedRealPoints; j++ )
		printf("%d, %lu, %05.6f, %05.6f, %05.6f, %05.6f, %05.6f, %05.6f, %05.6f\n", j, 0l, 0.0, 0.0, input_fft_signal[j], output_fft_signal[j], 
			0.0, 0.0, 0.0);
}