#include "nav_b_sm.h"
#include "message_store.h"

byte_state_machine::byte_state_machine(unsigned int frequency)
{
   freq = frequency;
   init();
}



void byte_state_machine::init()
{
   status = STAT_INIT;
   byte_status = S_BYTE_WAIT;
   bits_received = 0;
   dx_buf_ptr = 0;
   dx_buf_filled = 0;
   byte_mode = BYTE_MODE_LETTERS;
   error_count=0;
   error_buffer_ptr=0;
   error_buffer_filled=0;
   end_of_emission_counter=0;
   previous_DX_was_alpha=0;
   end_of_emission_detected=0;
   line_buffer[0]= '\0';
   message_buffer[0]= '\0';
   message_bbbb[0]= '\0';

}

void byte_state_machine::message_abort()
{
	printf("message abort\n%s\n",line_buffer);
   	line_buffer[0]= '\0';
   	message_buffer[0]= '\0';
}

void byte_state_machine::message_byte_out(unsigned char byte_in)
{
   regex_t     regex_som;
   regex_t     regex_eom;
   regmatch_t  pmatch[5];

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
	//	printf("\n");
                regcomp(&regex_som,"ZCZC +([A-X][ABCDEFGHIJKLTVWXYZ])([0-9][0-9])",REG_EXTENDED);
                regcomp(&regex_eom,"NNNN.*",REG_EXTENDED);


		if (regexec(&regex_som, line_buffer, 3, pmatch, 0) == 0)  // match with ZCZC 
		{
			strcpy(message_buffer,line_buffer);
			strcat(message_buffer,"\n");
			strncat(message_bbbb,line_buffer+pmatch[1].rm_so, pmatch[1].rm_eo - pmatch[1].rm_so);
			strncat(message_bbbb,line_buffer+pmatch[2].rm_so, pmatch[2].rm_eo - pmatch[2].rm_so);
			message_bbbb[4]='\0';
			printf("============START OF MESSAGE============ \n");

		}
		else
		{
			strcat(message_buffer,line_buffer);
			strcat(message_buffer,"\n");
			if (regexec(&regex_eom, line_buffer, 1, pmatch, 0) == 0)  //match with NNNN
			{
				add_message(message_bbbb,message_buffer,freq);
				message_buffer[0]= '\0';
				message_bbbb[0]= '\0';
				printf("============ END OF MESSAGE ============\n");
			}
		}
		printf("%s\n",line_buffer);
   		line_buffer[0]= '\0';
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
 		if(byte_mode == BYTE_MODE_LETTERS)
		{
			strncat(line_buffer, (char *)&code_to_ltrs[byte_in], 1);
		}
		else
		{
			strncat(line_buffer, (char *)&code_to_figs[byte_in], 1);
		}
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

		if (end_of_emission_detected)
		{
			post_end_of_emission_counter++;
			if(post_end_of_emission_counter == 2)
			{
	       			//printf("\nstopping reception\n");
				init();
				break;
			}
		}
		if (byte_received == ph1)
		{
	       		printf("\n alpha received in DX position\n");
			if(previous_DX_was_alpha)
			{
				end_of_emission_counter++;
				if(end_of_emission_counter == 1)
				{
	       				printf("\nend of emission detected\n");
					end_of_emission_detected=1;
					post_end_of_emission_counter=0;
				}
			}
			previous_DX_was_alpha = 1;
		}
		else
		{
			previous_DX_was_alpha = 0;
		}
		byte_status = S_BYTE_RECEIVED_DX;
  	    

	break;

     case S_BYTE_RECEIVED_DX:
	    //printf("        RX char received: %c\n", code_to_ltrs[(unsigned char) byte_received] );
	    if (dx_buf_filled)
	    {
		    DX_byte = dx_buffer[ (dx_buf_ptr) % 3];
		   

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
		init();
	}
}



void byte_state_machine::receive_bit(char bit_received)
{
  switch(status){
	case STAT_RECEIVING_BYTE:	
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

	break;



	case STAT_INIT:	
		if(bit_received == 'B') status = STAT_1B;
	break;


	case STAT_1B:	
		if(bit_received == 'B') 
		{
			status = STAT_2BS;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_2BS:	
		if(bit_received == 'B') 
		{
			status = STAT_3BS;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_3BS:	
		if(bit_received == 'B') 
		{
			status = STAT_4BS;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_4BS:	
		if(bit_received == 'B') 
		{
			status = STAT_5BS;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_5BS:	
		if(bit_received == 'B') 
		{
			status = STAT_6BS;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_6BS:	
		if(bit_received == 'Y') 
		{
			status = STAT_6BS1Y;
		}
		else
		{
			status = STAT_6BS;
		}
	break;

	case STAT_6BS1Y:	
		if(bit_received == 'Y') 
		{
			status = STAT_6BS2YS;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_6BS2YS:	
		if(bit_received == 'Y') 
		{
			status = STAT_6BS3YS;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_6BS3YS:	
                if(bit_received == 'Y')
                {
                        status = STAT_6BS4YS;
                }
		else
		{
			status = STAT_INIT;
		}
	break;


        case STAT_6BS4YS:
                if(bit_received == 'B')
                {
                        status = STAT_6BS4YS1B;
                }
                else
                {
                        status = STAT_INIT;
                }
        break;


	case STAT_6BS4YS1B:	
		if(bit_received == 'B') 
		{
			status = STAT_6BS4YS2BS;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_6BS4YS2BS:	
		if(bit_received == 'Y') 
		{
			status = STAT_6BS4YS2BS1Y;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_6BS4YS2BS1Y:	
		if(bit_received == 'Y') 
		{
			status = STAT_6BS4YS2BS2YS;
		}
		else
		{
			status = STAT_INIT;
		}
	break;

	case STAT_6BS4YS2BS2YS:	
		if(bit_received == 'B') 
		{
			status = STAT_6BS4YS2BS2YS1B;
		}
		else
		{
			status = STAT_INIT;
		}
	break;


	case STAT_6BS4YS2BS2YS1B:	
		if(bit_received == 'B') 
		{
			status = STAT_RECEIVING_BYTE;
			bits_received = 0;
			temp_byte = 0x00;
			printf("phasing detected\n");
		}
		else
		{
			status = STAT_INIT;
		}
	break;

  }
}
