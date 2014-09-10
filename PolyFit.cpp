#include "PolyFit.h"


#if POLYFIT_TIMING
	StopWatch fullPolyFitSW, minMaxSW, scaleFactorSW, startPolyFitSW,blk1SW,blk2SW,blk3SW,blk4SW,zeroCoeffsSW;
	void printTiming();
#endif

long double a[MAXNUMFITPOINTS+1][MAXNUMFITPOINTS+1], B[MAXNUMFITPOINTS+1];
long double S[MAXNUMFITPOINTS+1];

int polyfit(int order, double x[], double y[], int length, long double (&polycoefs)[MAXORDER+1])
{
	// TO BIG TO PROCESS! This is an optimization and MAXNUMFITPOINTS can be made bigger if necessary
	if( length > MAXNUMFITPOINTS )
		return -1;

	#if POLYFIT_TIMING
    	fullPolyFitSW.start();
    #endif

    #if POLYFIT_TIMING
   	 	minMaxSW.start();
    #endif
	long double xmin, xmax, ymin, ymax;

	// Find mins and maxes of data:
	xmin = x[0]; xmax = x[0];
	ymin = y[0]; ymax = y[0];

	for(int i = 0; i < length; i++ )
	{
		if( x[i] < xmin ) xmin = x[i];
		if( x[i] > xmax ) xmax = x[i];
		if( y[i] < ymin ) ymin = y[i];
		if( y[i] > ymax ) ymax = y[i];
	}
	#if POLYFIT_TIMING
   	 	minMaxSW.stop();
    #endif
	/* Polynomial Interpolation
	~~~~~~~~~~~~~~~~~~~~~~~~*/
     /* NB Data array uses elements [1][x] to [length][x]  */
     /*    All data values MUST be positive                    */

     /*--------------------------------------------------------*/
     /*    This algorithm has been translated and developed    */
     /*    from an original Acorn BBC/BBC Basic programme      */
     /*    (ref. D.G.K.Guy, Practical Computing, May 1985,     */
     /*    page 135)                                           */
     /*    I still don't understand how it works!              */
     /*--------------------------------------------------------*/

     /* dimension arrays/variables
	~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	// long double a[MAXNUMFITPOINTS+1][MAXNUMFITPOINTS+1], B[MAXNUMFITPOINTS+1];
	// long double S[MAXNUMFITPOINTS+1];
	long double A1, A2, Y1, m, S1, x1;
	long double xscale = 1;
	long double yscale = 1;
	long double xlow = xmax;
	long double ylow = ymax;

	int i, j, k, L, R;

	/* set scaling factors
	~~~~~~~~~~~~~~~~~~~~~~*/
	/* -- Avoids having to manage very large/very small numbers in polynomial
	      calculation
              See rescaling ruitine at end of this function
	*/
    //for( i = 1; i <= length; i++ )
		//printf( "x: %f y: %f\n", data_array[i][0], data_array[i][1]);
    #if POLYFIT_TIMING
   	 	scaleFactorSW.start();
    #endif

	for( i = 0; i < length; i++ )
	{
		if( x[i] < xlow && x[i] != 0 ) xlow = x[i];
	}

	if( xlow != 0 && xmax != 0) // This protects against the 0 case
	{
		if( xlow < .001 && xmax < 1000 ) xscale = 1 / xlow;
		else if( xmax > 1000 && xlow > .001 ) xscale = 1 / xmax;
	}

	for( i = 0; i < length; i++ )
	{
		if( y[i] < ylow && y[i] != 0 ) ylow = y[i];
	}

	if( ylow != 0 && ymax != 0 )
	{
		if( ylow < .001 && ymax < 1000 ) yscale = 1 / ylow;
		else if( ymax > 1000 && ylow > .001 ) yscale = 1 / ymax;
	}

    /* initialise array variables
	~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	// for(i = 0; i < MAXNUMFITPOINTS; i++ )
	// {
	// 	B[i] = 0; S[i] = 0;
	// 	for( j = 0; j < MAXNUMFITPOINTS; j++ )
	// 		a[i][j] = 0;
	// }
	bzero(a, sizeof(a));
	bzero(B, sizeof(B));
	bzero(S, sizeof(S));

	// for(i = 0; i <= MAXORDER; i++ )
	// 	polycoefs[i] = 0;	
	bzero(polycoefs, sizeof(polycoefs));

	#if POLYFIT_TIMING
   	 	scaleFactorSW.stop();
    #endif
     /* ensure all data is in ascending order wrt x
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	// datasort();
	// We garauntee that all data is in ascending order wrt x

     /* start of polynomial fit
	~~~~~~~~~~~~~~~~~~~~~~~*/
   	#if POLYFIT_TIMING
   	 	startPolyFitSW.start();
    #endif
	Y1 = 0;
	for( j = 0; j < length; j++ )
	{
		for( i = 1; i <= order; i++ )
		{
			B[i] = B[i] + y[j] * yscale * ldpow( x[j] * xscale, i );

			if( B[i] == LDBL_MAX ) 
				return -1;

			for( k = 1; k <= order; k++ )
		    {
		        a[i][k] = a[i][k] + ldpow( x[j] * xscale, (i + k) );

		        if( a[i][k] == LDBL_MAX ) 
		        	return -1;
	        }

			S[i] = S[i] + ldpow( x[j] * xscale, i );

			if( S[i] == LDBL_MAX ) 
				return -1;
		}

		Y1 = Y1 + y[j] * yscale;
		if( Y1 == LDBL_MAX ) 
			return -1;
	}
	#if POLYFIT_TIMING
   	 	startPolyFitSW.stop();
    #endif


	/*-----------------------------------------------------------------*/
   	#if POLYFIT_TIMING
   	 	blk1SW.start();
    #endif
	for( i = 1; i<= order; i++ )
	{
		for( j = 1; j <= order; j++ )
		{
			a[i][j] = a[i][j] - S[i] * S[j] / (long double) length;
		
			if( a[i][j] == LDBL_MAX )
				return -1;
		}

		B[i] = B[i] - Y1 * S[i] / (long double) length;

		if( B[i] == LDBL_MAX ) 
			return -1;
	}
	#if POLYFIT_TIMING
   	 	blk1SW.stop();
    #endif


	/*-----------------------------------------------------------------*/
   	#if POLYFIT_TIMING
   	 	blk2SW.start();
    #endif
	for( k = 1; k <= order; k++ )
	{
		R = k; A1 = 0;
		for( L = k; L <= order; L++ )
		{
			A2 = fabsl( a[L][k] );
			if( A2 > A1 ){ A1 = A2; R = L;}
		}

		if( A1 == 0 ) 
			return -1;
		if( R == k ) 
			goto polfit1;

		for( j = k; j <= order; j++ )
		{
			x1 = a[R][j]; a[R][j] = a[k][j]; a[k][j] = x1;
		}

		x1 = B[R]; B[R] = B[k]; B[k] = x1;
		
		polfit1:       
		for( i = k; i <= order; i++ )
		{
			m = a[i][k];
			for( j = k; j <= order; j++ )
			{ 
				if( i == k )
					a[i][j] = a[i][j] / m;
				else
					a[i][j] = a[i][j] - m * a[k][j];
			}

			if( i == k )
				B[i] = B[i] / m;
		    else
				B[i] = B[i] - m * B[k];
		}
	}
	#if POLYFIT_TIMING
   	 	blk2SW.stop();
    #endif

	/*-----------------------------------------------------------------*/
   	#if POLYFIT_TIMING
   	 	blk3SW.start();
    #endif
	polycoefs[order] = B[order];
	for( k = 1; k <= order - 1; k++ )
	{
		i = order - k; S1 = 0;

		for( j = 1; j <= order; j++ )
		{
			S1 = S1 + a[i][j] * polycoefs[j];
			if( S1 == LDBL_MAX ) 
				return -1;
		}

		polycoefs[i] = B[i] - S1;
	}
	#if POLYFIT_TIMING
   	 	blk3SW.stop();
    #endif

	/*-----------------------------------------------------------------*/
   	#if POLYFIT_TIMING
   	 	blk4SW.start();
    #endif
	S1 = 0;
	for( i = 1; i <= order; i++ )
    {
        S1 = S1 + polycoefs[i] * S[i] / (long double)length;

        if( S1 == LDBL_MAX ) 
        	return -1;
	}

	polycoefs[0] = (Y1 / (long double)length - S1);

	#if POLYFIT_TIMING
   	 	blk4SW.stop();
    #endif

	/*-----------------------------------------------------------------*/
	/* zero all coeficient values smaller than +/- .000001 (avoids -0)
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
   	#if POLYFIT_TIMING
   	 	zeroCoeffsSW.start();
    #endif
	for( i = 0; i <= order; i++ )
	{
		if( fabsl( polycoefs[i] * 1000000 ) < 1 ) 
			polycoefs[i] = 0;
	}	

	/* rescale parameters
	~~~~~~~~~~~~~~~~~~~~~*/
	for( i = 0; i <= order; i++ )
		polycoefs[i] = ( 1 / yscale ) * polycoefs[i] * ldpow( xscale, i );

	#if POLYFIT_TIMING
   	 	zeroCoeffsSW.stop();
    #endif

	#if POLYFIT_TIMING
    	fullPolyFitSW.stop();
    #endif

    #if POLYFIT_TIMING
   	 	printTiming();
    #endif
	/* No errors so return 'true'
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	return 0;
}

double polyval(int order, long double x, long double Coeffs[9])
{
	long double X[MAXORDER+1];
	bzero(X, sizeof(X));

	// Calculate the x-axis powers:
	X[0] = 1.0;
    for( int i=1; i<(order+1); i++ )
        X[i] = x*X[i-1];

    // Evaluate the polynomial for y at the given x value:
    double y = 0.0;
    for( int i=0; i<(order+1); i++ )
            y += Coeffs[i] * X[i];

    return y;
}

long double ldpow( long double n, unsigned p )
{
    /* Calculates n to the power (unsigned integer) p
     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    /*--------------------------------------------------------*/
    /* This function returns a long double value whereas the  */
    /* standard function 'pow' returns a double value.        */
    /*                                                        */
    /* Also, this function gives smaller errors than 'pow'    */
    /* where only INTEGER powers are required.                */
    /*--------------------------------------------------------*/
    long double x = 1;
    unsigned i;

    for(i = 0; i < p; i++ ) x = x * n;

    return x;
}

#if POLYFIT_TIMING
void printTiming()
{
    cout << "      TIMES:          " << endl;
    cout << "         MinMax time: " << minMaxSW.report()/1000000.0 << "ms" << endl;
    cout << "  Scatle Factor time: " << scaleFactorSW.report()/1000000.0 << "ms" << endl;
    cout << "      Start Fit time: " << startPolyFitSW.report()/1000000.0 << " ms" << endl;
    cout << "        Block 1 time: " << blk1SW.report()/1000000.0 << " ms" << endl;
    cout << "        Block 2 time: " << blk2SW.report()/1000000.0 << " ms" << endl;
    cout << "        Block 3 time: " << blk3SW.report()/1000000.0 << " ms" << endl;
    cout << "        Block 4 time: " << blk4SW.report()/1000000.0 << " ms" << endl;
    cout << "    Zero Coeffs time: " << zeroCoeffsSW.report()/1000000.0 << "ms" << endl;
    cout << "   Full Polyfit time: " << fullPolyFitSW.report()/1000000.0 << "ms" << endl;
}
#endif