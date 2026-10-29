#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iq_pipeline.h"
#include "wav.h"

void sample_in_1(double sample_I, double sample_Q);

#define PIPELINE_BUFFER_SECONDS 8
#define DEBUG_RECORD_SECONDS 900

static pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
static short *sample_buffer;
static unsigned int buffer_size;
static unsigned int in_idx;
static unsigned int out_idx;
static int sample_rate;

static void generate_random_filename(char *filename)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    strcpy(filename, "NT");
    for (int i = 2; i < 10; i++) {
        int key = rand() % (sizeof(charset) - 1);
        filename[i] = charset[key];
    }
    filename[10] = '\0';
    strcat(filename, ".wav");
}

int iq_pipeline_init(int configured_sample_rate)
{
    if (configured_sample_rate <= 0) {
        fprintf(stderr, "Invalid I/Q sample rate: %d\n", configured_sample_rate);
        return -1;
    }

    buffer_size = (unsigned int)configured_sample_rate * 2 * PIPELINE_BUFFER_SECONDS;
    sample_buffer = malloc((size_t)buffer_size * sizeof(*sample_buffer));
    if (sample_buffer == NULL) {
        perror("Unable to allocate I/Q sample buffer");
        buffer_size = 0;
        return -1;
    }

    sample_rate = configured_sample_rate;
    in_idx = 1;
    out_idx = 1;
    return 0;
}

void iq_pipeline_submit(const short *xi, const short *xq, unsigned int num_samples)
{
    unsigned int next_idx;

    if (sample_buffer == NULL || xi == NULL || xq == NULL) {
        return;
    }

    pthread_mutex_lock(&buffer_mutex);
    next_idx = (in_idx + 1) % buffer_size;
    for (unsigned int i = 0; i < num_samples; i++) {
        sample_buffer[next_idx] = xi[i];
        next_idx = (next_idx + 1) % buffer_size;
        sample_buffer[next_idx] = xq[i];
        next_idx = (next_idx + 1) % buffer_size;
    }
    in_idx = (next_idx + buffer_size - 1) % buffer_size;
    pthread_mutex_unlock(&buffer_mutex);
}

void iq_pipeline_run(int debug_mode)
{
    unsigned int debug_frames_remaining = (unsigned int)sample_rate * DEBUG_RECORD_SECONDS;
    WavFile *wav_file = NULL;

    if (debug_mode) {
        char filename[80];
        generate_random_filename(filename);
        wav_file = wav_open(filename, WAV_OPEN_WRITE);
        if (wav_file == NULL) {
            fprintf(stderr, "Unable to open debug WAV file %s\n", filename);
        } else {
            wav_set_format(wav_file, WAV_FORMAT_PCM);
            wav_set_num_channels(wav_file, 2);
            wav_set_sample_rate(wav_file, (WavU32)sample_rate);
            wav_set_sample_size(wav_file, sizeof(short));
            fprintf(stderr, "Recording the first %d minutes of I/Q samples to %s\n",
                    DEBUG_RECORD_SECONDS / 60, filename);
        }
    }

    for (;;) {
        unsigned int current_in_idx;

        usleep(50000);
        pthread_mutex_lock(&buffer_mutex);
        current_in_idx = in_idx;
        pthread_mutex_unlock(&buffer_mutex);

        while (out_idx != current_in_idx) {
            unsigned int from = (out_idx + 1) % buffer_size;
            unsigned int to = current_in_idx;
            unsigned int sample_count;
            unsigned int frame_count;
            unsigned int frames_to_write;
            unsigned int index;

            if (from > to) {
                to = buffer_size - 1;
            }
            sample_count = to - from + 1;
            frame_count = sample_count / 2;

            index = from;
            for (unsigned int i = 0; i < frame_count; i++) {
                sample_in_1((double)sample_buffer[index],
                            (double)sample_buffer[index + 1]);
                index += 2;
            }

            frames_to_write = frame_count;
            if (wav_file != NULL && frames_to_write > debug_frames_remaining) {
                frames_to_write = debug_frames_remaining;
            }
            if (wav_file != NULL && frames_to_write > 0) {
                size_t written = wav_write(wav_file, &sample_buffer[from], frames_to_write);
                if (written != frames_to_write) {
                    fprintf(stderr, "Incomplete write to debug WAV file\n");
                    wav_close(wav_file);
                    wav_file = NULL;
                } else {
                    debug_frames_remaining -= frames_to_write;
                    if (debug_frames_remaining == 0) {
                        wav_close(wav_file);
                        wav_file = NULL;
                    }
                }
            }

            out_idx = to;
        }
    }
}

void iq_pipeline_destroy(void)
{
    free(sample_buffer);
    sample_buffer = NULL;
    buffer_size = 0;
    sample_rate = 0;
}
