#include <stdio.h>
#include "fir1.h"
#include "fir2.h"
#define FIR_FILTER_BUFFER_SIZE 4096
#define DECIMATION_FACTOR 4

extern int sample_nbr;

/*  https://fiiir.com  s=252000, c=23000, Tb = 26000, Kaiser SA=60dB   */

#define FILTER_SIZE  37
static double filter_h[] = {
    -0.000281792569572607,
    -0.000251360578620161,
    0.000352714845005618,
    0.001713054514664199,
    0.003521302844075157,
    0.004809321525020601,
    0.004142355129614629,
    0.000251359504199421,
    -0.007058416160837216,
    -0.016118579814418200,
    -0.023209712806215206,
    -0.023300868039639527,
    -0.011669350602513945,
    0.014091475032761831,
    0.052470033032779466,
    0.097612533914950089,
    0.140463679436186456,
    0.171242462423331770,
    0.182439576738455317,
    0.171242462423331770,
    0.140463679436186456,
    0.097612533914950089,
    0.052470033032779466,
    0.014091475032761831,
    -0.011669350602513945,
    -0.023300868039639527,
    -0.023209712806215206,
    -0.016118579814418200,
    -0.007058416160837216,
    0.000251359504199421,
    0.004142355129614629,
    0.004809321525020597,
    0.003521302844075161,
    0.001713054514664200,
    0.000352714845005618,
    -0.000251360578620161,
    -0.000281792569572607
};
 
struct buffer_item
{
    double sample_I;
    double sample_Q;
};

struct buffer_item buffer_IQ[FIR_FILTER_BUFFER_SIZE];
static int buffer_ptr;
static int decimation_count;
static int freq_shift_idx;




void init_fir_filter1()
{
   int i;
	
   decimation_count = 0;
   buffer_ptr = 0;
   freq_shift_idx=0;
   for(i=0; i<FIR_FILTER_BUFFER_SIZE; i++)
   {
	buffer_IQ[i].sample_I = 0;
	buffer_IQ[i].sample_Q = 0;
   }
}

/*
void sample_in_1(double sample_I,double sample_Q)
{
// do a frequency shift of (minus) half the sample rate (1/4 of the total bandwidth of the full spectrum of I&Q)
// the way to perform a frequency shift of f1 on samples with a sample rate of s1 is to multiply I&Q (complex) with e^(j*2*pi*f1/s1 * n) with n the sample number
// if f1/s1 = 1/4 then you have to multiply with e^(O), e^(i*pi/2), e^(i*pi), e^(i*3*pi/2), ... or 1,j,-1,-j,1,j,-1, ....
 
	switch(freq_shift_idx) {
		case 0:
			fir_in_1(sample_I,sample_Q);
			break;

		case 1:
			fir_in_1(sample_Q,-sample_I);
			break;

		case 2:
			fir_in_1( -sample_I, -sample_Q);
			break;

		case 3:
			fir_in_1(-sample_Q,sample_I);
			break;
	}
	freq_shift_idx++;
	freq_shift_idx %= 4;
}
*/

void sample_in_1(double sample_I,double sample_Q)
{
   register int i;
   register int conv_buf_ptr;
   register double conv_sum_I;
   register double conv_sum_Q;
   register double filter_coef;

   //buffer_I[buffer_ptr] = sample_I;
   //buffer_Q[buffer_ptr] = sample_Q;
   
   buffer_IQ[buffer_ptr].sample_I = sample_I;
   buffer_IQ[buffer_ptr].sample_Q = sample_Q;

   decimation_count ++;
   if( (decimation_count % DECIMATION_FACTOR) == 0)
   {
	   conv_sum_I = 0.0;
	   conv_sum_Q = 0.0;
	   conv_buf_ptr = buffer_ptr;

	   if (buffer_ptr > FILTER_SIZE)
	   {
		   for(i=0; i < FILTER_SIZE; i++)
		   {
		      filter_coef=filter_h[i];
		      conv_sum_I += filter_coef * buffer_IQ[conv_buf_ptr].sample_I;
		      conv_sum_Q += filter_coef * buffer_IQ[conv_buf_ptr].sample_Q;
		      conv_buf_ptr--;
		   }
	   }
	   else
	   {
		   for(i=0; i < FILTER_SIZE; i++)
		   {
		      filter_coef=filter_h[i];
		      conv_sum_I += filter_coef * buffer_IQ[conv_buf_ptr].sample_I;
		      conv_sum_Q += filter_coef * buffer_IQ[conv_buf_ptr].sample_Q;
		      if(conv_buf_ptr == 0)
		      {
			 conv_buf_ptr = FIR_FILTER_BUFFER_SIZE-1;
		      }
		      else
		      {
		         conv_buf_ptr--;
		      }
		   }
	   }
		
	   sample_in_2(conv_sum_I,conv_sum_Q);
 
	   decimation_count = 0;
   }

   buffer_ptr++;
   buffer_ptr %= FIR_FILTER_BUFFER_SIZE;
}


