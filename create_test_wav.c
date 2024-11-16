#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <sys/wait.h>
#include "wav.h"

#define WAV_SAMPLE_RATE  259200
#define SECONDS  10
#define FREQ_STEP 300
// -(259200 - 900)
#define FREQ_START (55800) 
#define FREQ_STOP   73800  
#define BASE_ARRAY_SIZE (WAV_SAMPLE_RATE/FREQ_STEP)
  //every sine wave with a freq that is a multiple of FREQ_STEP can be represented as a repetition of BASE_ARRAY_SIZE samples
 


WavFile *fp;



void PrepWav()
{
    fp = wav_open("test.wav", WAV_OPEN_WRITE);
    wav_set_format(fp, WAV_FORMAT_PCM);
    wav_set_num_channels(fp, 2);
    wav_set_sample_rate(fp, WAV_SAMPLE_RATE);
    wav_set_sample_size(fp, sizeof(short));
}

void EndWav()
{
    wav_close(fp);	
}





int main(int argc, char *argv[])
{
  //double f1 = 64800.0;
  //double f = 0.0;

  //unsigned long int sample_nbr=0;
  double angle = 0.0;
  short sample_buffer[2];
  short sample_smart_buffer[BASE_ARRAY_SIZE][2];
  signed long int freq;
  unsigned int i;
  unsigned int index;


PrepWav();
  for (freq = FREQ_START; freq <= FREQ_STOP; freq += FREQ_STEP)
  {
	
    printf("freq: %d \n",(int)freq);
    for(i=0;i<BASE_ARRAY_SIZE;i++)
    {
 	angle = 2 * 3.14159265359 * ((double)freq) * ((double) i)  / (WAV_SAMPLE_RATE) ; 
 	sample_smart_buffer[i][0] = (short) (cos(angle)*1000);
 	sample_smart_buffer[i][1] = (short) (sin(angle)*1000);
  	printf("sample: %f %d %d \n",angle,sample_smart_buffer[i][0],sample_smart_buffer[i][1]);
    }
    for(i=0;i<WAV_SAMPLE_RATE;i++)  //produce 1 seconds of sine wave
    {
	index = i % BASE_ARRAY_SIZE;
	sample_buffer[0] = (short)sample_smart_buffer[index][0];
	sample_buffer[1] = (short)sample_smart_buffer[index][1];
  	//printf("wav_sample: %d %d %d %d \n",i,index,sample_buffer[0],sample_buffer[1]);
  	wav_write(fp, (void*)(sample_buffer), 1);
    }
  }
			


/*
for(sample_nbr=0;sample_nbr < (WAV_SAMPLE_RATE*SECONDS); sample_nbr ++)
{
 f = f1+(((double)sample_nbr)/ SECONDS /WAV_SAMPLE_RATE * 5000);
 angle += 2 * 3.14159265359 * f  /WAV_SAMPLE_RATE ; 
 if (angle > 2*3.14159265359) 
 {
	angle -= 2*3.14159265359 ;
 }
 if (angle < 2*3.14159265359) 
 {
	angle += 2*3.14159265359 ;
 }
 sample_buffer[0] = (short) (cos(angle)*1000);
 sample_buffer[1] = (short) (sin(angle)*1000);
  //printf("sample: %f %d %d \n",angle,sample_buffer[0],sample_buffer[1]);
 wav_write(fp, (void*)(sample_buffer), 1);
}

*/


EndWav();

return 0;
}

