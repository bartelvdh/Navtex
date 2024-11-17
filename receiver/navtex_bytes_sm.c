#include <stdio.h>
#include <string.h>
#include <regex.h>
#include "navtex_bytes_sm.h"
#include "message_store.h"

static const unsigned char code_to_ltrs[128] = {
	//0   1    2    3    4    5    6    7    8    9    a    b    c    d    e    f
	'_', '_', '_', '_', '_', '_', '_', 'p', '_', '_', '_', 'J', '_', 'W', 'A', '_', // 0
	'_', '_', '_', 'F', '_', 'Y', 'S', '_', '_', '-', 'D', '_', 'Z', '_', '_', '_', // 1
	'_', '_', '_', 'C', '_', 'P', 'I', '_', '_', 'G', 'R', '_', 'L', '_', '_', '_', // 2
	'_', 'M', 'N', '_', 'H', '_', '_', '_', 'O', '_', '_', '_', '_', '_', '_', '_', // 3
	'_', '_', '_', 'K', '_', 'Q', 'U', '_', '_', 'f', 'E', '_', 'q', '_', '_', '_', // 4
	'_', 'X', 'l', '_', '_', '_', '_', '_', 'B', '_', '_', '_', ' ', '_', '_', '_', // 5
	'_', 'V', ' ', '_', 'n', '_', '_', '_', 'T', '_', '_', '_', '_', '_', '_', '_', // 6
	'r', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_' // 7
};

static const unsigned char code_to_figs[128] = {
	//0   1    2    3    4    5    6    7    8    9    a    b    c    d    e    f
	'_', '_', '_', '_', '_', '_', '_', 'p', '_', '_', '_', 'b', '_', '2', '-', '_', // 0
	'_', '_', '_', '*', '_', '6', '\'','_', '_', '-', '%', '_', '+', ' ', '_', '_', // 1
	'_', '_', '_', ':', '_', '0', '8', '_', '_', '*', '4', '_', ')', '_', '_', '_', // 2
	'_', '.', ',', '_', '*', '_', '_', '_', '9', '_', '_', '_', '_', '_', '_', '_', // 3
	'_', '_', '_', '(', '_', '1', '7', '_', '_', 'f', '3', '_', 'q', '_', '_', '_', // 4
	'_', '/', 'l', '_', '_', '_', '_', '_', '?', '_', '_', '_', ' ', '_', '_', '_', // 5
	'_', '=', ' ', '_', 'n', '_', '_', '_', '5', '_', '_', '_', '_', '_', '_', '_', // 6
	'r', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_' // 7
};

static const int ph1 = 0x07;
static const int ph2 = 0x4c;


extern int sample_nbr;
int prev_sample_nbr =0;

#define STAT_INIT 	0
#define STAT_1B   	1
#define STAT_2BS  	2
#define STAT_3BS  	3
#define STAT_4BS  	4
#define STAT_5BS  	5
#define STAT_6BS  	6
#define STAT_6BS1Y  		7
#define STAT_6BS2YS 		8
#define STAT_6BS3YS 		9
#define STAT_6BS4YS 		10
#define STAT_6BS4YS1B 		11
#define STAT_6BS4YS2BS 		12
#define STAT_6BS4YS2BS1Y 	13
#define STAT_6BS4YS2BS2YS 	14
#define STAT_6BS4YS2BS2YS1B 	15
#define STAT_6BS4YS2BS2YS2BS 	16
#define STAT_RECEIVING_BYTE 	17

#define S_BYTE_WAIT 	1
#define S_BYTE_RECEIVED_DX 	2
#define S_BYTE_RECEIVED_RX 	3


#define BYTE_MODE_LETTERS 	3
#define BYTE_MODE_FIGURES 	4

#define E_BUFFER_SIZE 12
#define ERROR_THRESHOLD 10

static char DX_byte;
char dx_buffer[3];
char error_buffer[E_BUFFER_SIZE];

static char temp_byte;
static char line_buffer[5000];  // 5000 because there cannot be more than 4000 characters transmitted in a 10 min slot
static char message_buffer[5000];  // 5000 because there cannot be more than 4000 characters transmitted in a 10 min slot
static char message_bbbb[10]; 
static int status;
static int byte_mode;
static int bits_received;
int dx_buf_ptr;
int dx_buf_filled;
int byte_status;
int error_count;
int error_buffer_ptr;
int error_buffer_filled;
int end_of_emission_counter;
int previous_DX_was_alpha;
int end_of_emission_detected;
int post_end_of_emission_counter;



void init_byte_state_machine()
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

void message_abort()
{
	printf("message abort\n%s\n",line_buffer);
   	line_buffer[0]= '\0';
   	message_buffer[0]= '\0';
}

void message_byte_out(unsigned char byte_in)
{
   regex_t     regex_som;
   regex_t     regex_eom;
   regmatch_t  pmatch[5];

	//printf("new byte  s:%d d(%d) o(%d)\n",sample_nbr,sample_nbr - prev_sample_nbr, sample_nbr % 36288 );
	prev_sample_nbr = sample_nbr;
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
				add_message(message_bbbb,message_buffer);
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




void receive_rxdx_byte(unsigned char byte_received)
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
				init_byte_state_machine();
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
		   
	            //printf("s_nbr: %d (%d)       RX char received: %c (DX = %c) RX-b = %d DX-b = %d", sample_nbr, sample_nbr - prev_sample_nbr, code_to_ltrs[(unsigned char) byte_received], code_to_ltrs[(unsigned char) DX_byte], (int)byte_received, (int)DX_byte );

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
		init_byte_state_machine();
	}
}



void receive_bit(char bit_received)
{
        //if (sample_nbr > 4709952)
	//{
	// printf("received %c\n",bit_received);
	//}
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
			//printf("sample# : %d\n",sample_nbr);
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
