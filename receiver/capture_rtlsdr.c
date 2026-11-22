#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include <rtl-sdr.h>

#include "capture_options.h"
#include "iq_pipeline.h"

#define NAVTEX_CENTER_FREQUENCY 504000U
#define IQ_SAMPLE_RATE IQ_PIPELINE_BASE_SAMPLE_RATE
#define RTL_DECIMATION_FACTOR 4U
#define RTL_INPUT_SAMPLE_RATE IQ_PIPELINE_HIGH_SAMPLE_RATE
#define CONVERSION_BUFFER_SAMPLES 2048
#define USB_STRING_SIZE 256

typedef struct {
    rtlsdr_dev_t *device;
    unsigned char pending_i;
    int has_pending_i;
} RtlSdrCallbackContext;

static void append_sample(unsigned char raw_i, unsigned char raw_q,
                          short *output_i, short *output_q,
                          uint32_t *output_count)
{
    output_i[*output_count] = (short)(((int)raw_i - 128) * 128);
    output_q[*output_count] = (short)(((int)raw_q - 128) * 128);
    (*output_count)++;
    if (*output_count == CONVERSION_BUFFER_SAMPLES) {
        iq_pipeline_submit(output_i, output_q, *output_count);
        *output_count = 0;
    }
}

static int find_rtlsdr_blog_v4(uint32_t *device_index, int report_errors)
{
    uint32_t device_count = rtlsdr_get_device_count();

    for (uint32_t i = 0; i < device_count; i++) {
        char manufacturer[USB_STRING_SIZE] = {0};
        char product[USB_STRING_SIZE] = {0};
        char serial[USB_STRING_SIZE] = {0};

        if (rtlsdr_get_device_usb_strings(i, manufacturer, product, serial) != 0) {
            if (report_errors) {
                fprintf(stderr, "Unable to read USB identity for RTL-SDR device %u\n", i);
            }
            continue;
        }

        if (strcmp(manufacturer, "RTLSDRBlog") == 0 &&
            strcmp(product, "Blog V4L") == 0) {
            *device_index = i;
            return 0;
        }
    }

    if (report_errors) {
        fprintf(stderr, "No RTL-SDR Blog V4L device found\n");
    }
    return -1;
}

static void rtl_sdr_callback(unsigned char *buffer, uint32_t length, void *context)
{
    RtlSdrCallbackContext *callback_context = context;
    short xi[CONVERSION_BUFFER_SAMPLES];
    short xq[CONVERSION_BUFFER_SAMPLES];
    uint32_t output_count = 0;
    uint32_t offset = 0;

    if (callback_context->has_pending_i && length > 0) {
        append_sample(callback_context->pending_i, buffer[0], xi, xq,
                      &output_count);
        callback_context->has_pending_i = 0;
        offset = 1;
    }

    while (offset + 1 < length) {
        append_sample(buffer[offset], buffer[offset + 1], xi, xq,
                      &output_count);
        offset += 2;
    }

    if (offset != length) {
        callback_context->pending_i = buffer[offset];
        callback_context->has_pending_i = 1;
    }

    if (output_count > 0) {
        iq_pipeline_submit(xi, xq, output_count);
    }
}

static void *read_rtlsdr_samples(void *context)
{
    RtlSdrCallbackContext *callback_context = context;
    int result = rtlsdr_read_async(callback_context->device, rtl_sdr_callback,
                                   callback_context, 0, 0);

    if (result != 0) {
        fprintf(stderr, "RTL-SDR asynchronous capture failed (error %d)\n", result);
    }
    iq_pipeline_stop();
    return NULL;
}

int capture_rtlsdr(const CaptureOptions *capture_options)
{
    uint32_t device_index;
    rtlsdr_dev_t *device = NULL;
    RtlSdrCallbackContext callback_context = {0};
    pthread_t reader_thread;
    int thread_error;
    int result;
    int pipeline_initialized = 0;
    int capture_started = 0;

    if (find_rtlsdr_blog_v4(&device_index, 1) != 0) {
        return 0;
    }

    result = rtlsdr_open(&device, device_index);
    if (result != 0) {
        fprintf(stderr, "Unable to open RTL-SDR Blog V4 (error %d)\n", result);
        return 0;
    }
    callback_context.device = device;

    if (rtlsdr_set_direct_sampling(device, 0) != 0) {
        fprintf(stderr, "Unable to disable RTL-SDR direct sampling mode\n");
        goto cleanup;
    }

    if (rtlsdr_set_bias_tee(device, capture_options->bias_t != 0) != 0) {
        fprintf(stderr, "Unable to %s RTL-SDR bias tee\n",
                capture_options->bias_t ? "enable" : "disable");
        goto cleanup;
    }
    if (capture_options->bias_t) {
        fprintf(stderr, "RTL-SDR bias tee enabled\n");
    }

    if (rtlsdr_set_sample_rate(device, RTL_INPUT_SAMPLE_RATE) != 0) {
        fprintf(stderr, "Unable to set RTL-SDR sample rate to %u Hz\n",
                RTL_INPUT_SAMPLE_RATE);
        goto cleanup;
    }

    if (rtlsdr_set_center_freq(device, NAVTEX_CENTER_FREQUENCY) != 0) {
        fprintf(stderr, "Unable to tune RTL-SDR to %u Hz\n", NAVTEX_CENTER_FREQUENCY);
        goto cleanup;
    }

    if (rtlsdr_set_tuner_gain_mode(device, 0) != 0) {
        fprintf(stderr, "Unable to enable RTL-SDR automatic tuner gain\n");
        goto cleanup;
    }

    if (iq_pipeline_init((int)RTL_INPUT_SAMPLE_RATE) != 0) {
        goto cleanup;
    }
    pipeline_initialized = 1;

    if (rtlsdr_reset_buffer(device) != 0) {
        fprintf(stderr, "Unable to reset RTL-SDR sample buffer\n");
        goto cleanup;
    }

    fprintf(stderr,
            "RTL-SDR Blog V4 tuned to %u Hz at %u samples/s, decimated by %u to %u samples/s\n",
            NAVTEX_CENTER_FREQUENCY, RTL_INPUT_SAMPLE_RATE,
            RTL_DECIMATION_FACTOR, IQ_SAMPLE_RATE);
    thread_error = pthread_create(&reader_thread, NULL, read_rtlsdr_samples,
                                  &callback_context);
    if (thread_error != 0) {
        fprintf(stderr, "Unable to start RTL-SDR reader thread: %s\n",
                strerror(thread_error));
        goto cleanup;
    }

    iq_pipeline_run(capture_options->debug_mode);
    thread_error = pthread_join(reader_thread, NULL);
    if (thread_error != 0) {
        fprintf(stderr, "Unable to join RTL-SDR reader thread: %s\n",
                strerror(thread_error));
    }
    capture_started = 1;

cleanup:
    if (pipeline_initialized) {
        iq_pipeline_destroy();
    }
    if (rtlsdr_close(device) != 0) {
        fprintf(stderr, "Unable to close RTL-SDR device cleanly\n");
    }
    return capture_started;
}
