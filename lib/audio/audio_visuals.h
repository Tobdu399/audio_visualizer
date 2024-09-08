#ifndef _AUDIO_VISUALS_H_
#define _AUDIO_VISUALS_H_

#include "../main.h"
#include "../gui/simple_graphics.h"


#define VISUALIZER_WIDTH  400
#define VISUALIZER_HEIGHT 200


namespace audio_visuals {
    bool init();
}


class FreqIntensityBar {

public:
    int x;
    int y;
    int bar_width;
    int bar_height = 0;
    int max_height = VISUALIZER_HEIGHT;
    int bar_target_height = 0;
    RGBColor bar_color;

    double time_since_last_update = 0;
    bool updated_within_second = false;


public:
    FreqIntensityBar(int x, int y, int width, int height, RGBColor color) {
        this->x    = x;
        this->y    = y;
        bar_width  = width;
        bar_height = height;
        bar_color  = color;
    }

    void update(double elapsed_time) {
        // Smooth transition logic
        double difference = bar_target_height - bar_height;

        // Adjust the speed dynamically based on the difference
        double speed = std::fabs(difference) * 0.020;   // 0.025 seems to be a good magic number

        // Update the bar height smoothly
        if (difference > 0) {
            bar_height += speed * elapsed_time;
            if (bar_height > bar_target_height) {
                bar_height = bar_target_height;
            }
        } else if (difference < 0) {
            bar_height -= speed * elapsed_time;
            if (bar_height < bar_target_height) {
                bar_height = bar_target_height;
            }
        }

        // Ensure the height stays within bounds
        if (bar_height > max_height) bar_height = max_height;
        if (bar_height < 2) bar_height = 2;
    }

    void draw() {
        // std::cout << "x: " << x << " y: " << y << " w: " << bar_width << " h: " << bar_height << std::endl;
        simple_graphics::draw_rect(Position2d{x, y - (bar_height/2)}, Size2d{bar_width, bar_height}, bar_color, true);
    }

};


extern uint maximum_intensity;
extern std::vector<FreqIntensityBar> frequency_intensity_bars;


class Particle {

private:
    float x, y, z;
    float target_brightness = 100;
    float current_brightness = 100;
    RGBColor color = {255, 255, 255};

    void reposition() {
        x = (WIDTH/2 - VISUALIZER_WIDTH/2) + rand() % VISUALIZER_WIDTH;
        y = (HEIGHT/2 - VISUALIZER_HEIGHT/2) + rand() % VISUALIZER_HEIGHT;
        z = 3 + rand() % 3;
    }

    float translate(int value, int start1, int stop1, int start2, int stop2) {
        return (((float)value-(float)start1)/((float)stop1-(float)start1))*((float)stop2-(float)start2)+(float)start2;
    }


public:
    Particle() {
        x = rand() % WIDTH;
        y = rand() % HEIGHT;
        z = 3 + rand() % 3;
    }

    void update(float elapsed_time) {
        // Make sure the particles stays within bounds
        if (x < 0 || x > WIDTH || y < 0 || y > HEIGHT) {
            reposition();
            return;
        }

        // Count 1/3 of the bars as bars that represent the bass intensity. Not the best solution but works.
        int count_bars = static_cast<int>(frequency_intensity_bars.size() * 0.33);
        float bass_intensity = 0.f;
        for (int i = 0; i < count_bars; i++) {
            bass_intensity += frequency_intensity_bars[i].bar_target_height;
        }
        bass_intensity /= count_bars;

        int color_value = translate(bass_intensity, 0, 50, 20, 255);
        if (color_value > 255) color_value = 255;
        target_brightness = color_value;

        // Update the brightness smoothly
        double difference = target_brightness - current_brightness;
        double adjustment_speed = std::fabs(difference) * 0.010;

        if (difference > 0) {
            current_brightness += adjustment_speed * elapsed_time;
            if (current_brightness > target_brightness) {
                current_brightness = target_brightness;
            }
        } else if (difference < 0) {
            current_brightness -= adjustment_speed * elapsed_time;
            if (current_brightness < target_brightness) {
                current_brightness = target_brightness;
            }
        }

        // Ensure the brightness stays within range 50-255
        if (current_brightness > 255) current_brightness = 255;
        if (current_brightness < 50) current_brightness = 50;

        // Move the particle
        // Max speed: elapsed_time / (25000 - 255^1.8) = elapsed_time / 3532,952...
        // Min speed: elapsed_time / (25000 -  50^1.8) = elapsed_time / 23856,737...

        float speed = elapsed_time / (25000 - pow(current_brightness, 1.8));

        z -= speed;
        if (z < 0) z = 0;
        x += (x - (float)WIDTH / 2.f) * (speed / z);
        y += (y - (float)HEIGHT / 2.f) * (speed / z);

    }

    void draw() {
        float alpha = 200 - ((z / 2) * 200);
        if (alpha > 200) alpha = 200;
        else if (alpha < 1) alpha = 1;

        // Calculate the new color with the alpha
        float brightness = alpha + ((alpha / 200.f) * 255);
        if (brightness > 255) brightness = 255;
        else if (brightness < 0) brightness = 0;

        float alpha_normalized = brightness / 255;

        RGBColor blended_color = {  // Works on black background only!
            current_brightness * alpha_normalized,
            current_brightness * alpha_normalized,
            current_brightness * alpha_normalized
        };

        simple_graphics::draw_rect(Position2d{(int)x, (int)y}, Size2d{2, 2}, blended_color, true);
    }

};

void visualize_audio();

extern std::vector<Particle> particles;


#endif