#ifndef _AUDIO_CAPTURE_H_
#define _AUDIO_CAPTURE_H_


#include "../main.h"


#define BAR_COUNT 20


struct FrequencyBand {
    float lower_freq;
    float upper_freq;
};


std::vector<FrequencyBand> generate_frequency_bands(int num_bins, float start_freq, float end_freq);
std::vector<double> compute_fft();
void audio_capture_and_playback_thread();

extern std::vector<FrequencyBand> frequency_bands;
extern std::vector<short> system_audio_data;


#endif