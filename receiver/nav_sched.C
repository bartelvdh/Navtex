
#include <stdio.h>
#include "fir2cpp.h"
#include "fir3cpp.h"
#include "decoder.h"
#include "nav_b_sm.h"
#include "nav_sched.h"


static byte_state_machine state_machine_518(518);
static byte_state_machine state_machine_490(490);

static decoder decoder_518(&state_machine_518);
static decoder decoder_490(&state_machine_490);
static fir_filter3 fir_filter_518(&decoder_518);
static fir_filter3 fir_filter_490(&decoder_490);


extern "C" void init_fir2_wrapper()
{
	init_fir_filter2(&fir_filter_518,&fir_filter_490);
}


