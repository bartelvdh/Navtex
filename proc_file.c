#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fir1.h"
#include "nav_sched.h"
#include "message_store.h"
#include "wav.h"
#include "quad_mod.h"

///////////// NAVTEX/SITOR-B /////////////
#define D_SAMPLE_RATE 2073600.0
#define D_DECIMATION_FACTOR 8


extern int sample_nbr;


int wav_sample_rate = (int)(D_SAMPLE_RATE / D_DECIMATION_FACTOR);



WavFile* PrepWavRead()
{
    WavFile *fp;

    char filename[80];
    strcpy(filename,"NThEcaYatj.wav");
    //strcpy(filename,"test.wav");
    fp = wav_open(filename, WAV_OPEN_READ);
    //wav_set_format(fp, WAV_FORMAT_PCM);
    //wav_set_num_channels(fp, 2);
    //wav_set_sample_rate(fp, wav_sample_rate);
    //wav_set_sample_size(fp, sizeof(short));
    return fp;
}


void EndWav(WavFile* fp)
{
    wav_close(fp);
}



int main(int argc, char *argv[])
{
   WavFile *fp;
   short sample_buffer[2];
   unsigned long sample_nr; 
   int seconds;
   int minutes;


   init_dsp();
   sample_nr = 0;

   fp = PrepWavRead();

   while (wav_read(fp, (void*)(&sample_buffer), 1) == 1)
   {
      sample_nr++;
      sample_nbr++;
      	if((sample_nr % 2592000) == 0)
      	{
		seconds = sample_nr/259200;
		minutes = seconds/60;
		seconds %= 60;
		printf("time: %d:%d s:%d\n",minutes,seconds,sample_nbr);
	}
      sample_in_1((double) sample_buffer[0],(double) sample_buffer[1]);
   }
   end_quad_mod();   
   EndWav(fp);

   return 0;
}

