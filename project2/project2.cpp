#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include<omp.h>

#ifndef NUMT
#define NUMT  8
#endif

//numnodes
#ifndef NUMNODES
#define NUMNODES  10000
#endif

#define XMIN     -1.
#define XMAX      1.
#define YMIN     -1.
#define YMAX      1.

#define N	0.70

float
Height( int iu, int iv )	// iu,iv = 0 .. NUMNODES-1
{
	float x = -1.  +  2.*(float)iu /(float)(NUMNODES-1);	// -1. to +1.
	float y = -1.  +  2.*(float)iv /(float)(NUMNODES-1);	// -1. to +1.

	float xn = pow( fabs(x), (double)N );
	float yn = pow( fabs(y), (double)N );
	float r = 1. - xn - yn;
	if( r <= 0. )
	        return 0.;
	float height = pow( r, 1./(float)N );
	return height;
}

int main(int argc, char *argv[]) {

    #ifndef _OPENMP
            fprintf(stderr, "No openMP support\n");
            return 1;
    #endif

    omp_set_num_threads(NUMT);      //set up the number of threads for the loop

    // the area of a single full-sized tile:
	// (not all tiles are full-sized, though)

	float fullTileArea = (  ( ( XMAX - XMIN )/(float)(NUMNODES-1) )  *
				( ( YMAX - YMIN )/(float)(NUMNODES-1) )  );
    
    float halfTileArea = fullTileArea / 2;

    float quarterTileArea = fullTileArea / 4;

    //performance
    double maxPerformance = 0.;
    //what we're going to be summing all the heights up into with a reduction, needs to be
    //multiplied by 2 at the very end before printing
    double volume = 0.;

    double time0 = omp_get_wtime();

    #pragma omp parallel for default(none), shared(fullTileArea, halfTileArea, quarterTileArea, stderr), reduction(+:volume)
    for(int i = 0; i < NUMNODES * NUMNODES; i++){
        //the following three variables are private by default
        int iu = i % NUMNODES;
        int iv = i / NUMNODES;
        float z = Height(iu, iv);
        //check if we are on an edge
        if(iu == 0 || iu == NUMNODES - 1){
            //check if we are on a corner
            if(iv == 0 || iv == NUMNODES - 1){
                //if we are on a corner, multiply height by the quatertilearea
                z = z * quarterTileArea;
            }
            else{
                //just an edge, but not a corner, so we multiply by the halftilearea
                z = z * halfTileArea;
            }
        }
        else{
            //we are in the center somewhere, so we multiply by the fulltilearea
            z = z * fullTileArea;
        }
        volume += z;        //add z to the current total of volume
    }

    double time1 = omp_get_wtime();
    //units of performance
    double megaHeightsPerSecond = (double)NUMNODES*NUMNODES / (time1 - time0) / 1000000.;
    if(megaHeightsPerSecond > maxPerformance){
        maxPerformance = megaHeightsPerSecond;
    }

    //at the end, volume needs to be doubled
    volume = volume * 2.;
    fprintf(stderr, "threads: %2d ; numnodes: %8d ; volume: %6.2f ; megaheights/sec: %6.2lf ; time: %1.5lf\n",
                NUMT, NUMNODES, volume, megaHeightsPerSecond, (time1 - time0));
    
    return 0;
}