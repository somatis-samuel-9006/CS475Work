#include "simd.p5.h"


#ifndef NUMTRIALS
#define NUMTRIALS 10
#endif

//size of the 3 arrays
#ifndef DATASET
#define DATASET 8000
#endif


//input arrays
float a[DATASET];
float b[DATASET];
float c[DATASET];

void
SimdMul( float *a, float *b,   float *c,   int len )
{
	int limit = ( len/SSE_WIDTH ) * SSE_WIDTH;
	__asm
	(
		".att_syntax\n\t"
		"movq    -24(%rbp), %r8\n\t"		// a
		"movq    -32(%rbp), %rcx\n\t"		// b
		"movq    -40(%rbp), %rdx\n\t"		// c
	);

	for( int i = 0; i < limit; i += SSE_WIDTH )
	{
		__asm
		(
			".att_syntax\n\t"
			"movups	(%r8), %xmm0\n\t"	// load the first sse register
			"movups	(%rcx), %xmm1\n\t"	// load the second sse register
			"mulps	%xmm1, %xmm0\n\t"	// do the multiply
			"movups	%xmm0, (%rdx)\n\t"	// store the result
			"addq $16, %r8\n\t"
			"addq $16, %rcx\n\t"
			"addq $16, %rdx\n\t"
		);
	}

	for( int i = limit; i < len; i++ )
	{
		c[i] = a[i] * b[i];
	}
}

//non simd arraymult function
void
arrMul( float *a, float *b, float *c, int len)
{
	int i;
	for(i = 0; i < len; i++){
		c[i] = a[i] * b[i];
	}
}


float
SimdMulSum( float *a, float *b, int len )
{
	float sum[4] = { 0., 0., 0., 0. };
	int limit = ( len/SSE_WIDTH ) * SSE_WIDTH;

	__asm
	(
		".att_syntax\n\t"
		"movq    -40(%rbp), %r8\n\t"		// a
		"movq    -48(%rbp), %rcx\n\t"		// b
		"leaq    -32(%rbp), %rdx\n\t"		// &sum[0]
		"movups	 (%rdx), %xmm2\n\t"		// 4 copies of 0. in xmm2
	);

	for( int i = 0; i < limit; i += SSE_WIDTH )
	{
		__asm
		(
			".att_syntax\n\t"
			"movups	(%r8), %xmm0\n\t"	// load the first sse register
			"movups	(%rcx), %xmm1\n\t"	// load the second sse register
			"mulps	%xmm1, %xmm0\n\t"	// do the multiply
			"addps	%xmm0, %xmm2\n\t"	// do the add
			"addq $16, %r8\n\t"
			"addq $16, %rcx\n\t"
		);
	}

	__asm
	(
		".att_syntax\n\t"
		"movups	 %xmm2, (%rdx)\n\t"	// copy the sums back to sum[ ]
	);

	for( int i = limit; i < len; i++ )
	{
		sum[0] += a[i] * b[i];
	}

	return sum[0] + sum[1] + sum[2] + sum[3];
}

int main() {


	//fill arrays a and b with 1.0
	int i;
	for(i = 0; i < DATASET; i++){
		a[i] = 1.0;
		b[i] = 1.0;
	}

	double peakTime = 0.0;
	for (i = 0; i < NUMTRIALS; i++){

		//simd multiplication
		double time0 = omp_get_wtime();
		SimdMul(a, b, c, DATASET);
		double time1 = omp_get_wtime();

		//calculate elapsed time with unit change
		double Tsse = (time1 - time0) * 1000000.;

		//non simd multiplication
		// double time3 = omp_get_wtime();
		// arrMul(a, b, c, DATASET);
		// double time4 = omp_get_wtime();

		// double Tnonsse = (time4 - time3) * 1000000.;

		//get longest time
		if(Tsse > peakTime){
			peakTime = Tsse;
		}
	}

	fprintf(stderr, "Array size: %d  Elasped Time: %6.2lf\n", DATASET, peakTime);

	return 0;
}