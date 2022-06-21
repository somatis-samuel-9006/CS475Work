#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include<omp.h>

//GLOBALS:

//the state
int	NowYear;		// 2021 - 2026
int	NowMonth;		// 0 - 11

float NowPrecip;		// inches of rain per month
float NowTemp;		// temperature this month
float NowHeight;		// grain height in inches
int	NowNumDeer;		// number of deer in the current population
int NowNumLions;     // number of mountain lions

//other params
const float GRAIN_GROWS_PER_MONTH =		9.0;
const float ONE_DEER_EATS_PER_MONTH =		1.0;

const float AVG_PRECIP_PER_MONTH =		7.0;	// average
const float AMP_PRECIP_PER_MONTH =		6.0;	// plus or minus
const float RANDOM_PRECIP =			2.0;	// plus or minus noise

const float AVG_TEMP =				60.0;	// average
const float AMP_TEMP =				20.0;	// plus or minus
const float RANDOM_TEMP =			10.0;	// plus or minus noise

const float MIDTEMP =				40.0;
const float MIDPRECIP =				10.0;

//seed for random
unsigned int seed = 0;

float ang = (  30.*(float)NowMonth + 15.  ) * ( M_PI / 180. );

float temp = AVG_TEMP - AMP_TEMP * cos( ang );
float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );

//for generating random numbers

float
Ranf( unsigned int *seedp,  float low, float high )
{
        float r = (float) rand_r( seedp );              // 0 - RAND_MAX

        return(   low  +  r * ( high - low ) / (float)RAND_MAX   );
}


int
Ranf( unsigned int *seedp, int ilow, int ihigh )
{
        float low = (float)ilow;
        float high = (float)ihigh + 0.9999f;

        return (int)(  Ranf(seedp, low,high) );
}


//for squaring in temp and percipitation factor
float
SQR( float x )
{
        return x*x;
}

//components of simulation

void Deer(){
    while( NowYear < 2027 )
    {
        //fprintf(stderr, "In deer function, NowNumdeer = %d and NowHeight = %6.2f\n", NowNumDeer, NowHeight);
        // compute a temporary next-value for this quantity
        // based on the current state of the simulation:
        int nextNumDeer = NowNumDeer;
        int carryingCapacity = (int)( NowHeight );

        if( nextNumDeer < carryingCapacity )
            nextNumDeer++;
        else
            if( nextNumDeer > carryingCapacity )
                nextNumDeer--;
        //mountain lions
        int oneinthree = Ranf(&seed, 1, 12);
        if(NowNumLions == 1 && nextNumDeer >= 1 && (oneinthree >=5 && oneinthree <= 8)){
            nextNumDeer--;
        }
        if(NowNumLions > 1 && nextNumDeer >= 1 && (oneinthree >=5 && oneinthree <= 8)){
            nextNumDeer--;
            nextNumDeer--;
        }        
        //clamp against 0, cant have less than 0 deer
        if( nextNumDeer < 0 )
            nextNumDeer = 0;
        

        // DoneComputing barrier:
        #pragma omp barrier
        //now, we assing the local variable computed above into the global variable
        NowNumDeer = nextNumDeer;
        //fprintf(stderr, "we hit the assigning part in deer, NowNumdeer = %d\n", NowNumDeer);

        // DoneAssigning barrier:
        #pragma omp barrier
        //do nothing here, the watcher thread is doing its work

        // DonePrinting barrier:
        #pragma omp barrier
        //now we restart from the top with the next month
        //fprintf(stderr, "Deer function going back to top\n");
    }
}

void Grain(){
    while( NowYear < 2027 )
    {
        // compute a temporary next-value for this quantity
        // based on the current state of the simulation:
        //fprintf(stderr, "In grain, NowTemp = %6.2f and NowPrecip = %6.2f\n", NowTemp, NowPrecip);
        float tempFactor = exp(   -SQR(  ( NowTemp - MIDTEMP ) / 10.  )   );
        float precipFactor = exp(   -SQR(  ( NowPrecip - MIDPRECIP ) / 10.  )   );

        float nextHeight = NowHeight;
        nextHeight += tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH;
        nextHeight -= (float)NowNumDeer * ONE_DEER_EATS_PER_MONTH;

        if( nextHeight < 0. ) 
            nextHeight = 0.;

        // DoneComputing barrier:
        #pragma omp barrier
        //copy local grain height into global current grain height
        NowHeight = nextHeight;
        //fprintf(stderr, "we hit the assinging part in grain, Nowheight = %6.2f\n", NowHeight);

        // DoneAssigning barrier:
        #pragma omp barrier
        //do nothing, watching is doing its work

        // DonePrinting barrier:
        #pragma omp barrier
        //restart from the top with the next month
        //fprintf(stderr, "grain function going back to top\n");

    }
}

