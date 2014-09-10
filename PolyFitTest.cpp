#include "PolyFit.h"
#include <iostream>

int main ()
{
	cout << "Creating a poly fitter 9000!" << endl;

	// We must padd the dataset with 0,0 first element for this
	// 11 element set to work with the 1-indexed polyfit
	/*
	double data[12][2] =   {{0.0000000000, 0.0000000000},
							{0.0000000000, 0.0000000000},
							{0.0600099840, 3.8650850800},
							{0.1200017920, 2.6375739500},
							{0.1799564800, 0.9647171460},
							{0.2400128000, 0.5100284640},
							{0.2999490560, 2.5608219600},
							{0.3599777280, 4.2344544400},
							{0.4199508480, 3.6824732600},
							{0.4799424000, 1.5011326800},
							{0.5399434240, -0.7833248330},
							{0.6599360000, 0.0000000000}};
	*/
							
	double data2[4][2] = 
	 {{0.000000, 0.000000},
	  {0.000000, 0.000000},
	  {0.999975, 72.366537},
	  {1.999913, -68.656711}};

	double x[5] = {0.000000, 0.999975, 1.999913, 3, 17};
	double y[5] = {0.000000, 72.366537, -68.656711, 115.689, 300};

	//
	// The correct answer should be:
	// Creating a poly fitter 9000!
	// Coeffs: 0 179.07 -106.705 0 
	// Calculation took: 95396 ns
	// Test es fine!
	//

    long double coeffs[9];
	polyfit( 3, x, y, 5, coeffs);

	cout << "Coeffs: ";
	for( int i = 0; i < 4; i++ )
		cout << coeffs[i] << " ";
	cout << endl;

	// Should return "Coeffs: 1.10204 10.5739 -18.5839 -1.81836"

	cout << "Test es fine!" << endl;
	return 0;
}