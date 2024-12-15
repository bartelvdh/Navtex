#include "nav_b_sm.h"
#include <stdbool.h>

extern "C" int add_message(char *bbbb,char *message, int freq);



byte_state_machine::byte_state_machine(unsigned int frequency)
{
   freq = frequency;
   init();
}



void byte_state_machine::init()
{
   status = STAT_INIT;
   byte_status = S_BYTE_WAIT;
   byte_mode = BYTE_MODE_LETTERS;

   bits_received = 0;
   dx_buf_ptr = 0;
   dx_buf_filled = false;

   error_count=0;
   error_buffer_ptr=0;
   error_buffer_filled=false;

   end_of_emission_counter=0;
   previous_DX_was_alpha=false;

   line_buffer[0]=    '\0';
   message_buffer[0]= '\0';
   message_bbbb[0]=   '\0';

   phase_det_disable_timer=0;

   byte_reception_enabled=false;
   message_reception_ongoing=false;

}

void byte_state_machine::message_abort()
{
	printf("message abort\n");
   	if(message_reception_ongoing)
	{
		add_message(message_bbbb,message_buffer,freq);
	}
	init();
}



void byte_state_machine::message_line_out()
{
   regex_t     regex_som;
   regex_t     regex_eom;
   regmatch_t  pmatch[4];

		if(message_reception_ongoing)
		{
			strcat(message_buffer,line_buffer);
			strcat(message_buffer,"\n");
			printf("line added: %s\n",line_buffer);
		}

                regcomp(&regex_som,"(CZC|Z.ZC|ZC.C|ZCZ.) +([A-Z][A-Z])([0-9][0-9])",REG_EXTENDED); 	//Start of message
		if (regexec(&regex_som, line_buffer, 4, pmatch, 0) == 0)  // match with ZCZC 
		{
			strcpy(message_buffer,line_buffer);
			strcat(message_buffer,"\n");
			strncat(message_bbbb,line_buffer+pmatch[2].rm_so, pmatch[2].rm_eo - pmatch[2].rm_so);
			strncat(message_bbbb,line_buffer+pmatch[3].rm_so, pmatch[3].rm_eo - pmatch[3].rm_so);
			message_bbbb[4]='\0';
			printf("============START OF MESSAGE============ \n");
   			message_reception_ongoing=true;
		}
		else
		{
                	regcomp(&regex_eom,"NNN.*|N.NN.*|NN.N.*",REG_EXTENDED);					//End of message
			if (regexec(&regex_eom, line_buffer, 1, pmatch, 0) == 0)  //match with NNNN
			{
				if(message_reception_ongoing) 
				{
					add_message(message_bbbb,message_buffer,freq);
				}
				message_buffer[0]= '\0';
				message_bbbb[0]= '\0';
				printf("============ END OF MESSAGE ============\n");
   				message_reception_ongoing=false;
			}
		}

   		line_buffer[0]= '\0';
}


void byte_state_machine::message_byte_out(unsigned char byte_in)
{

	if (byte_in == '\0')
	{
	     // ERROR
	     printf("*");
	     strcat(line_buffer, "*");
	}
	else if (code_to_ltrs[byte_in] == 'l')
	{
   		byte_mode = BYTE_MODE_LETTERS;
	}
	else if (code_to_ltrs[byte_in] == 'f')
	{
   		byte_mode = BYTE_MODE_FIGURES;
	} 
	else if (code_to_ltrs[byte_in] == 'n')
	{
		message_line_out();
	} 
	else if (code_to_ltrs[byte_in] == 'r')
	{
	//	printf("\r");
	} 
	else if (code_to_ltrs[byte_in] == 'p')
	{
	} 
	else if (code_to_ltrs[byte_in] == 'q')
	{
	} 
	else
	{
		printf(".");
 		if(byte_mode == BYTE_MODE_LETTERS)
		{
			strncat(line_buffer, (char *)&code_to_ltrs[byte_in], 1);
		}
		else
		{
			strncat(line_buffer, (char *)&code_to_figs[byte_in], 1);
		}
		printf(";");
	}

}




