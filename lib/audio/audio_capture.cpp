#include "audio_capture.h"


unsigned int CHANNELS = 2;
unsigned int SAMPLE_RATE = 44100;
unsigned int BUFFER_TIME_MS = 50;
unsigned int FRAMES_PER_BUFFER = (SAMPLE_RATE * BUFFER_TIME_MS / 1000);

std::vector<short> system_audio_data(FRAMES_PER_BUFFER * CHANNELS);
std::mutex audio_mutex;

std::vector<FrequencyBand> frequency_bands(BAR_COUNT, {0.f, 0.f});


std::vector<FrequencyBand> generate_frequency_bands(int num_bins, float start_freq, float end_freq) {
    std::vector<FrequencyBand> frequency_bands(num_bins);

    // Logarithmic scale
    for (int i = 0; i < num_bins; i++) {
        float lower = start_freq * pow(end_freq / start_freq, static_cast<float>(i) / num_bins);
        float upper = start_freq * pow(end_freq / start_freq, static_cast<float>(i + 1) / num_bins);
        frequency_bands[i] = { lower, upper };
    }

    return frequency_bands;
}


void apply_pre_emphasis(std::vector<short>& audio_data, double alpha, std::vector<double>& pre_emphasized_data) {
    pre_emphasized_data.resize(FRAMES_PER_BUFFER);

    // Previous sample for pre-emphasis, initialized to zero
    double prev_sample = 0.0;

    for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
        // Stereo to mono mix
        double current_sample = (audio_data[i * 2] + audio_data[i * 2 + 1]) / 2.0;

        // Apply pre-emphasis filter
        pre_emphasized_data[i] = current_sample - alpha * prev_sample;

        // Store the current sample for the next iteration
        prev_sample = current_sample;
    }
}


std::vector<double> compute_fft() {
    fftw_complex* in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FRAMES_PER_BUFFER);
    fftw_complex* out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FRAMES_PER_BUFFER);
    fftw_plan plan = fftw_plan_dft_1d(FRAMES_PER_BUFFER, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    std::vector<double> pre_emphasized_data;

    // Lock audio data and apply pre-emphasis safely
    {
        std::lock_guard<std::mutex> lock(audio_mutex);
        apply_pre_emphasis(system_audio_data, 0.97, pre_emphasized_data);

        // Copy pre-emphasized data to FFT input
        for (int i = 0; i < FRAMES_PER_BUFFER; i++) {
            in[i][0] = pre_emphasized_data[i];  // Real part
            in[i][1] = 0.0;                     // Imaginary part set to 0
        }
    }

    // Perform FFT
    fftw_execute(plan);

    std::vector<double> bin_intensities(BAR_COUNT, 0.0);
    double freq_resolution = static_cast<double>(SAMPLE_RATE) / FRAMES_PER_BUFFER;

    // Calculate bin intensities (the logic remains the same)
    for (int bin = 0; bin < BAR_COUNT; bin++) {
        float lower_freq = frequency_bands[bin].lower_freq;
        float upper_freq = frequency_bands[bin].upper_freq;

        int start_idx = static_cast<int>(lower_freq / freq_resolution);
        int end_idx = std::min(static_cast<int>(upper_freq / freq_resolution), static_cast<int>(FRAMES_PER_BUFFER / 2));

        double bin_magnitude = 0.0;
        
        // Sum the magnitudes for the current frequency bin
        for (int i = start_idx; i <= end_idx; i++) {
            double real = out[i][0];
            double imag = out[i][1];
            double magnitude = sqrt(real * real + imag * imag);
            bin_magnitude += magnitude;
        }

        // Normalize by the number of FFT samples in the bin
        if (end_idx - start_idx > 0) {
            bin_magnitude /= (end_idx - start_idx);
        }

        // Custom scaling factor (boost higher frequencies more aggressively)
        double custom_scale_factor = (1.0 + (double)bin / 10.0);

        // Apply the custom scale factor to the magnitude
        bin_intensities[bin] = bin_magnitude * custom_scale_factor;
    }

    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);

    return bin_intensities;
}


