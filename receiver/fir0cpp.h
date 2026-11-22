#ifndef FIR0CPP_H
#define FIR0CPP_H

#ifdef __cplusplus
extern "C" {
#endif

void init_fir_filter0(void);
void sample_in_0(const short *interleaved_samples, unsigned int frame_count);

#ifdef __cplusplus
}
#endif

#endif
