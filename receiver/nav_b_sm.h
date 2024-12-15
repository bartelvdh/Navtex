#ifndef NAV_B_SM_H
#define NAV_B_SM_H


#include <stdio.h>
#include <string.h>
#include <regex.h>


#define STAT_INIT       	0
#define STAT_B         		1
#define STAT_BB        		2
#define STAT_BBB        	3
#define STAT_BBBB        	4
#define STAT_BBBBB        	5
#define STAT_BBBBBB        	6
#define STAT_BBBBBBY			7
#define STAT_BBBBBBYY			8
#define STAT_BBBBBBYYY			9
#define STAT_BBBBBBYYYY			10
#define STAT_BBBBBBYYYYB		11
#define STAT_BBBBBBYYYYBB		12
#define STAT_BBBBBBYYYYBBY			13
#define STAT_BBBBBBYYYYBBYY			14
#define STAT_BBBBBBYYYYBBYYB			15
#define STAT_BBBBBBYYYYBBYYBB			16
#define STAT_BBBBBBYYYYBBYYBBB			17
#define STAT_BBBBBBYYYYBBYYBBBB			18
#define STAT_BBBBBBYYYYBBYYBBBBB		19
#define STAT_BBBBBBYYYYBBYYBBBBBB		20
#define STAT_BBBBBBYYYYBBYYBBBBBBY		21	
#define STAT_BBBBBBYYYYBBYYBBBBBBYY		22
#define STAT_BBBBBBYYYYBBYYBBBBBBYYY 		23
#define STAT_BBBBBBYYYYBBYYBBBBBBYYYY 		24
#define STAT_BBBBBBYYYYBBYYBBBBBBYYYYB		25
#define STAT_BBBBBBYYYYBBYYBBBBBBYYYYBB		26
#define STAT_BBBBBBYYYYBBYYBBBBBBYYYYBBY	27
#define STAT_BBBBBBYYYYBBYYBBBBBBYYYYBBYY	28
#define STAT_BBBBBBYYYYBBYYBBBBBBYYYYBBYYB 	29

#define S_BYTE_WAIT     1
#define S_BYTE_RECEIVED_DX      2
#define S_BYTE_RECEIVED_RX      3


#define BYTE_MODE_LETTERS       3
#define BYTE_MODE_FIGURES       4

#define E_BUFFER_SIZE 20
#define ERROR_THRESHOLD 12

#define PHASE_DETECTION_DIS_TIMER_VALUE  100*11   //11 seconds, 100 is bits per second



class byte_state_machine {    


  private:
     unsigned char code_to_ltrs[128]= {
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


     unsigned char code_to_figs[128] = {
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





     int ph1 = 0x07;
     int ph2 = 0x4c;

	char DX_byte;
	char dx_buffer[3];
	char error_buffer[E_BUFFER_SIZE];

	char temp_byte;
	char line_buffer[5000];  // 5000 because there cannot be more than 4000 characters transmitted in a 10 min slot
	char message_buffer[5000];  // 5000 because there cannot be more than 4000 characters transmitted in a 10 min slot
	char message_bbbb[10];
	int status;
	int byte_mode;
	int bits_received;
	int dx_buf_ptr;
	int dx_buf_filled;
	int byte_status;
	int error_count;
	int error_buffer_ptr;
	int error_buffer_filled;
	int end_of_emission_counter;
	int previous_DX_was_alpha;
	int phase_det_disable_timer;
	unsigned int freq;
	int byte_reception_enabled;
	int message_reception_ongoing;

void init();
void message_abort();
void message_byte_out(unsigned char byte_in);
void receive_rxdx_byte(unsigned char byte_received);
void message_line_out();
  



  public:         
	byte_state_machine(unsigned int frequency);
	void receive_bit(char bit_received);
};


#endif
