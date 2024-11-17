#include "navtex_bytes_sm.h"
#include <stdio.h>
#include <math.h>


#define SAMPLES_PER_BIT 9
#define SAMPLES_TO_BURN 2
#define SAMPLES_TO_USE 5
#define SAMPLE_RATE 900
#define NAVTEX_FSK_SHIFT 85

 

// note: SAMPLES_PER_BIT = SAMPLES_TO_USE + 2 * SAMPLES_TO_BURN


extern int sample_nbr;

static int seq_nbr;
static int status;
static double bitsum;
static float Brotated_samplesumR;
static float Brotated_samplesumI;
static float Yrotated_samplesumR;
static float Yrotated_samplesumI;
static int samplecount;
static int bit_sync_offset;
static int next_bit_sync_offset;
static int burn_count;
float bit_filterR[SAMPLES_TO_USE];
float bit_filterI[SAMPLES_TO_USE];


#define STATUS_INIT 0
#define STATUS_SYNCED_WAIT 1
#define STATUS_SYNCED_BIT_START 2
#define STATUS_SYNCED_RECEIVING 3


void init_bit_det()
{
   int i;
   float angle;

   seq_nbr = 0; 
   status = STATUS_INIT;
   bitsum = (double)0.0;
   Brotated_samplesumR=(float)0.0;
   Brotated_samplesumI=(float)0.0;
   Yrotated_samplesumR=(float)0.0;
   Yrotated_samplesumI=(float)0.0;
  
   for(i=0;i<SAMPLES_TO_USE;i++) 
   {
	angle = (i*2*3.1415*NAVTEX_FSK_SHIFT) / SAMPLE_RATE ;
	bit_filterR[i]=cos(angle);
	bit_filterI[i]=sin(angle);
   }   
}


void bd_in_bit_sync(int offset)
{
   if(status == STATUS_INIT)
   {
      status = STATUS_SYNCED_WAIT;
      bit_sync_offset = offset;
   }
   next_bit_sync_offset = offset;
}


void bd_decoded_sample_in(double ds,double sampleR,double sampleI,double prevR,double prevI)
{
   seq_nbr ++;
   float Brot;
   float Yrot;

   if(status == STATUS_INIT)
   {
      return;
   }
   if(status == STATUS_SYNCED_WAIT)
   {
      if( (seq_nbr % SAMPLES_PER_BIT)  == bit_sync_offset)
      {
         status = STATUS_SYNCED_BIT_START;
	  burn_count = 0;
      }
   }
   if(status == STATUS_SYNCED_BIT_START)
   {
      if(burn_count == SAMPLES_TO_BURN)
      {
         status = STATUS_SYNCED_RECEIVING;
	 samplecount = 0;
	 bitsum = 0.0;
	   Brotated_samplesumR=(float)0.0;
	   Brotated_samplesumI=(float)0.0;
	   Yrotated_samplesumR=(float)0.0;
	   Yrotated_samplesumI=(float)0.0;
	   //printf("start using samples for bit sn = %d\n",sample_nbr); 
 
         return;
      }
      else
      {
	burn_count++;
        return;
      }
   }
   if(status == STATUS_SYNCED_RECEIVING)
   {
     // X1= A+iB; X2 = C+iD  X1*X2 = (AC-BD) +i(AD+BC)
     bitsum += ds;
     Yrotated_samplesumR+=(float) sampleR*bit_filterR[samplecount]-sampleI*bit_filterI[samplecount];
     Yrotated_samplesumI+=(float) sampleR*bit_filterI[samplecount]+sampleI*bit_filterR[samplecount];;
     Brotated_samplesumR+=(float) sampleR*bit_filterR[samplecount]+sampleI*bit_filterI[samplecount];
     Brotated_samplesumI+=(float) -sampleR*bit_filterI[samplecount]+sampleI*bit_filterR[samplecount];;

     samplecount++;
     //printf("using %lf\n",ds); 
     if(samplecount == SAMPLES_TO_USE)
     {
	Brot = Brotated_samplesumR*Brotated_samplesumR+Brotated_samplesumI*Brotated_samplesumI;
	Yrot = Yrotated_samplesumR*Yrotated_samplesumR+Yrotated_samplesumI*Yrotated_samplesumI;
	//if(bitsum > 0.0)
	if(Brot>Yrot)
        {
	   receive_bit('B');
        }
	else
        {
	   receive_bit('Y');
	//   printf("Bit Y,  bitsum=%f,  BrssR %f, BrssI %f, YrssR %f, YrssI %f \n",bitsum,Brotated_samplesumR,Brotated_samplesumI,Yrotated_samplesumR,Yrotated_samplesumI);
        }
        status = STATUS_SYNCED_WAIT;
 	   //printf("stop using samples for bit sn = %d bso=%d\n",sample_nbr,next_bit_sync_offset); 
   	bit_sync_offset = next_bit_sync_offset ;
     }
   }
} 


