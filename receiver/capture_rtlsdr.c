#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include <rtl-sdr.h>

#include "capture_options.h"
#include "iq_pipeline.h"

#define NAVTEX_CENTER_FREQUENCY 504000U
#define IQ_SAMPLE_RATE 252000U
#define CONVERSION_BUFFER_SAMPLES 2048
#define USB_STRING_SIZE 256

typedef struct {
    rtlsdr_dev_t *device;
    unsigned char pending_i;
    int has_pending_i;
} RtlSdrCallbackContext;

static int find_rtlsdr_blog_v4(uint32_t *device_index)
{
    uint32_t device_count = rtlsdr_get_device_count();

    for (uint32_t i = 0; i < device_count; i++) {
        char manufacturer[USB_STRING_SIZE] = {0};
        char product[USB_STRING_SIZE] = {0};
        char serial[USB_STRING_SIZE] = {0};

        if (rtlsdr_get_device_usb_strings(i, manufacturer, product, serial) != 0) {
            fprintf(stderr, "Unable to read USB identity for RTL-SDR device %u\n", i);
            continue;
        }

        if (strcmp(manufacturer, "RTLSDRBlog") == 0 &&
            strcmp(product, "Blog V4") == 0) {
            *device_index = i;
            return 0;
        }
    }

    fprintf(stderr, "No RTL-SDR Blog V4 device found\n");
    return -1;
}

static void rtl_sdr_callback(unsigned char *buffer, uint32_t length, void *context)
{
    RtlSdrCallbackContext *callback_context = context;
    short xi[CONVERSION_BUFFER_SAMPLES];
    short xq[CONVERSION_BUFFER_SAMPLES];
    uint32_t offset = 0;

    if (callback_context->has_pending_i && length > 0) {
        xi[0] = (short)(((int)callback_context->pending_i - 128) * 256);
        xq[0] = (short)(((int)buffer[0] - 128) * 256);
        iq_pipeline_submit(xi, xq, 1);
        callback_context->has_pending_i = 0;
        offset = 1;
    }

    while (offset + 1 < length) {
        uint32_t sample_count = (length - offset) / 2;
        if (sample_count > CONVERSION_BUFFER_SAMPLES) {
            sample_count = CONVERSION_BUFFER_SAMPLES;
        }

        for (uint32_t i = 0; i < sample_count; i++) {
            xi[i] = (short)(((int)buffer[offset + 2 * i] - 128) * 256);
            xq[i] = (short)(((int)buffer[offset + 2 * i + 1] - 128) * 256);
        }

        iq_pipeline_submit(xi, xq, sample_count);
        offset += sample_count * 2;
    }

    if (offset != length) {
        callback_context->pending_i = buffer[offset];
        callback_context->has_pending_i = 1;
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
    return NULL;
}

void *capture_rtlsdr(void *options)
{
    const CaptureOptions *capture_options = options;
    uint32_t device_index;
    rtlsdr_dev_t *device = NULL;
    RtlSdrCallbackContext callback_context = {0};
    pthread_t reader_thread;
    int thread_error;
    int result;

    if (find_rtlsdr_blog_v4(&device_index) != 0) {
        return NULL;
    }

    result = rtlsdr_open(&device, device_index);
    if (result != 0) {
        fprintf(stderr, "Unable to open RTL-SDR Blog V4 (error %d)\n", result);
        return NULL;
    }
    callback_context.device = device;

    if (rtlsdr_set_sample_rate(device, IQ_SAMPLE_RATE) != 0) {
        fprintf(stderr, "Unable to set RTL-SDR sample rate to %u Hz\n", IQ_SAMPLE_RATE);
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

    if (iq_pipeline_init((int)IQ_SAMPLE_RATE) != 0) {
        goto cleanup;
    }

    if (rtlsdr_reset_buffer(device) != 0) {
        fprintf(stderr, "Unable to reset RTL-SDR sample buffer\n");
        iq_pipeline_destroy();
        goto cleanup;
    }

    fprintf(stderr, "RTL-SDR Blog V4 tuned to %u Hz at %u samples/s\n",
            NAVTEX_CENTER_FREQUENCY, IQ_SAMPLE_RATE);
    thread_error = pthread_create(&reader_thread, NULL, read_rtlsdr_samples,
                                  &callback_context);
    if (thread_error != 0) {
        fprintf(stderr, "Unable to start RTL-SDR reader thread: %s\n",
                strerror(thread_error));
        iq_pipeline_destroy();
        goto cleanup;
    }

    iq_pipeline_run(capture_options->debug_mode);

cleanup:
    if (rtlsdr_close(device) != 0) {
        fprintf(stderr, "Unable to close RTL-SDR device cleanly\n");
    }
    return NULL;
}
