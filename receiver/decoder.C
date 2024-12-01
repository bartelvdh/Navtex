#include "decoder.h"




decoder::decoder(byte_state_machine * bsm)
{
   int i;
   float angle;

   prevI=0.0;
   prevQ=0.0;

   bs_seq_nbr = 0;
   bd_seq_nbr = 0;
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

        prev_offset = -1;
        dab_index=0;
        cb_index=0;
        csa_index=0;
        dab_primed=0;
        cb_primed=0;
        csa_primed=0;

	output_bsm = bsm;
}


void decoder::sample_in(double sampleI,double sampleQ)
{
  double prodReal;
  double prodImg;
  double result;

        prodReal = sampleI*prevI + sampleQ*prevQ;
        prodImg  = sampleQ*prevI - sampleI*prevQ;

        
        result = atan2(prodImg,prodReal);

        prevI=sampleI;
        prevQ=sampleQ;

        bs_decoded_sample_in(result);
        bd_decoded_sample_in(result,sampleI,sampleQ);
}


void decoder::bd_in_bit_sync(int offset)
{
   if(status == STATUS_INIT)
   {
      status = STATUS_SYNCED_WAIT;
      bit_sync_offset = offset;
   }
   next_bit_sync_offset = offset;
}


void decoder::bd_decoded_sample_in(double ds,double sampleR,double sampleI)
{
   bd_seq_nbr ++;
   float Brot;
   float Yrot;

   if(status == STATUS_INIT)
   {
      return;
   }
   if(status == STATUS_SYNCED_WAIT)
   {
      if( (bd_seq_nbr % SAMPLES_PER_BIT)  == bit_sync_offset)
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
     Yrotated_samplesumR+=(float) sampleR*bit_filterR[samplecount]-sampleI*bit_filterI[samplecount];
     Yrotated_samplesumI+=(float) sampleR*bit_filterI[samplecount]+sampleI*bit_filterR[samplecount];;
     Brotated_samplesumR+=(float) sampleR*bit_filterR[samplecount]+sampleI*bit_filterI[samplecount];
     Brotated_samplesumI+=(float) -sampleR*bit_filterI[samplecount]+sampleI*bit_filterR[samplecount];;

     samplecount++;
     if(samplecount == SAMPLES_TO_USE)
     {
        Brot = Brotated_samplesumR*Brotated_samplesumR+Brotated_samplesumI*Brotated_samplesumI;
        Yrot = Yrotated_samplesumR*Yrotated_samplesumR+Yrotated_samplesumI*Yrotated_samplesumI;
        if(Brot>Yrot)
        {
           output_bsm->receive_bit('B');
        }
        else
        {
           output_bsm->receive_bit('Y');
        }
        status = STATUS_SYNCED_WAIT;
        bit_sync_offset = next_bit_sync_offset ;
     }
   }
}




void decoder::bs_decoded_sample_in(double ds)
{
        int i;

        //keep the 'SAMPLES_PER_BIT' last samples (10)
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
                if((bs_seq_nbr % SAMPLES_PER_BIT) == 0)
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
//                      printf("max_index = %d\n",max_index);
                        if (prev_offset == -1  || max_index == prev_offset)
                        {
                        }
                        else
                        {
//                                      printf("max_index2 = %d\n",max_index);
                                        if ( max_index > prev_offset )
                                        {
                                                if ( max_index - prev_offset > 4 )
                                                {
                                                        max_index = (prev_offset -1 +SAMPLES_PER_BIT) % SAMPLES_PER_BIT;
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
                                                        max_index = (prev_offset -1 +SAMPLES_PER_BIT ) % SAMPLES_PER_BIT;
                                                }
                                        }
//                                      printf("max_index3 = %d\n",max_index);
                        }
                        prev_offset = max_index;
//                                      printf("max_index3 = %d\n",max_index);
                        bd_in_bit_sync( (max_index+5) % SAMPLES_PER_BIT);  //shift since correlation mask brings 4.5 samples shift
                }

        bs_seq_nbr = (bs_seq_nbr+1) % SAMPLES_PER_BIT;
        }

}


