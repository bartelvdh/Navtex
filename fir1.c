#include <stdio.h>
#include "fir1.h"
#include "fir2.h"
#define FIR_FILTER_BUFFER_SIZE 4096
#define DECIMATION_FACTOR 4

extern int sample_nbr;

#define FILTER_SIZE  35
static double filter_h[] = {
  -0.000282304309686843,
    -0.000543120959375865,
    -0.000336613100371430,
    0.000886369105899062,
    0.003175918941373535,
    0.005527386193468761,
    0.005857928064102629,
    0.001898093815410678,
    -0.007142512866688993,
    -0.018931231228965312,
    -0.027530244258189032,
    -0.024895486438640022,
    -0.004203016819645970,
    0.036309024519465993,
    0.090941116270532973,
    0.147160271546502963,
    0.189491326085450745,
    0.205234190878712031,
    0.189491326085450745,
    0.147160271546502963,
    0.090941116270532973,
    0.036309024519465993,
    -0.004203016819645970,
    -0.024895486438640022,
    -0.027530244258189014,
    -0.018931231228965312,
    -0.007142512866688999,
    0.001898093815410678,
    0.005857928064102629,
    0.005527386193468761,
    0.003175918941373535,
    0.000886369105899062,
    -0.000336613100371430,
    -0.000543120959375865,
    -0.000282304309686843
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


