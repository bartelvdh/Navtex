#ifndef IQ_PIPELINE_H
#define IQ_PIPELINE_H

int iq_pipeline_init(int sample_rate);
void iq_pipeline_submit(const short *xi, const short *xq, unsigned int num_samples);
void iq_pipeline_run(int debug_mode);
void iq_pipeline_destroy(void);

#endif