void byte_state_machine::receive_rxdx_byte(unsigned char byte_received)
{

  switch(byte_status){
     case S_BYTE_WAIT:

	 if( byte_received == ph1 )
         {
            byte_status = S_BYTE_RECEIVED_RX;
	 }
	
	 if( byte_received == ph2 )
         {
            byte_status = S_BYTE_RECEIVED_DX;
	 }
	break;


     case S_BYTE_RECEIVED_RX:
	       	//printf("        DX char received: %c\n", code_to_ltrs[byte_received] );
		dx_buffer[dx_buf_ptr]=byte_received;
                dx_buf_ptr ++;	
		if (dx_buf_ptr == 3)
		{
			dx_buf_ptr = 0;
   			dx_buf_filled = 1;
		}

		// end_of_emission  detection

		if (byte_received == ph1)
		{
	       		printf("\n alpha received in DX position\n");
			if(previous_DX_was_alpha)
			{
				end_of_emission_counter++;
				if(end_of_emission_counter == 2)
				{
	       				printf("\nend of emission detected\n");
					printf("\nstopping reception\n");
					message_abort();
					break;
				}
			}
			previous_DX_was_alpha = true;
		}
		else
		{
			previous_DX_was_alpha = false;
		}
		byte_status = S_BYTE_RECEIVED_DX;
  	    

	break;

     case S_BYTE_RECEIVED_DX:
	    //printf("        RX char received: %c\n", code_to_ltrs[(unsigned char) byte_received] );
	    if (dx_buf_filled)
	    {
		    DX_byte = dx_buffer[ dx_buf_ptr ];
		   

		    if (code_to_ltrs[(unsigned char) byte_received] != '_' )
		    {
                       message_byte_out(byte_received);
		    }
		    else
                    {
		       if (code_to_ltrs[(unsigned char) DX_byte] != '_' )
             		{
                       		message_byte_out(DX_byte);
 			}
		       else 
                       	{
				// output error
                       		message_byte_out('\0');
			}
	  	    }
	    }
            byte_status = S_BYTE_RECEIVED_RX;
	break;
     }



	// count bytes containing error (sliding window)
	if(error_buffer_filled)
	{
		if(error_buffer[error_buffer_ptr] == '_')
		{
			error_count--;
		}
	}
	error_buffer[error_buffer_ptr]=code_to_ltrs[(unsigned char) byte_received];
	if(error_buffer[error_buffer_ptr] == '_')
	{
		error_count++;
	}

	error_buffer_ptr++;
	if(error_buffer_ptr == E_BUFFER_SIZE)
	{
		error_buffer_ptr = 0;
		error_buffer_filled = 1;
	}

        if(error_count > ERROR_THRESHOLD) 
	{
                message_byte_out('\0');
		printf("\n error th exceeded \n");
		message_abort();
	}
}



