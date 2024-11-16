#include <stdio.h> 
#include "fir3.h"
#define FIR_FILTER_BUFFER_SIZE 1024
#define DECIMATION_FACTOR 6

extern int sample_nbr;

#define FILTER_SIZE  37



static double filter_h[] = {
    0.000000000000000000,
    0.000117760253350358,
    0.000452222831487449,
    0.001140565412231802,
    0.002341523938197725,
    0.004220031412310397,
    0.006926642563249298,
    0.010573885665773409,
    0.015212562897002178,
    0.020811522954121579,
    0.027244399650179992,
    0.034286210211824500,
    0.041621574885609802,
    0.048864797327432534,
    0.055590352803414615,
    0.061370731976606904,
    0.065817345362456009,
    0.068619524436030463,
    0.069576690837441890,
    0.068619524436030463,
    0.065817345362456009,
    0.061370731976606904,
    0.055590352803414615,
    0.048864797327432534,
    0.041621574885609802,
    0.034286210211824500,
    0.027244399650179992,
    0.020811522954121579,
    0.015212562897002178,
    0.010573885665773409,
    0.006926642563249298,
    0.004220031412310393,
    0.002341523938197729,
    0.001140565412231803,
    0.000452222831487449,
    0.000117760253350358,
    0.000000000000000000,
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
}



void sample_in_2(double sample_I,double sample_Q)
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
       //printf("fir2 to fir3 %d \n",sample_nbr);
       sample_in_3(conv_sum_I,conv_sum_Q);

       decimation_count = 0;
   }

   buffer_ptr++;
   buffer_ptr %= FIR_FILTER_BUFFER_SIZE;
}


