#include "audio_visuals.h"
#include "audio_visuals.h"
#include "audio_capture.h"


std::vector<FreqIntensityBar> frequency_intensity_bars = {};
std::vector<Particle> particles = {};
uint maximum_intensity = 600000;

float width_of_area_for_bar = VISUALIZER_WIDTH / BAR_COUNT;
float bar_width             = width_of_area_for_bar * (2.f / 5.f);
float padding               = width_of_area_for_bar - bar_width;


bool audio_visuals::init() {
    for (int i = 0; i < BAR_COUNT; i++) {
        uint x = padding/2 + ((WIDTH / 2 - VISUALIZER_WIDTH / 2) + i * width_of_area_for_bar);
        RGBColor color = {255 - (255 / BAR_COUNT) * i, (255 / BAR_COUNT) * i, 0};
        frequency_intensity_bars.push_back(FreqIntensityBar(x, HEIGHT/2, bar_width, 0, color));
    }

    for (int i = 0; i < 1000; i++) {
        Particle particle;
        particles.push_back(particle);
    }

    return true;
}


std::vector<int> calculate_heights(std::vector<double> bin_intensities) {
    
    // Map band intensities to bar heights
    std::vector<int> heights(BAR_COUNT, 0);
    
    // Normalize each intensity to the fixed height
    for (int i = 0; i < BAR_COUNT; i++) {
        heights[i] = static_cast<int>(std::round((bin_intensities[i] / maximum_intensity) * frequency_intensity_bars[i].max_height));
    }

    return heights;
    
}


void visualize_audio() {

    std::vector<int> heights = calculate_heights(compute_fft());

    for (int i = 0; i < BAR_COUNT; i++) {
        frequency_intensity_bars[i].bar_target_height = heights[i];
    }
}
