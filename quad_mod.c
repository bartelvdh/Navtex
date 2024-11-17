#include <math.h>
#include <stdio.h>
#include "quad_mod.h"
#include "bit_sync.h"
#include "bit_det.h"


double prevI;
double prevQ;
extern int sample_nbr;
//WavFile *dfp;


unsigned int dec_wav_sample_rate = 900;


void QPrepWav()
{
/*
    dfp = wav_open("decimated.wav", WAV_OPEN_WRITE);
    wav_set_format(dfp, WAV_FORMAT_PCM);
    wav_set_num_channels(dfp, 2);
    wav_set_sample_rate(dfp, dec_wav_sample_rate);
    wav_set_sample_size(dfp, sizeof(short));
*/
}

void init_quad_mod()
{
	prevI=0.0;
	prevQ=0.0;
//	QPrepWav();
}




void end_quad_mod()
{
 //   wav_close(dfp);
}



void sample_in_quad_mod(double sampleI,double sampleQ)
{
  double prodReal;
  double prodImg; 
  double result;
//  short sample_store[2];

	/* from the GNU-RADIO quadrature demodulator:
	Mathematically, this block calculates the product of the one-sample delayed-&-conjugated input and the undelayed signal, and then calculates the argument (a.k.a. angle, in radians) of the resulting complex number
	*/

//        printf("I:%.6f; Q:%.6f\n", sampleI,sampleQ);
//        sample_store[0]= (short) sampleI*1000;
//        sample_store[1]= (short) sampleQ*1000;
//        wav_write(dfp, (void*)(sample_store),1);	



	  /*
	Let z1 = p + iq and z2 = r + is be two complex numbers (p, q, r and s are real),
	 then their product is defined as z1 * z2 = (pr - qs) + i(ps + qr). 

	 Let z1 be the new sample and z2 the previous sample conjugated
	 z2 = r -is  so Z1*Z2 = pr+qs  + (qr-ps) i;     p=sampleI; q=sampleQ; r=prevI; s=prevQ;

	  e.g. sample = (1,1) prev = (1,-1); argument should be 90degrees or pi/2 radians
	  1*1+(1*(-1)) = 0; 1*1 - (1*(-1)) = 2  => 90 degrees
	*/


	prodReal = sampleI*prevI + sampleQ*prevQ;
	prodImg  = sampleQ*prevI - sampleI*prevQ;


	//if((prodReal<1) || (prodImg<1))
	//{
	 //  result = 0;
	//}
	//else 
	{
	   result = atan2(prodImg,prodReal);
	   //result = atan2(prodImg,prodReal)* sqrt(sampleI*sampleI + sampleQ*sampleQ)*sqrt(prevI*prevI+prevI*prevI);
	}
	//return result;
        // 	printf("demodulated symbol input/sample#: %lf : %d \n",result,sample_nbr);
        // 	printf("        input samples#: %lf:%lf  (%lf:%lf)\n",sampleI,sampleQ,prevI,prevQ);
        //printf("dsi: %lf\n",result);
        //printf("sample# : %d\n",sample_nbr);

	prevI=sampleI;
	prevQ=sampleQ;

	//feed bit sync module
	//printf("angle %f.1 \n",result*180/3.1415);
        bs_decoded_sample_in(result);
        bd_decoded_sample_in(result,sampleI,sampleQ,prevI,prevQ);


}


