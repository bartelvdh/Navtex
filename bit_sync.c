#include <stdio.h>
#include "bit_det.h"
#include <math.h>

//#define BIT_SYNC_BUF_SIZE 9*7*2
#define SAMPLES_PER_BIT 9
// was 35
#define CORRELATION_BUF_NUMBITS 56   
#define CORRELATION_BUF_SAMPLE_SIZE  (CORRELATION_BUF_NUMBITS*SAMPLES_PER_BIT)
#define SPB2 4   //samples per bit -1  and divided by 2
// 9 is samples per bit, 7 is bits per symbol, 2 is #of symbols to consider for bit syncing
//static int bit_sync_buffer[BIT_SYNC_BUF_SIZE];
//static int bit_sync_sum[SAMPLES_PER_BIT];

#define STATUS_SAMPLING_DATA
#define STATUS_WAITING

extern int sample_nbr;

//static int global_seq_nbr;
//static double prev_ds; 

double delta_angle_buffer[SAMPLES_PER_BIT];
double correlation_buffer[CORRELATION_BUF_SAMPLE_SIZE];
double correlation_sumarray[SAMPLES_PER_BIT];

static int dab_index;
static int cb_index;
static int csa_index;
static int dab_primed;
static int cb_primed;
static int csa_primed;
static int prev_offset;
static int seq_nbr;

int correlation_mask[SAMPLES_PER_BIT] = {
0,
1,
1,
1,
0,
-1,
-1,
-1,
0
};



void init_bit_sync()
{
   //global_seq_nbr=0;
   //prev_ds = 1.0;
   	seq_nbr = 0; 
        prev_offset = -1;
	dab_index=0;
	cb_index=0;
	csa_index=0;
	dab_primed=0;
	cb_primed=0;
	csa_primed=0;

}


void bs_decoded_sample_in(double ds)
{
	int i;

	//keep the 'SAMPLES_PER_BIT' last samples (9)
	delta_angle_buffer[dab_index]=ds;	
	dab_index++;
	if (dab_index == SAMPLES_PER_BIT)
	{
		dab_index = 0;
		dab_primed = 1;
	}
	
	//correlate the 'SAMPLES_PER_BIT' last samples with the correlation mask (correlate === make sumproduct)
	//and store the absolute value of the this correlation result in the correlation buffer
	if (dab_primed) 
	{
		double temp;
		int dab_j;

		temp = 0.0;
		dab_j = dab_index;
		for (i=0;i<SAMPLES_PER_BIT;i++)
 		{
			temp+=correlation_mask[i]*delta_angle_buffer[dab_j];
			dab_j++;
			dab_j %= SAMPLES_PER_BIT;
		}
		correlation_buffer[cb_index]=fabs(temp);
		cb_index++;
		if(cb_index == CORRELATION_BUF_SAMPLE_SIZE)
		{
			cb_index = 0;
			cb_primed = 1;
		}
	}
 	
 	//calculate the sums of all correlations related to the same offset
	//in fact only one of 9 needs to be recomputed...
	if(cb_primed) 
	{
		double temp;
		temp=0.0;

		for (i=csa_index;i< CORRELATION_BUF_SAMPLE_SIZE;i+=SAMPLES_PER_BIT)
		{
			temp += correlation_buffer[i];
		}
		correlation_sumarray[csa_index] = temp;
		csa_index++;
		if(csa_index == SAMPLES_PER_BIT)
		{
			csa_index = 0;
			csa_primed = 1;
		}
	}
	//find highest value in correlation sum array
	//do it only 1 out of 9 times
	if(csa_primed) 
	{
		if((seq_nbr % 9) == 0)
		{
			double temp_max;
			int max_index;

			temp_max = -1.0;
			for (i=0;i<SAMPLES_PER_BIT;i++) 
			{
				if(correlation_sumarray[i] > temp_max)
				{
					temp_max = correlation_sumarray[i];
					max_index = i;
				}
			}
//			printf("max_index = %d\n",max_index);
			if (prev_offset == -1  || max_index == prev_offset)
			{
			}
			else 
			{
//					printf("max_index2 = %d\n",max_index);
					if ( max_index > prev_offset )
					{
						if ( max_index - prev_offset > 4 )
						{
							max_index = (prev_offset -1 +9) % SAMPLES_PER_BIT;
						}
						else
						{	
							max_index = (prev_offset +1) % SAMPLES_PER_BIT;
						}
					} 
					else
					{
						if ( prev_offset - max_index > 4 )
						{
							max_index = (prev_offset +1) % SAMPLES_PER_BIT;
						}
						else
						{	
							max_index = (prev_offset -1 +9 ) % SAMPLES_PER_BIT;
						}
					}
//					printf("max_index3 = %d\n",max_index);
			}
			prev_offset = max_index;
//					printf("max_index3 = %d\n",max_index);
           		bd_in_bit_sync( (max_index+5) % SAMPLES_PER_BIT);  //shift since correlation mask brings 4.5 samples shift
			if (sample_nbr > 4709952)
			{
//			printf("NEW bit_sync_offset: %d \n",max_index);
			}
		}
		
	seq_nbr = (seq_nbr+1) % 9;	
	}

} 
