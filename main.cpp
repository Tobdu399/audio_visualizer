#include "lib/main.h"
#include "lib/gui/simple_graphics.h"
#include "lib/audio/audio_capture.h"
#include "lib/audio/audio_visuals.h"


volatile bool PROCESS_INTERRUPTED = false;


void handle_sigint(int signal) {
    (void)signal;
    std::cout << YELLOW << "[WARN]" << CLEAR << " Keyboard interrupt!" << std::endl;
    PROCESS_INTERRUPTED = true;
}


int main() {
    signal(SIGINT, handle_sigint);


    double elapsed_time = 0.0;


    std::cout << MAGENTA << "\n"   
    " █████╗ ██╗   ██╗██████╗ ██╗ ██████╗     ██╗   ██╗██╗███████╗██╗   ██╗ █████╗ ██╗     ██╗███████╗███████╗██████╗\n" 
    "██╔══██╗██║   ██║██╔══██╗██║██╔═══██╗    ██║   ██║██║██╔════╝██║   ██║██╔══██╗██║     ██║╚══███╔╝██╔════╝██╔══██╗\n"
    "███████║██║   ██║██║  ██║██║██║   ██║    ██║   ██║██║███████╗██║   ██║███████║██║     ██║  ███╔╝ █████╗  ██████╔╝\n"
    "██╔══██║██║   ██║██║  ██║██║██║   ██║    ╚██╗ ██╔╝██║╚════██║██║   ██║██╔══██║██║     ██║ ███╔╝  ██╔══╝  ██╔══██╗\n"
    "██║  ██║╚██████╔╝██████╔╝██║╚██████╔╝     ╚████╔╝ ██║███████║╚██████╔╝██║  ██║███████╗██║███████╗███████╗██║  ██║\n"
    "╚═╝  ╚═╝ ╚═════╝ ╚═════╝ ╚═╝ ╚═════╝       ╚═══╝  ╚═╝╚══════╝ ╚═════╝ ╚═╝  ╚═╝╚══════╝╚═╝╚══════╝╚══════╝╚═╝  ╚═╝ v1.1"
    "\n" << CLEAR << std::endl;

    std::cout << GREEN << "[INFO]" << CLEAR << " Setup started. Initializing..." << std::endl;


    if (!simple_graphics::init()) {
        PROCESS_INTERRUPTED = true;
    }

    if (!simple_graphics::create_display("Audio Visualizer", WIDTH, HEIGHT, FLAGS)) {
        PROCESS_INTERRUPTED = true;
    }

    if (!audio_visuals::init()) {
        PROCESS_INTERRUPTED = true;
    }


    std::thread audio_thread(audio_capture_and_playback_thread);

    // Wait until the thread is initialized and running
    std::this_thread::sleep_for(std::chrono::milliseconds(100));


    std::cout << GREEN << "[INFO]" << CLEAR << " Setup complete. Program running..." << std::endl;


    while (!PROCESS_INTERRUPTED) {

        visualize_audio();  // Do all the necessary calculations

        // Clear the display and draw the particles first
        simple_graphics::fill_display(RGBColor{0, 0, 0});

        for (int i = 0; i < particles.size(); i++) {
            particles[i].update(elapsed_time);
            particles[i].draw();
        }

        // Draw the title and the info texts
        simple_graphics::draw_text(
            "Audio Visualizer v1.1", Position2d{10, 10},
            simple_graphics::font24, RGBColor{255, 255, 255}, true
        );

        simple_graphics::draw_text(
            "20 Hz",
            Position2d{
                WIDTH/2  - VISUALIZER_WIDTH/2 - 70,
                HEIGHT/2 - 11
            }, simple_graphics::font16, RGBColor{255, 255, 255}, true
        );
 
        simple_graphics::draw_text(
            "20 kHz",
            Position2d{
                WIDTH/2  + VISUALIZER_WIDTH/2 + 20,
                HEIGHT/2 - 11
            }, simple_graphics::font16, {255, 255, 255}, true
        );
 
        simple_graphics::draw_text(
            (std::string("Max intensity: ") + std::to_string(maximum_intensity)).c_str(),
            Position2d{
                WIDTH/2  - VISUALIZER_WIDTH/2  - 10,
                HEIGHT/2 + VISUALIZER_HEIGHT/2 + 20
            }, simple_graphics::font16, RGBColor{255, 255, 255}, true
        );

        // Draw the boxes around the audio visualizer
        simple_graphics::draw_rect(
            Position2d{
                WIDTH/2  - VISUALIZER_WIDTH/2  - 10,
                HEIGHT/2 - VISUALIZER_HEIGHT/2 - 10
            },
            Size2d{
                VISUALIZER_WIDTH  + 20,
                VISUALIZER_HEIGHT + 20
            }, RGBColor{20, 20, 20}, true
        );
        
        simple_graphics::draw_rect(
            Position2d{
                WIDTH/2  - VISUALIZER_WIDTH/2  - 10,
                HEIGHT/2 - VISUALIZER_HEIGHT/2 - 10
            },
            Size2d{
                VISUALIZER_WIDTH  + 20,
                VISUALIZER_HEIGHT + 20
            }, RGBColor{150, 150, 150}, false
        );

        // Update and draw the audio visualizer
        for (int i = 0; i < frequency_intensity_bars.size(); i++) {
            frequency_intensity_bars[i].update(elapsed_time);
            frequency_intensity_bars[i].draw();
        }


        // Handle keyboard inputs

        for (SDL_Keycode pressed_key : simple_graphics::KEYS_PRESSED) {
            if (pressed_key == SDLK_UP) {
                maximum_intensity += 100000;
            } else if (pressed_key == SDLK_DOWN) {
                maximum_intensity -= 100000;
                if (maximum_intensity < 100000) {
                    maximum_intensity = 100000;
                }
            } else if (pressed_key == SDLK_RETURN) {
                maximum_intensity = 600000;
            }
        }


        simple_graphics::update_display();
        elapsed_time = simple_graphics::limit_fps(60);
    }


    std::cout << GREEN << "[INFO]" << CLEAR << " Shutting down..." << std::endl;

    // Clean up
    simple_graphics::close_display();

    if (audio_thread.joinable())
        audio_thread.join();

    std::cout << GREEN << "[INFO]" << CLEAR << " Application shutdown complete.\n" << std::endl;

    return 0;
}
