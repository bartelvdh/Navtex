#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "capture_options.h"
#include "capture_rtlsdr.h"
#include "capture_sdrplay.h"

void init_fir_filter1(void);
void init_fir2_wrapper(void);

void init_dsp(void)
{
    init_fir_filter1();
}

int main(int argc, char *argv[])
{
    CaptureOptions options = {0, 0, 'A'};
    pthread_t capture_thread;
    char c;
    int use_rtlsdr = 0;
    int sdrplay_options_used = 0;

    opterr = 0;
    while ((c = (char)getopt(argc, argv, "brdp:")) != (char)-1) {
        switch (c) {
        case 'b':
            options.bias_t = 1;
            sdrplay_options_used = 1;
            printf("set BiasT active\n");
            break;
        case 'r':
            use_rtlsdr = 1;
            break;
        case 'd':
            options.debug_mode = 1;
            printf("debug mode enabled; a WAV file of the first 15 minutes of I/Q samples will be produced\n");
            break;
        case 'p':
            sdrplay_options_used = 1;
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

    if (use_rtlsdr && sdrplay_options_used) {
        fprintf(stderr, "Options -b and -p are only supported by the SDRplay backend\n");
        return 1;
    }

    init_dsp();
    init_fir2_wrapper();

    void *(*capture_function)(void *) = use_rtlsdr ? capture_rtlsdr : capture_sdrplay;
    int thread_error = pthread_create(&capture_thread, NULL, capture_function, &options);
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
