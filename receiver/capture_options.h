#ifndef CAPTURE_OPTIONS_H
#define CAPTURE_OPTIONS_H

typedef enum {
    CAPTURE_BACKEND_AUTO,
    CAPTURE_BACKEND_RTLSDR,
    CAPTURE_BACKEND_SDRPLAY
} CaptureBackend;

typedef struct {
    int debug_mode;
    unsigned char bias_t;
    char antenna_port;
    CaptureBackend backend;
} CaptureOptions;

#endif
