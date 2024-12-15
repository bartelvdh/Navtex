#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h> 
#include <regex.h>
#include <time.h>
#include <sys/wait.h>
#include "sdrplay_api.h"
#include "sdrplay_api_dev.h"
#include "message_store.h"

void init_fir_filter1();
void sample_in_1(double sample_I,double sample_Q);
void init_fir2_wrapper();

///////////// NAVTEX/SITOR-B /////////////
#define NAVTEX_UPPER 518000
#define NAVTEX_LOWER 490000

//first software decimation factor)
#define DECIMATION1 4

//252000
#define IN_SAMPLE_RATE 252000.0 

#define H_DECIMATION_FACTOR 8

//2016000
#define H_SAMPLE_RATE (IN_SAMPLE_RATE*H_DECIMATION_FACTOR*1.0)

#define TEMP_BUFFER_SIZE 1024 // must be a multiple of 4!!!

//14000
#define FREQ_OFFSET (NAVTEX_UPPER-NAVTEX_LOWER)/2

//504000
#define FREQ_TUNER  ((NAVTEX_UPPER+NAVTEX_LOWER)/2)

int in_sample_rate = (int)(IN_SAMPLE_RATE);


int masterInitialised = 0;
int slaveUninitialised = 0;

pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t in_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t out_mutex=PTHREAD_MUTEX_INITIALIZER;

sdrplay_api_DeviceT *chosenDevice = NULL;


short *sample_buffer;
unsigned int s_buffer_size;
int in_idx, out_idx;




void StreamACallback(short *xi, short *xq, sdrplay_api_StreamCbParamsT *params, unsigned int numSamples, unsigned int reset, void *cbContext)
{
    unsigned int i; 
    unsigned int next_idx;

    //protect this function, since it can be called multiple times 
    pthread_mutex_lock(&mutex);


    pthread_mutex_lock(&in_mutex);
    next_idx = in_idx+1; 
    pthread_mutex_unlock(&in_mutex);

    next_idx %= s_buffer_size ;

    for (i=0;i<numSamples;i++)
    {
	sample_buffer[next_idx]=xi[i];
        next_idx++; 
        next_idx %= s_buffer_size ;

	sample_buffer[next_idx]=xq[i];
        next_idx++; 
        next_idx %= s_buffer_size ;
    }
    if(next_idx == 0)
    {
   	next_idx = s_buffer_size-1;
    }
    else
    {
	next_idx--;
    }

    //protect in_idx as it is also read by the output thread
    pthread_mutex_lock(&in_mutex);
    in_idx = next_idx;  
    pthread_mutex_unlock(&in_mutex);
    
    pthread_mutex_unlock(&mutex);

    return;

}

void StreamBCallback(short *xi, short *xq, sdrplay_api_StreamCbParamsT *params, unsigned int numSamples, unsigned int reset, void *cbContext)
{
    if (reset)
        printf("sdrplay_api_StreamBCallback: numSamples=%d\n", numSamples);

    // Process stream callback data here - this callback will only be used in dual tuner mode

    return;
}

