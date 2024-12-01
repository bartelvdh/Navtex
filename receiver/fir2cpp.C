#include <stdio.h> 
#include <math.h> 
#include "fir2cpp.h"
#define FIR_FILTER_BUFFER_SIZE 1024
#define DECIMATION_FACTOR_2 7

unsigned short freq_shift_idx;

#define FILTER_SIZE_2  47

//NOTE: FS/Freq_Shift = 4,5 hence 9 elements are needed for the freq shift filter
#define FS_IN 63000 
#define FREQ_SHIFT 14000
#define FREQ_SHIFT_FILTER_SIZE 9


static double freq_shift_coef_real[FREQ_SHIFT_FILTER_SIZE];
static double freq_shift_coef_img[FREQ_SHIFT_FILTER_SIZE];



/*  https://fiiir.com  s=63000, c=2300, Tb = 5500, Kaiser SA=65dB   */

static double filter_h[] = {
    -0.000147108163274380,
    -0.000344080458697122,
    -0.000638246787779785,
    -0.001020551993014626,
    -0.001453156218493991,
    -0.001862463815197339,
    -0.002135768595997864,
    -0.002123044308901244,
    -0.001644885141859342,
    -0.000506805672445239,
    0.001480845849197846,
    0.004479099821734249,
    0.008595303819273255,
    0.013859966338180792,
    0.020208854764962980,
    0.027473387873362013,
    0.035381576745767467,
    0.043570581210619269,
    0.051610508057461923,
    0.059037578414322979,
    0.065393451564912761,
    0.070266515161527154,
    0.073330495501089402,
    0.074375892066497792,
    0.073330495501089402,
    0.070266515161527154,
    0.065393451564912802,
    0.059037578414322937,
    0.051610508057461923,
    0.043570581210619269,
    0.035381576745767467,
    0.027473387873362013,
    0.020208854764962980,
    0.013859966338180792,
    0.008595303819273255,
    0.004479099821734245,
    0.001480845849197846,
    -0.000506805672445239,
    -0.001644885141859342,
    -0.002123044308901244,
    -0.002135768595997864,
    -0.001862463815197341,
    -0.001453156218493992,
    -0.001020551993014626,
    -0.000638246787779785,
    -0.000344080458697122,
    -0.000147108163274380
};
 
static double buffer_I[FIR_FILTER_BUFFER_SIZE];
static double buffer_Q[FIR_FILTER_BUFFER_SIZE];
static int buffer_ptr;
static int decimation_count;


static double buffer_I_490[FIR_FILTER_BUFFER_SIZE];
static double buffer_Q_490[FIR_FILTER_BUFFER_SIZE];
static int buffer_ptr_490;
static int decimation_count_490;

static fir_filter3 * out_ff3_518;
static fir_filter3 * out_ff3_490;



void init_fir_filter2(fir_filter3 * ff3_518, fir_filter3 * ff3_490)
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

	for(i=0;i<FREQ_SHIFT_FILTER_SIZE;i++){
		freq_shift_coef_real[i] = cos( (2*M_PI*i*FREQ_SHIFT) /FS_IN);
		freq_shift_coef_img[i] = -sin( (2*M_PI*i*FREQ_SHIFT) /FS_IN);
	}
   out_ff3_518 = ff3_518;
   out_ff3_490 = ff3_490;
}

void sample_in_2(double sample_I,double sample_Q)
{
	//perform a freq shift
        fir_in_2(
                 sample_I*freq_shift_coef_real[freq_shift_idx]-sample_Q*freq_shift_coef_img[freq_shift_idx],
                 sample_I*freq_shift_coef_img[freq_shift_idx]+sample_Q*freq_shift_coef_real[freq_shift_idx]
	);


        fir_in_2_490(
                 sample_I*freq_shift_coef_real[freq_shift_idx]+sample_Q*freq_shift_coef_img[freq_shift_idx],
                 -sample_I*freq_shift_coef_img[freq_shift_idx]+sample_Q*freq_shift_coef_real[freq_shift_idx]
	);

        freq_shift_idx++;
        freq_shift_idx %= FREQ_SHIFT_FILTER_SIZE;
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
   if( (decimation_count % DECIMATION_FACTOR_2) == 0)
   {
	   conv_sum_I = 0.0;
	   conv_sum_Q = 0.0;
	   conv_buf_ptr = buffer_ptr;

	   for(i=0; i < FILTER_SIZE_2; i++)
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
       out_ff3_518->sample_in(conv_sum_I,conv_sum_Q);

       decimation_count = 0;
   }

   buffer_ptr++;
   buffer_ptr %= FIR_FILTER_BUFFER_SIZE;
}



void fir_in_2_490(double sample_I,double sample_Q)
{
   int i;
   int conv_buf_ptr;
   double conv_sum_I;
   double conv_sum_Q;
   double filter_coef;

   buffer_I_490[buffer_ptr_490] = sample_I;
   buffer_Q_490[buffer_ptr_490] = sample_Q;


   decimation_count_490++;
   if( (decimation_count_490 % DECIMATION_FACTOR_2) == 0)
   {
	   conv_sum_I = 0.0;
	   conv_sum_Q = 0.0;
	   conv_buf_ptr = buffer_ptr_490;

	   for(i=0; i < FILTER_SIZE_2; i++)
	   {
 	      filter_coef=filter_h[i];
	      conv_sum_I += filter_coef * buffer_I_490[conv_buf_ptr];
	      conv_sum_Q += filter_coef * buffer_Q_490[conv_buf_ptr];
	      if(conv_buf_ptr == 0)
	      {
		 conv_buf_ptr = FIR_FILTER_BUFFER_SIZE-1;
	      }
	      else
	      {
		 conv_buf_ptr -= 1;
	      }
	   }
       out_ff3_490->sample_in(conv_sum_I,conv_sum_Q);

       decimation_count_490 = 0;
   }

   buffer_ptr_490++;
   buffer_ptr_490 %= FIR_FILTER_BUFFER_SIZE;
}