void audio_capture_and_playback_thread() {
    // Audio source: hw:Loopback,1   (snd-aloop must be enabled!)
    // Audio output: default

    std::cout << GREEN << "[AC INFO]" << CLEAR << " Starting audio capture and playback thread..." << std::endl;


    frequency_bands = generate_frequency_bands(BAR_COUNT, 20, 20000);


    // Setup audio capture and playback

    snd_pcm_t* capture_handle      = nullptr;
    snd_pcm_t* playback_handle     = nullptr;
    snd_pcm_hw_params_t* hw_params = nullptr;
    short* local_buffer            = nullptr;
    int dir, rc;

    local_buffer = new short[FRAMES_PER_BUFFER * CHANNELS];

    // Open capture PCM
    rc = snd_pcm_open(&capture_handle, INPUT_DEVICE, SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        std::cout << RED << "[AC ERROR]" << CLEAR << " Unable to open PCM capture device." << std::endl;
        PROCESS_INTERRUPTED = true;
    }

    // Open playback PCM
    rc = snd_pcm_open(&playback_handle, OUTPUT_DEVICE, SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0) {
        std::cout << RED << "[AC ERROR]" << CLEAR << " Unable to open PCM playback device." << std::endl;
        PROCESS_INTERRUPTED = true;;
    }

    // Set up hardware parameters for capture
    snd_pcm_hw_params_malloc(&hw_params);
    if (!hw_params) {
        std::cout << RED << "[AC ERROR]" << CLEAR << " Failed to allocate hardware parameter structure." << std::endl;
        PROCESS_INTERRUPTED = true;;
    }

    snd_pcm_hw_params_any(capture_handle, hw_params);
    snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(capture_handle, hw_params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_rate_near(capture_handle, hw_params, &SAMPLE_RATE, &dir);
    snd_pcm_hw_params_set_channels(capture_handle, hw_params, CHANNELS);
    snd_pcm_uframes_t buffer_size = FRAMES_PER_BUFFER;
    snd_pcm_hw_params_set_period_size_near(capture_handle, hw_params, &buffer_size, &dir);
    
    rc = snd_pcm_hw_params(capture_handle, hw_params);
    if (rc < 0) {
        std::cout << RED << "[AC ERROR]" << CLEAR << " Unable to set HW parameters for capture." << std::endl;
        PROCESS_INTERRUPTED = true;;
    }

    snd_pcm_hw_params_free(hw_params);
    hw_params = nullptr;

    // Set up hardware parameters for playback
    snd_pcm_hw_params_malloc(&hw_params);
    if (!hw_params) {
        std::cout << RED << "[AC ERROR]" << CLEAR << " Failed to allocate hardware parameter structure." << std::endl;
        PROCESS_INTERRUPTED = true;;
    }

    snd_pcm_hw_params_any(playback_handle, hw_params);
    snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &SAMPLE_RATE, &dir);
    snd_pcm_hw_params_set_channels(playback_handle, hw_params, CHANNELS);
    buffer_size = FRAMES_PER_BUFFER;
    snd_pcm_hw_params_set_period_size_near(playback_handle, hw_params, &buffer_size, &dir);
    
    rc = snd_pcm_hw_params(playback_handle, hw_params);
    if (rc < 0) {
        std::cout << RED << "[AC ERROR]" << CLEAR << " Unable to set HW parameters for playback." << std::endl;
        PROCESS_INTERRUPTED = true;;
    }

    snd_pcm_hw_params_free(hw_params);
    hw_params = nullptr;

    // Prepare PCM devices
    rc = snd_pcm_prepare(capture_handle);
    if (rc < 0) {
        std::cout << RED << "[AC ERROR]" << CLEAR << " Unable to prepare PCM capture device." << std::endl;
        PROCESS_INTERRUPTED = true;;
    }

    rc = snd_pcm_prepare(playback_handle);
    if (rc < 0) {
        std::cout << RED << "[AC ERROR]" << CLEAR << " Unable to prepare PCM playback device." << std::endl;
        PROCESS_INTERRUPTED = true;;
    }

    std::cout << GREEN << "[AC INFO]" << CLEAR << " Audio capture and playback setup copmlete." << std::endl;

    std::cout << GREEN << "[AC INFO]" << CLEAR << " Audio recording and playback started." << std::endl;


    // Finally capture and output audio

    while (!PROCESS_INTERRUPTED) {
        int rc = snd_pcm_readi(capture_handle, local_buffer, FRAMES_PER_BUFFER);

        if (rc == -EPIPE) {
            std::cout << RED << "[AC ERROR]" << CLEAR << " Overrun occurred in capture." << std::endl;
            snd_pcm_prepare(capture_handle);
            PROCESS_INTERRUPTED = true;;

        } else if (rc < 0) {
            std::cout << RED << "[AC ERROR]" << CLEAR << " Cannot read from PCM capture device: " << snd_strerror(rc) << std::endl;
            PROCESS_INTERRUPTED = true;;  // Exit loop on serious error

        } else if (rc != (int)FRAMES_PER_BUFFER) {
            std::cout << YELLOW << "[AC WARN]" << CLEAR << " Short read from PCM capture device: read " << rc << " frames!" << std::endl;

        } else {
            std::lock_guard<std::mutex> lock(audio_mutex);
            std::copy(local_buffer, local_buffer + FRAMES_PER_BUFFER * CHANNELS, system_audio_data.begin());

            // Playback logic with similar error handling
            rc = snd_pcm_writei(playback_handle, local_buffer, FRAMES_PER_BUFFER);
            if (rc == -EPIPE) {
                std::cout << RED << "[AC ERROR]" << CLEAR << " Underrun occurred in playback." << std::endl;
                snd_pcm_prepare(playback_handle);
                PROCESS_INTERRUPTED = true;;

            } else if (rc < 0) {
                std::cout << RED << "[AC ERROR]" << CLEAR << " Cannot write to PCM playback device." << std::endl;
                PROCESS_INTERRUPTED = true;;  // Exit loop on serious error

            } else if (rc != (int)FRAMES_PER_BUFFER) {
                std::cout << YELLOW << "[AC WARN]" << CLEAR << " Short write to PCM playback device: wrote " << rc << " frames!" << std::endl;
            }
        }
    }

    // Cleanup on exit
    if (capture_handle) snd_pcm_close(capture_handle);
    if (playback_handle) snd_pcm_close(playback_handle);
    if (local_buffer) delete[] local_buffer;

    std::cout << YELLOW << "[AC WARN]" << CLEAR << " Audio capture and playback thread stopped!" << std::endl;
}