void EventCallback(sdrplay_api_EventT eventId, sdrplay_api_TunerSelectT tuner, sdrplay_api_EventParamsT *params, void *cbContext)
{
    switch(eventId)
    {
    case sdrplay_api_GainChange:
        printf("sdrplay_api_EventCb: %s, tuner=%s gRdB=%d lnaGRdB=%d systemGain=%.2f\n",
               "sdrplay_api_GainChange", (tuner == sdrplay_api_Tuner_A)? "sdrplay_api_Tuner_A":
               "sdrplay_api_Tuner_B", params->gainParams.gRdB, params->gainParams.lnaGRdB,
               params->gainParams.currGain);
        break;

    case sdrplay_api_PowerOverloadChange:
        printf("sdrplay_api_PowerOverloadChange: tuner=%s powerOverloadChangeType=%s\n",
               (tuner == sdrplay_api_Tuner_A)? "sdrplay_api_Tuner_A": "sdrplay_api_Tuner_B",
               (params->powerOverloadParams.powerOverloadChangeType ==
               sdrplay_api_Overload_Detected)? "sdrplay_api_Overload_Detected":
               "sdrplay_api_Overload_Corrected");
        // Send update message to acknowledge power overload message received
        sdrplay_api_Update(chosenDevice->dev, tuner, sdrplay_api_Update_Ctrl_OverloadMsgAck,sdrplay_api_Update_Ext1_None);

        break;

    case sdrplay_api_RspDuoModeChange:
        printf("sdrplay_api_EventCb: %s, tuner=%s modeChangeType=%s\n",
               "sdrplay_api_RspDuoModeChange", (tuner == sdrplay_api_Tuner_A)?
               "sdrplay_api_Tuner_A": "sdrplay_api_Tuner_B",
               (params->rspDuoModeParams.modeChangeType == sdrplay_api_MasterInitialised)?
               "sdrplay_api_MasterInitialised":
               (params->rspDuoModeParams.modeChangeType == sdrplay_api_SlaveAttached)?
               "sdrplay_api_SlaveAttached":
               (params->rspDuoModeParams.modeChangeType == sdrplay_api_SlaveDetached)?
               "sdrplay_api_SlaveDetached":
               (params->rspDuoModeParams.modeChangeType == sdrplay_api_SlaveInitialised)?
               "sdrplay_api_SlaveInitialised":
               (params->rspDuoModeParams.modeChangeType == sdrplay_api_SlaveUninitialised)?
               "sdrplay_api_SlaveUninitialised":
               (params->rspDuoModeParams.modeChangeType == sdrplay_api_MasterDllDisappeared)?
               "sdrplay_api_MasterDllDisappeared":
               (params->rspDuoModeParams.modeChangeType == sdrplay_api_SlaveDllDisappeared)?
               "sdrplay_api_SlaveDllDisappeared": "unknown type");

        if (params->rspDuoModeParams.modeChangeType == sdrplay_api_MasterInitialised)
        {
            masterInitialised = 1;
        }
        if (params->rspDuoModeParams.modeChangeType == sdrplay_api_SlaveUninitialised)
        {
            slaveUninitialised = 1;
        }
        break;

    case sdrplay_api_DeviceRemoved:
        printf("sdrplay_api_EventCb: %s\n", "sdrplay_api_DeviceRemoved");
        break;

    default:
        printf("sdrplay_api_EventCb: %d, unknown event\n", eventId);
        break;
    }
}



