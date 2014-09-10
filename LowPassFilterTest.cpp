#include "LowPassFilter.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <stdlib.h>     /* atof */

using namespace std;

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

#define DT 0.0025
void genSignal(unsigned long* timeStamp, double* signal, int numPoints) {
    /* Generate two sine waves of different frequencies and
     * amplitudes.
     */  
    int i;
    unsigned long t = 0;
    double T = 0.0;
    for (i = 0; i < numPoints; ++i) {
        timeStamp[i] = t;
        signal[i] = 1300 + -1*(10.0*cos(10*2*M_PI*T) + 0.25*sin(151*2*M_PI*T) + 0.25*cos(204*2*M_PI*T) + 0.25*sin(248*2*M_PI*T));
        t += (unsigned long)(DT*1000000000 + 0.5);
        T += DT;
    }
}

int main(int argc, char* argv[])
{
	unsigned int numPoints = 128;
	double cutoffFrequency = 10.0;
	unsigned long timeStamp[MAX_NUM_REAL_POINTS];
	double inputSignal[MAX_NUM_REAL_POINTS];
	double outputSignal[MAX_NUM_REAL_POINTS];

	unsigned int numToSkip = 0;


	if( argc < 3 )
	{
		cout << "Usage: LowPassFilter.bin [filterFrequency] [numPointsInFilter] [0 or 1 (Az=0,El=1)] <controllawfilename.txt>" << endl;
		return 1;
	}
	else if( argc < 5 )
	{
		#if DEBUG_LPF
		cout << "Using generated data...." << endl;
		#endif
		cutoffFrequency = atof(argv[1]); // Hz
		numPoints = atof(argv[2]);
		genSignal(timeStamp, inputSignal, numPoints);
	}
	else
	{
		#if DEBUG_LPF
		cout << "Opening " << argv[1] << "..." << endl;
		#endif

		cutoffFrequency = atof(argv[1]); // Hz
		numPoints = atof(argv[2]);

		ifstream infile;
		infile.open(argv[4], ifstream::in);

		string line;
		std::vector<std::string> parsedLine;
		unsigned int cnt = 0;
		#if DEBUG_LPF
		printf("--------------\n");
		#endif

		// skip the first data point because it is slow:
		getline(infile, line);

		// skip a configurable amount of data in beginning:
		while( cnt < numToSkip )
		{
			getline(infile, line);
			cnt++;
		}

		cnt = 0;
		while( cnt < numPoints && getline(infile, line) )
		{
		    //cout << "line: " << line << endl;
		    // Parse input file:
		    parsedLine = split(line, ',');
		    timeStamp[cnt] = atol(parsedLine[1].c_str());
		    inputSignal[cnt] = atof(parsedLine[8 + atoi(argv[3])].c_str());
		    #if DEBUG_LPF
		    printf("initial: %f\n", inputSignal[cnt]);
		    #endif
		    cnt++;
		}

		if( cnt < numPoints )
		{
			cout << "Not enough data found in '" << argv[4] << "'. Are you sure the file exists?" << endl;
			return 1;
		}

	}

	LowPassFilter lpf;

	lpf.configure(numPoints);

	#if DEBUG_LPF
	cout << "Creating a LowPassFilter using " << numPoints << " points returned " 
		 << ret << endl;
	#endif

	lpf.filter(cutoffFrequency, timeStamp, inputSignal, outputSignal);

	#if DEBUG_LPF
	cout << "Filtering returned " << ret << endl;
	#endif

	lpf.printResults();

	#if DEBUG_LPF
	printf("--------------\n");
	for(unsigned int i = 0; i < numPoints; i++ )
		printf("final: %f\n", outputSignal[i]);
	cout << "Done testing." << endl;
	#endif

	return 0;
}