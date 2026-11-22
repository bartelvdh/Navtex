#include <limits.h>
#include <stddef.h>

#include "fir0cpp.h"
#include "fir1cpp.h"

#define DECIMATION_FACTOR_0 4U
#define FILTER_SIZE_0 21U
#define OUTPUT_BUFFER_FRAMES 1024U

static const double filter_h_0[FILTER_SIZE_0] = {
    -0.000928753083968424,
    -0.003963405116660199,
    -0.001026352883960977,
    0.012645424460821819,
    0.018172064610594858,
    -0.011391012464726999,
    -0.056423892839638744,
    -0.037394461516342188,
    0.100277568318859991,
    0.289981592769995189,
    0.380102455490051450,
    0.289981592769995189,
    0.100277568318859991,
    -0.037394461516342188,
    -0.056423892839638799,
    -0.011391012464726999,
    0.018172064610594854,
    0.012645424460821819,
    -0.001026352883960977,
    -0.003963405116660202,
    -0.000928753083968424
};

struct buffer_item_0 {
    short sample_i;
    short sample_q;
};

static struct buffer_item_0 history_0[FILTER_SIZE_0];
static unsigned int history_index_0;
static unsigned int decimation_phase_0;

static short clamp_sample(double sample)
{
    if (sample > SHRT_MAX) {
        return SHRT_MAX;
    }
    if (sample < SHRT_MIN) {
        return SHRT_MIN;
    }
    return (short)sample;
}

void init_fir_filter0(void)
{
    for (unsigned int i = 0; i < FILTER_SIZE_0; i++) {
        history_0[i].sample_i = 0;
        history_0[i].sample_q = 0;
    }
    history_index_0 = 0;
    decimation_phase_0 = 0;
}

void sample_in_0(const short *interleaved_samples, unsigned int frame_count)
{
    short output_samples[OUTPUT_BUFFER_FRAMES * 2];
    unsigned int output_frames = 0;

    if (interleaved_samples == NULL) {
        return;
    }

    for (unsigned int frame = 0; frame < frame_count; frame++) {
        double filtered_i = 0.0;
        double filtered_q = 0.0;

        history_0[history_index_0].sample_i = interleaved_samples[frame * 2];
        history_0[history_index_0].sample_q = interleaved_samples[frame * 2 + 1];
        history_index_0++;
        if (history_index_0 == FILTER_SIZE_0) {
            history_index_0 = 0;
        }

        decimation_phase_0++;
        if (decimation_phase_0 != DECIMATION_FACTOR_0) {
            continue;
        }
        decimation_phase_0 = 0;

        unsigned int history_index = history_index_0;
        for (unsigned int tap = 0; tap < FILTER_SIZE_0; tap++) {
            history_index = history_index == 0 ? FILTER_SIZE_0 - 1 : history_index - 1;
            filtered_i += filter_h_0[tap] * history_0[history_index].sample_i;
            filtered_q += filter_h_0[tap] * history_0[history_index].sample_q;
        }

        output_samples[output_frames * 2] = clamp_sample(filtered_i);
        output_samples[output_frames * 2 + 1] = clamp_sample(filtered_q);
        output_frames++;
        if (output_frames == OUTPUT_BUFFER_FRAMES) {
            sample_in_1(output_samples, output_frames);
            output_frames = 0;
        }
    }

    if (output_frames > 0) {
        sample_in_1(output_samples, output_frames);
    }
}