int captureIQ()
{
    sdrplay_api_DeviceT devs[6];
    unsigned int ndev;
    int i;
    int c_in_idx;
    float ver = 0.0;
    sdrplay_api_ErrT err;
    sdrplay_api_DeviceParamsT *deviceParams = NULL;
    sdrplay_api_CallbackFnsT cbFns;
    sdrplay_api_RxChannelParamsT *chParams;
    //unsigned int byte_count = 0;
    unsigned int num_samples;

    int reqTuner = 0;
    int master_slave = 0;

    unsigned int chosenIdx = 0;
    unsigned int from,to;
    
    printf("requested Tuner%c Mode=%s\n", (reqTuner == 0)? 'A': 'B', (master_slave == 0)?
               "Single_Tuner": "Master/Slave");
    
    // Open API//////////
    if ((err = sdrplay_api_Open()) != sdrplay_api_Success)
    {
        printf("sdrplay_api_Open failed %s\n", sdrplay_api_GetErrorString(err));
    }
    else
    {
        // Enable debug logging output////////
        //if ((err = sdrplay_api_DebugEnable(NULL, 1)) != sdrplay_api_Success)
        //{
        //    printf("sdrplay_api_DebugEnable failed %s\n", sdrplay_api_GetErrorString(err));
        //}

        // Check API versions match /////////////
        if ((err = sdrplay_api_ApiVersion(&ver)) != sdrplay_api_Success)
        {
            printf("sdrplay_api_ApiVersion failed %s\n", sdrplay_api_GetErrorString(err));
        }
	else
	{
            printf("sdrplay_api_ApiVersion: %f\n", SDRPLAY_API_VERSION);
	}
        if (ver != SDRPLAY_API_VERSION)
        {
            printf("API version don't match (local=%.2f dll=%.2f)\n", SDRPLAY_API_VERSION, ver);
            goto CloseApi;
        }

        // Lock API while device selection is performed /////////////
        sdrplay_api_LockDeviceApi();

        // Fetch list of available devices
        if ((err = sdrplay_api_GetDevices(devs, &ndev, sizeof(devs) /
               sizeof(sdrplay_api_DeviceT))) != sdrplay_api_Success)
        {
            printf("sdrplay_api_GetDevices failed %s\n", sdrplay_api_GetErrorString(err));
            goto UnlockDeviceAndCloseApi;
        }
        printf("MaxDevs=%d NumDevs=%d\n", sizeof(devs) / sizeof(sdrplay_api_DeviceT), ndev);
        if (ndev > 0)
        {
            for (i = 0; i < (int)ndev; i++)
            {
                if (devs[i].hwVer == SDRPLAY_RSPduo_ID)
                    printf("Dev%d: SerNo=%s hwVer=%d tuner=0x%.2x rspDuoMode=0x%.2x\n", i,
                       devs[i].SerNo, devs[i].hwVer , devs[i].tuner, devs[i].rspDuoMode);
                else
                    printf("Dev%d: SerNo=%s hwVer=%d tuner=0x%.2x\n", i, devs[i].SerNo,
                       devs[i].hwVer, devs[i].tuner);
            }

            // Choose device
            {
                // Pick first device of any type
                for (i = 0; i < (int)ndev; i++)
                {
                    chosenIdx = i;
                    break;
                }
            }
            if (i == ndev)
            {
                printf("Couldn't find a suitable device to open - exiting\n");
                goto UnlockDeviceAndCloseApi;
            }
            printf("chosenDevice = %d\n", chosenIdx);
            chosenDevice = &devs[chosenIdx];

            // Select chosen device //////////
            if ((err = sdrplay_api_SelectDevice(chosenDevice)) != sdrplay_api_Success)
            {
                printf("sdrplay_api_SelectDevice failed %s\n", sdrplay_api_GetErrorString(err));
                goto UnlockDeviceAndCloseApi;
            }

            // Unlock API now that device is selected
            sdrplay_api_UnlockDeviceApi();

            // Get Device paramters ///////////////// 
            if ((err = sdrplay_api_GetDeviceParams(chosenDevice->dev, &deviceParams)) !=
               sdrplay_api_Success)
            {
                printf("sdrplay_api_GetDeviceParams failed %s\n",
                      sdrplay_api_GetErrorString(err));
                goto CloseApi;
            }
		else
		{
                printf("sdrplay_api_GetDeviceParams retrieved\n");
		}

            // Check for NULL pointers before changing settings
            if (deviceParams == NULL)
            {
                printf("sdrplay_api_GetDeviceParams returned NULL deviceParams pointer\n");
                goto CloseApi;
            }

            // Configure dev parameters  /////////////////////
            if (deviceParams->devParams != NULL)
            {
              // This will be NULL for slave devices, only the master can change these parameters
                // Only need to update non-default settings
                if (master_slave == 0)
                {
 		    deviceParams->devParams->mode = sdrplay_api_ISOCH; 
 		    //deviceParams->devParams->mode = sdrplay_api_BULK; 
                    deviceParams->devParams->fsFreq.fsHz = H_SAMPLE_RATE ;

        	    deviceParams->devParams->rspDxParams.antennaSel = sdrplay_api_RspDx_ANTENNA_B;
		    deviceParams->devParams->rspDxParams.biasTEnable = 1;

        	    deviceParams->devParams->rspDxParams.rfNotchEnable=0;
        	    deviceParams->devParams->rspDxParams.rfDabNotchEnable=1;

            // Configure tuner parameters (depends on selected Tuner which set of parameters to use)
            chParams = deviceParams->rxChannelA;
            if (chParams != NULL)
            {


                chParams->tunerParams.loMode = sdrplay_api_LO_Auto;
                chParams->tunerParams.rfFreq.rfHz = (float)FREQ_TUNER; //(1/4 times sampling freq after decimation)
                chParams->tunerParams.bwType = sdrplay_api_BW_0_200;
                chParams->tunerParams.ifType = sdrplay_api_IF_Zero;
                //chParams->tunerParams.ifType = sdrplay_api_IF_0_450;
                chParams->tunerParams.gain.gRdB = 30;
                //chParams->tunerParams.gain.LNAstate = 3; // LNAstate3 = 9dB (gain reduction)
                //chParams->tunerParams.gain.LNAstate = 4; // LNAstate4 = 12dB (gain reduction)
		//chParams->tunerParams.gain.LNAstate = 5; // LNAstate5 = 15dB (gain reduction)
                //chParams->tunerParams.gain.LNAstate = 8; // LNAstate4 = 30dB (gain reduction)
                chParams->tunerParams.gain.LNAstate = 0; // LNAstate0 = 0dB  FOR USE WITH FURUNO NAVTEX ANTENNA
        	chParams->ctrlParams.decimation.enable=1;
		chParams->ctrlParams.decimation.decimationFactor=  H_DECIMATION_FACTOR;
                // set AGC
                //chParams->ctrlParams.agc.enable = sdrplay_api_AGC_DISABLE;
                chParams->ctrlParams.agc.enable = sdrplay_api_AGC_5HZ;
            }
            else
            {
                printf("sdrplay_api_GetDeviceParams returned NULL chParams pointer\n");
                goto CloseApi;
            }
        	printf("devParams->fsFreq.fsHz: %f \n", deviceParams->devParams->fsFreq.fsHz);
        	printf("rxChannelA->tunerParams.rfFreq.rfHz: %f \n", deviceParams->rxChannelA->tunerParams.rfFreq.rfHz);

                }
                else
                {
                    // Can't change Fs in master/slave mode
                printf("unexpected slave device config\n");
                goto CloseApi;
                }
            }


            // Assign callback functions to be passed to sdrplay_api_Init()
            cbFns.StreamACbFn = StreamACallback;
            cbFns.StreamBCbFn = StreamBCallback;
            cbFns.EventCbFn = EventCallback;



   	    s_buffer_size = in_sample_rate*2*8;   // *2 for I and Q; *8 for 8 seconds of buffer; s_buffer_size must be a multiple of 2
	    sample_buffer = malloc(s_buffer_size*sizeof(short)); 
            in_idx  = 1;  // should not be zero!!
	    out_idx = 1; // idem

	    //////////////////// LET'S START ///////////////////////////////////
            // Now we're ready to start by calling the initialisation function
            // This will configure the device and start streaming

            if ((err = sdrplay_api_Init(chosenDevice->dev, &cbFns, NULL)) != sdrplay_api_Success)
            {
                printf("sdrplay_api_Init failed %s\n", sdrplay_api_GetErrorString(err));
                if (err == sdrplay_api_StartPending) // This can happen if we're starting in master/slave mode as a slave and the master is not yet running
                {
                    while(1)
                    {
                        sleep(1);
                        if (masterInitialised) // Keep polling flag set in event callback until the master is initialised
                        {
                            // Redo call - should succeed this time
                            if ((err = sdrplay_api_Init(chosenDevice->dev, &cbFns, NULL)) != sdrplay_api_Success)
                            {
                                printf("sdrplay_api_Init failed %s\n", sdrplay_api_GetErrorString(err));
                            }
                        printf("Waiting for master to initialise\n");
                            goto CloseApi;
                        }
                    }
                }
                else
		{

		}
                    goto CloseApi;
            }


 	    /////// MAIN CONSUMER LOOP ///////
            int go_on=1;
	    while(go_on)
	    {
		usleep(50000);
    		pthread_mutex_lock(&in_mutex);
	  	c_in_idx = in_idx;
    		pthread_mutex_unlock(&in_mutex);


           	while(out_idx != c_in_idx)
	        {
			int i;
			int index;

                	from = out_idx+1;
        		from %= s_buffer_size ;
			to = c_in_idx;
			if(from>to)
			{
				to=s_buffer_size-1;
			}
                        //time to transfer  from from to to
			num_samples=(to-from+1);  //one sample is an I or a Q value
			//num_bytes=num_samples*sizeof(short);

			index=from;
			for(i=0;i<num_samples;i+=2)
			{
		           sample_in_1((double) sample_buffer[index],(double) sample_buffer[index+1]);
			   index+=2;
			} 


			//byte_count += num_bytes;
			//printf("out_bytecount=%u\n",byte_count);
		 	out_idx = to;	
		}		
	    }



            // Release device (make it available to other applications)
            sdrplay_api_ReleaseDevice(chosenDevice);
   	    free(sample_buffer);  
        }
UnlockDeviceAndCloseApi:
        // Unlock API
        sdrplay_api_UnlockDeviceApi();

CloseApi:
        // Close API
        sdrplay_api_Close();
    }
    return 0;
}


