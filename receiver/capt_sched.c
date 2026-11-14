#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "capture_options.h"
#include "capture_rtlsdr.h"
#include "capture_sdrplay.h"

#define CAPTURE_RETRY_DELAY_SECONDS 60

void init_fir_filter1(void);
void init_fir2_wrapper(void);

void init_dsp(void)
{
    init_fir_filter1();
}

static int capture_preferred_backend(const CaptureOptions *options)
{
    if (options->backend == CAPTURE_BACKEND_RTLSDR) {
        return capture_rtlsdr(options);
    }
    return capture_sdrplay(options);
}

static void *capture_devices(void *context)
{
    const CaptureOptions *options = context;

    for (;;) {
        int capture_started;

        if (options->backend == CAPTURE_BACKEND_AUTO) {
            CaptureOptions rtl_options = *options;
            rtl_options.backend = CAPTURE_BACKEND_RTLSDR;
            capture_started = capture_rtlsdr(&rtl_options);
            if (capture_started) {
                capture_started = capture_sdrplay(options);
                if (capture_started) {
                    capture_rtlsdr(&rtl_options);
                }
            } else {
                capture_started = capture_sdrplay(options);
                if (capture_started) {
                    capture_rtlsdr(&rtl_options);
                }
            }
        } else {
            capture_preferred_backend(options);
        }

        fprintf(stderr, "Retrying hardware detection in %d seconds\n",
                CAPTURE_RETRY_DELAY_SECONDS);
        sleep(CAPTURE_RETRY_DELAY_SECONDS);
    }
}

int main(int argc, char *argv[])
{
    CaptureOptions options = {0, 0, 'A', CAPTURE_BACKEND_AUTO};
    pthread_t capture_thread;
    char c;
    int rtl_option_used = 0;
    int sdrplay_option_used = 0;
    int antenna_option_used = 0;

    opterr = 0;
    while ((c = (char)getopt(argc, argv, "brsdp:")) != (char)-1) {
        switch (c) {
        case 'b':
            options.bias_t = 1;
            printf("set BiasT active\n");
            break;
        case 'r':
            rtl_option_used = 1;
            options.backend = CAPTURE_BACKEND_RTLSDR;
            break;
        case 's':
            sdrplay_option_used = 1;
            options.backend = CAPTURE_BACKEND_SDRPLAY;
            break;
        case 'd':
            options.debug_mode = 1;
            printf("debug mode enabled; a WAV file of the first 15 minutes of I/Q samples will be produced\n");
            break;
        case 'p':
            antenna_option_used = 1;
            if (strcmp(optarg, "A") == 0) {
                options.antenna_port = 'A';
            } else if (strcmp(optarg, "B") == 0) {
                options.antenna_port = 'B';
            } else if (strcmp(optarg, "C") == 0 || strcmp(optarg, "Z") == 0) {
                options.antenna_port = 'C';
            } else {
                fprintf(stderr, "invalid antenna port option\n");
                return 1;
            }
            printf("Antenna port selected: %c\n", options.antenna_port);
            break;
        case '?':
            if (optopt == 'p') {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
            } else if (isprint(optopt)) {
                fprintf(stderr, "Unknown option `-%c'.\n", optopt);
            } else {
                fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
            }
            return 1;
        default:
            abort();
        }
    }

    if (rtl_option_used && sdrplay_option_used) {
        fprintf(stderr, "Options -r and -s cannot be used together\n");
        return 1;
    }

    if (rtl_option_used && antenna_option_used) {
        fprintf(stderr, "Option -p is only supported by the SDRplay backend\n");
        return 1;
    }
    if (antenna_option_used && options.backend == CAPTURE_BACKEND_AUTO) {
        options.backend = CAPTURE_BACKEND_SDRPLAY;
    }

    init_dsp();
    init_fir2_wrapper();

    int thread_error = pthread_create(&capture_thread, NULL, capture_devices, &options);
    if (thread_error != 0) {
        fprintf(stderr, "Unable to start capture thread: %s\n",
                strerror(thread_error));
        return 1;
    }

    for (;;) {
        sleep(61);
    }

    return 0;
}