void byte_state_machine::receive_bit(char bit_received)
{

   if(byte_reception_enabled)
   {
                temp_byte <<= 1;
                if(bit_received == 'Y')
                {
                        temp_byte |= 0x01;
                }
                bits_received += 1;
                if(bits_received == 7)
                {
                        // full byte received, handle it here...
                        receive_rxdx_byte(temp_byte);
                        // get ready to receive the next byte
                        bits_received = 0;
                        temp_byte = 0x00;
                }

   } 

   if(phase_det_disable_timer != 0)
   {
      phase_det_disable_timer--; 
	if(phase_det_disable_timer ==0) 
	{
		printf("phase det disable timer expired\n");
	}
   }
   else 
   {



      switch(status){

	case STAT_INIT:	
		if(bit_received == 'B') status = STAT_B;
	break;


	case STAT_B:	
		if(bit_received == 'B') 
		{
			status = STAT_BB;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_BB:	
		if(bit_received == 'B') 
		{
			status = STAT_BBB;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_BBB:	
		if(bit_received == 'B') 
		{
			status = STAT_BBBB;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_BBBB:	
		if(bit_received == 'B') 
		{
			status = STAT_BBBBB;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_BBBBB:	
		if(bit_received == 'B') 
		{
			status = STAT_BBBBBB;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_BBBBBB:	
		if(bit_received == 'Y') 
		{
			status = STAT_BBBBBBY;
		}
		else
		{
			status = STAT_BBBBBB;
		}
	break;

	case STAT_BBBBBBY:	
		if(bit_received == 'Y') 
		{
			status = STAT_BBBBBBYY;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_BBBBBBYY:	
		if(bit_received == 'Y') 
		{
			status = STAT_BBBBBBYYY;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_BBBBBBYYY:	
                if(bit_received == 'Y')
                {
                        status = STAT_BBBBBBYYYY;
                }
		else
		{
			status = STAT_INIT;
		}
	break;


        case STAT_BBBBBBYYYY:
                if(bit_received == 'B')
                {
                        status = STAT_BBBBBBYYYYB;
                }
                else
                {
                        status = STAT_INIT;
                }
        break;


	case STAT_BBBBBBYYYYB:	
		if(bit_received == 'B') 
		{
			status = STAT_BBBBBBYYYYBB;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_BBBBBBYYYYBB:	
		if(bit_received == 'Y') 
		{
			status = STAT_BBBBBBYYYYBBY;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_BBBBBBYYYYBBY:	
		if(bit_received == 'Y') 
		{
			status = STAT_BBBBBBYYYYBBYY;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_BBBBBBYYYYBBYY:	
		if(bit_received == 'B') 
		{
			status = STAT_BBBBBBYYYYBBYYB;
		}
		else
		{
			status = STAT_INIT;
		}
	break;


	case STAT_BBBBBBYYYYBBYYB:	
		if(bit_received == 'B') 
		{
			status = STAT_BBBBBBYYYYBBYYBB;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_BBBBBBYYYYBBYYBB:	
		if(bit_received == 'B') 
		{
			status = STAT_BBBBBBYYYYBBYYBBB;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_BBBBBBYYYYBBYYBBB:	
		if(bit_received == 'B') 
		{
			status = STAT_BBBBBBYYYYBBYYBBBB;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_BBBBBBYYYYBBYYBBBB:	
		if(bit_received == 'B') 
		{
			status = STAT_BBBBBBYYYYBBYYBBBBB;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_BBBBBBYYYYBBYYBBBBB:	
		if(bit_received == 'B') 
		{
			status = STAT_BBBBBBYYYYBBYYBBBBBB;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_BBBBBBYYYYBBYYBBBBBB:	
		if(bit_received == 'Y') 
		{
			status = STAT_BBBBBBYYYYBBYYBBBBBBY;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_BBBBBBYYYYBBYYBBBBBBY:	
		if(bit_received == 'Y') 
		{
			status = STAT_BBBBBBYYYYBBYYBBBBBBYY;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_BBBBBBYYYYBBYYBBBBBBYY:	
		if(bit_received == 'Y') 
		{
			status = STAT_BBBBBBYYYYBBYYBBBBBBYYY;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_BBBBBBYYYYBBYYBBBBBBYYY:
		if(bit_received == 'Y') 
		{
			status = STAT_BBBBBBYYYYBBYYBBBBBBYYYY;
		}
		else
		{
			status = STAT_INIT;
		}
	break;
	
	case STAT_BBBBBBYYYYBBYYBBBBBBYYYY:
		if(bit_received == 'B') 
		{
			status = STAT_BBBBBBYYYYBBYYBBBBBBYYYYB;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_BBBBBBYYYYBBYYBBBBBBYYYYB:
		if(bit_received == 'B') 
		{
			status = STAT_BBBBBBYYYYBBYYBBBBBBYYYYBB;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_BBBBBBYYYYBBYYBBBBBBYYYYBB:
		if(bit_received == 'Y') 
		{
			status = STAT_BBBBBBYYYYBBYYBBBBBBYYYYBBY;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_BBBBBBYYYYBBYYBBBBBBYYYYBBY:
		if(bit_received == 'Y') 
		{
			status = STAT_BBBBBBYYYYBBYYBBBBBBYYYYBBYY;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_BBBBBBYYYYBBYYBBBBBBYYYYBBYY:
		if(bit_received == 'B') 
		{
			status = STAT_BBBBBBYYYYBBYYBBBBBBYYYYBBYYB;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_BBBBBBYYYYBBYYBBBBBBYYYYBBYYB:
		if(bit_received == 'B') 
		{
			byte_reception_enabled = true;
			bits_received = 0;
                        temp_byte = 0x00;
                        printf("phasing detected\n");
			phase_det_disable_timer = PHASE_DETECTION_DIS_TIMER_VALUE; 
		}
		status = STAT_INIT;
	break;


     }
   }
}