void Watcher(){
    while( NowYear < 2027 )
    {
        // watcher is watching
        //fprintf(stderr, "watcher watching before done computing\n");

        // DoneComputing barrier:
        #pragma omp barrier
        //watcher is watching
        //fprintf(stderr, "watcher watching before done assinging\n");
        // DoneAssigning barrier:
        #pragma omp barrier
        //print the current state
        fprintf(stderr, "%6.2f\n", ( (5./9.) * (NowTemp - 32) ) );
        //increment month count
        NowMonth++;
        //fprintf(stderr, "NowMonth is %d\n", NowMonth);
        if(NowMonth > 11){
            //fprintf(stderr, "I'm here\n");
            NowMonth = 0;
            NowYear++;
        }

        //compute new temp and percip using the current month
        //float ang2 = (  30.*(float)NowMonth + 15.  ) * ( M_PI / 180. );

        //float temp2 = AVG_TEMP - AMP_TEMP * cos( ang2 );
        ang = (  30.*(float)NowMonth + 15.  ) * ( M_PI / 180. );

        temp = AVG_TEMP - AMP_TEMP * cos( ang );
        precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );
        NowTemp = temp + Ranf( &seed, -RANDOM_TEMP, RANDOM_TEMP );

        //float precip2 = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang2 );
        NowPrecip = precip + Ranf( &seed,  -RANDOM_PRECIP, RANDOM_PRECIP );

        if( NowPrecip < 0. )
            NowPrecip = 0.;        

        // DonePrinting barrier:
        #pragma omp barrier
        //watcher is done, restart with next month
        //fprintf(stderr, "watcher going back to top\n");
    }
}

void MountainLion(){
    //if there are atleast 3 deer in the area, a mountain lion will show up
    //if there are 6 deer, 2 mountain lions will show up
    //if the number drops back to 3 or less, one mtn lion will leave
    //if there is one mountain lion, there is a 1 in three chance that a deer dies
    //if there are two mountain lions, two deer die if the chance succeeds
    while( NowYear < 2027 )
    {
        // compute a temporary next-value for this quantity
        // based on the current state of the simulation:
        int nextNumLions = NowNumLions;

        if(NowNumDeer >= 3 && NowNumDeer <= 5){
            nextNumLions = 1;
        }
        else if(NowNumDeer > 5){
            nextNumLions = 2;
        }
        else{
            //lions leave because there are less than 3 deer
            nextNumLions = 0;
        }

        // DoneComputing barrier:
        #pragma omp barrier
        NowNumLions = nextNumLions;
        

        // DoneAssigning barrier:
        #pragma omp barrier
        

        // DonePrinting barrier:
        #pragma omp barrier
        
    }
}

int main(int argc, char *argv[]){

    #ifndef _OPENMP
        fprintf(stderr, "No openMP support\n");
        return 1;
    #endif
    float x = Ranf( &seed, -1.f, 1.f );
    // starting date and time:
    NowMonth =    0;
    NowYear  = 2021;

    // starting state (feel free to change this if you want):
    NowNumDeer = 1;
    NowHeight =  1.;
    NowNumLions = 0;

    //compute nowTemp and nowPrecip for starting month

    NowTemp = temp + Ranf( &seed, -RANDOM_TEMP, RANDOM_TEMP );

    NowPrecip = precip + Ranf( &seed,  -RANDOM_PRECIP, RANDOM_PRECIP );

    if( NowPrecip < 0. )
        NowPrecip = 0.;

    //fprintf(stderr, "In main, NowTemp = %6.2f and NowPrecip = %6.2f\n", NowTemp, NowPrecip);
    omp_set_num_threads(4);	// same as # of sections

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            Deer();
        }

        #pragma omp section
        {
            Grain();
        }

        #pragma omp section
        {
            MountainLion();
        }

        #pragma omp section
        {
            Watcher();
        }
	}

    //after the parallel sections has ended, the simulation is done
    fprintf(stderr, "\nThe simulation has finished\n");

    return 0;
}