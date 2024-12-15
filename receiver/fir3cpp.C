#include "fir3cpp.h"



fir_filter3::fir_filter3(decoder * dec)
{
   int i;
	
   decimation_count = 0;
   buffer_ptr = 0;

   for(i=0; i<FIR_FILTER_BUFFER_SIZE; i++)
   {
      buffer_I[i]=0;
      buffer_Q[i]=0;
   }
   output_dec = dec;
}



void fir_filter3::sample_in(double sample_I,double sample_Q)
{
   int i;
   int conv_buf_ptr;
   double conv_sum_I;
   double conv_sum_Q;
   double filter_coef;

   buffer_I[buffer_ptr] = sample_I;
   buffer_Q[buffer_ptr] = sample_Q;

   decimation_count++;
   if( (decimation_count % DECIMATION_FACTOR_3) == 0)
   {
	   conv_sum_I = 0.0;
	   conv_sum_Q = 0.0;
	   conv_buf_ptr = buffer_ptr;

	   for(i=0; i < FILTER_SIZE_3; i++)
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
	output_dec->sample_in(conv_sum_I,conv_sum_Q); 
        decimation_count = 0;
   }

   buffer_ptr++;
   buffer_ptr %= FIR_FILTER_BUFFER_SIZE;
}

