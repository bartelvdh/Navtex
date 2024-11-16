
#include <stdio.h>
#include "fir1.h"
#include "fir2.h"
#include "fir3.h"
#include "quad_mod.h"
#include "bit_sync.h"
#include "bit_det.h"
#include "navtex_bytes_sm.h"


int sample_nbr;



void init_dsp()
{
	init_fir_filter1();
	init_fir_filter2();
	init_fir_filter3();
	init_quad_mod();
	init_bit_sync();
	init_bit_det();
	init_byte_state_machine();
}