int seconds_to_next_10_min_slot() {
    // Get the current time
    time_t now = time(NULL);
    
    // Convert to struct tm for UTC
    struct tm *timeinfo = gmtime(&now);

    // Calculate the seconds passed since the last 10-minute slot
    int minutes = timeinfo->tm_min % 10;
    int seconds = timeinfo->tm_sec;
    int seconds_passed = minutes * 60 + seconds;

    // Calculate the remaining seconds to the next 10-minute slot
    int seconds_to_next_slot = 10 * 60 - seconds_passed ;

    return seconds_to_next_slot;
}

int next_10_minute_slot() {
    // Get the current time
    time_t now = time(NULL);
    
    // Convert to struct tm for local time
    struct tm *timeinfo = gmtime(&now);

    // Calculate the number of 10-minute slots passed since the start of the day
    int slots_passed = timeinfo->tm_hour * 6 + timeinfo->tm_min / 10;

    // The next 10-minute slot
    int next_slot = (slots_passed+1) % 24 ;

    return next_slot;
}

int maximum(int a, int b) {
 	if(a>b) {
		return(a);
 	} else {
		return(b);
	}
}


void *scheduler(void *prt)
{

			captureIQ();
			return(NULL);
}





void init_dsp()
{
        init_fir_filter1();
}


int main(int argc, char *argv[])
{
    pthread_t thread2;
    
    if (argc!=1)
 	{
		printf("invalid arguments\n");
		exit(-1);
	}
    
   init_dsp();
   init_fir2_wrapper();



   pthread_create( &thread2, (pthread_attr_t *)NULL, scheduler, (void *) NULL ); 

   for(;;)
   {
	sleep(61);
   }

   return 0;
}

