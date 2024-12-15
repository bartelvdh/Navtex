#ifndef DECODER_H
#define DECODER_H


#include <stdio.h>
#include <math.h>
#include "nav_b_sm.h"


#define SAMPLES_TO_BURN 2
#define SAMPLES_TO_USE 5
#define SAMPLE_RATE 900
#define NAVTEX_FSK_SHIFT 85


#define STATUS_INIT 0
#define STATUS_SYNCED_WAIT 1
#define STATUS_SYNCED_BIT_START 2
#define STATUS_SYNCED_RECEIVING 3

#define SAMPLES_PER_BIT 9
// was 35
#define CORRELATION_BUF_NUMBITS 63
#define CORRELATION_BUF_SAMPLE_SIZE  (CORRELATION_BUF_NUMBITS*SAMPLES_PER_BIT)

class decoder  {


private:

int bs_seq_nbr;
int status;
double bitsum;
float Brotated_samplesumR;
float Brotated_samplesumI;
float Yrotated_samplesumR;
float Yrotated_samplesumI;
int samplecount;
int bit_sync_offset;
int next_bit_sync_offset;
int burn_count;
float bit_filterR[SAMPLES_TO_USE];
float bit_filterI[SAMPLES_TO_USE];

double prevI;
double prevQ;


double delta_angle_buffer[SAMPLES_PER_BIT];
double correlation_buffer[CORRELATION_BUF_SAMPLE_SIZE];
double correlation_sumarray[SAMPLES_PER_BIT];

int dab_index;
int cb_index;
int csa_index;
int dab_primed;
int cb_primed;
int csa_primed;
int prev_offset;
int bd_seq_nbr;

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


void bd_decoded_sample_in(double ds,double sampleR,double sampleI);
void bd_in_bit_sync(int offset);
void bs_decoded_sample_in(double ds);

byte_state_machine * output_bsm;


public:

decoder(byte_state_machine * bsm);
void sample_in(double sampleI,double sampleQ);


};

#endif
