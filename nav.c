
#include <stdio.h>
#include "wav.h"
#include "fir1.h"
#include "fir2.h"
#include "fir3.h"
#include "quad_mod.h"
#include "bit_sync.h"
#include "bit_det.h"
#include "navtex_bytes_sm.h"

WavFile *fp;

int sample_nbr;


int main(int argc, char *argv[])
{
	short buffer[2];  // 2 = I and Q sample, each having 2 bytes (short)



	init_fir_filter1();
	init_fir_filter2();
	init_fir_filter3();
	init_quad_mod();
	init_bit_sync();
	init_bit_det();
	init_byte_state_machine();

	sample_nbr=1;

	fp = wav_open("/home/pi/Dev/out.wav", WAV_OPEN_READ);
        while ( wav_read(fp, buffer,1) != 0)
	{
		sample_in_1((double)buffer[0],(double)buffer[1]);
		sample_nbr++;
	}
	wav_close(fp);
}

