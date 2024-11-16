#include "quad_mod.h"
#define FIR_FILTER_BUFFER_SIZE 1024
#define DECIMATION_FACTOR 8


/*  https://fiiir.com  s=8000, c=250, Tb = 400, Kaiser SA=50dB   */ 


#define FILTER_SIZE 61

static double filter_h[] = {
   -0.000224292751269908,
    -0.000460263323995915,
    -0.000790619167168023,
    -0.001214173888151644,
    -0.001718922780864195,
    -0.002280274180310769,
    -0.002860010226536093,
    -0.003406177517088067,
    -0.003854054066021719,
    -0.004128266718722614,
    -0.004146047369803361,
    -0.003821523227979228,
    -0.003070843302818178,
    -0.001817858108731060,
    0.000000000000000001,
    0.002426036182199808,
    0.005479229838401821,
    0.009150196390057992,
    0.013398536981688998,
    0.018151864198893832,
    0.023306691490530721,
    0.028731253784668880,
    0.034270195484424087,
    0.039750929341535547,
    0.044991345774770056,
    0.049808446806253924,
    0.054027400530101778,
    0.057490467586282790,
    0.060065244665458317,
    0.061651702900708892,
    0.062187569346966773,
    0.061651702900708892,
    0.060065244665458317,
    0.057490467586282790,
    0.054027400530101778,
    0.049808446806253924,
    0.044991345774770056,
    0.039750929341535547,
    0.034270195484424087,
    0.028731253784668880,
    0.023306691490530721,
    0.018151864198893832,
    0.013398536981689010,
    0.009150196390057992,
    0.005479229838401825,
    0.002426036182199808,
    0.000000000000000001,
    -0.001817858108731060,
    -0.003070843302818177,
    -0.003821523227979228,
    -0.004146047369803361,
    -0.004128266718722614,
    -0.003854054066021719,
    -0.003406177517088067,
    -0.002860010226536093,
    -0.002280274180310770,
    -0.001718922780864195,
    -0.001214173888151645,
    -0.000790619167168023,
    -0.000460263323995915,
    -0.000224292751269908,
};


static double buffer_I[FIR_FILTER_BUFFER_SIZE];
static double buffer_Q[FIR_FILTER_BUFFER_SIZE];
static int buffer_ptr;
static int decimation_count;



void init_fir_filter3()
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



void sample_in_3(double sample_I,double sample_Q)
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
		sample_in_quad_mod(conv_sum_I,conv_sum_Q);
      decimation_count = 0;
   }

   buffer_ptr++;
   buffer_ptr %= FIR_FILTER_BUFFER_SIZE;
}


