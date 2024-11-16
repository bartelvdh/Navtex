#include <stdio.h> 
#include "fir2.h"
#include "fir3.h"
#define FIR_FILTER_BUFFER_SIZE 1024
#define DECIMATION_FACTOR 7

extern int sample_nbr;

unsigned short freq_shift_idx;

#define FILTER_SIZE  49



static double filter_h[] = {
   -0.000129947925935584,
    -0.000305240554266564,
    -0.000572496538184548,
    -0.000929756870196358,
    -0.001352150761468049,
    -0.001785402621812617,
    -0.002141833509738044,
    -0.002300141931851463,
    -0.002109929475699450,
    -0.001401402865354654,
    0.000000000000000001,
    0.002255069210971520,
    0.005490092428429565,
    0.009776222333550674,
    0.015111536256190748,
    0.021408360804625826,
    0.028488055148479421,
    0.036084731743229487,
    0.043858416326694419,
    0.051417027280707059,
    0.058345433757715379,
    0.064238886217281313,
    0.068737441554297216,
    0.071557731363186952,
    0.072518597258295542,
    0.071557731363186952,
    0.068737441554297216,
    0.064238886217281313,
    0.058345433757715379,
    0.051417027280707059,
    0.043858416326694419,
    0.036084731743229459,
    0.028488055148479421,
    0.021408360804625826,
    0.015111536256190748,
    0.009776222333550674,
    0.005490092428429565,
    0.002255069210971520,
    0.000000000000000001,
    -0.001401402865354654,
    -0.002109929475699450,
    -0.002300141931851463,
    -0.002141833509738044,
    -0.001785402621812616,
    -0.001352150761468050,
    -0.000929756870196358,
    -0.000572496538184548,
    -0.000305240554266565,
    -0.000129947925935584
};
 
static double buffer_I[FIR_FILTER_BUFFER_SIZE];
static double buffer_Q[FIR_FILTER_BUFFER_SIZE];
static int buffer_ptr;
static int decimation_count;





void init_fir_filter2()
{
   int i;
	
   decimation_count = 0;

   buffer_ptr = 0;
   for(i=0; i<FIR_FILTER_BUFFER_SIZE; i++)
   {
      buffer_I[i]=0;
      buffer_Q[i]=0;
   }
   freq_shift_idx=0;
}

void sample_in_2(double sample_I,double sample_Q)
{
// do a frequency shift of (minus) half the sample rate (1/4 of the total bandwidth of the full spectrum of I&Q)
// the way to perform a frequency shift of f1 on samples with a sample rate of s1 is to multiply I&Q (complex) with e^(j*2*pi*f1/s1 * n) with n the sample number
// if f1/s1 = 1/4 then you have to multiply with e^(O), e^(i*pi/2), e^(i*pi), e^(i*3*pi/2), ... or 1,j,-1,-j,1,j,-1, ....

        switch(freq_shift_idx) {
                case 0:
                        fir_in_2(sample_I,sample_Q);
                        break;

                case 1:
                        fir_in_2(sample_Q,-sample_I);
                        break;

                case 2:
                        fir_in_2( -sample_I, -sample_Q);
                        break;

                case 3:
                        fir_in_2(-sample_Q,sample_I);
                        break;
        }
        freq_shift_idx++;
        freq_shift_idx %= 4;
}


void fir_in_2(double sample_I,double sample_Q)
{
   int i;
   int conv_buf_ptr;
   double conv_sum_I;
   double conv_sum_Q;
   double filter_coef;

   buffer_I[buffer_ptr] = sample_I;
   buffer_Q[buffer_ptr] = sample_Q;


   decimation_count++;
   if( (decimation_count % DECIMATION_FACTOR) == 0)
   {
	   conv_sum_I = 0.0;
	   conv_sum_Q = 0.0;
	   conv_buf_ptr = buffer_ptr;

	   for(i=0; i < FILTER_SIZE; i++)
	   {
 	      filter_coef=filter_h[i];
	      conv_sum_I += filter_coef * buffer_I[conv_buf_ptr];
	      conv_sum_Q += filter_coef * buffer_Q[conv_buf_ptr];
	      if(conv_buf_ptr == 0)
	      {
		 conv_buf_ptr = FIR_FILTER_BUFFER_SIZE-1;
	      }
	      else
	      {
		 conv_buf_ptr -= 1;
	      }
	   }
       sample_in_3(conv_sum_I,conv_sum_Q);

       decimation_count = 0;
   }

   buffer_ptr++;
   buffer_ptr %= FIR_FILTER_BUFFER_SIZE;
}


