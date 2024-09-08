#ifndef _SIMPLE_GRAPHICS_H_
#define _SIMPLE_GRAPHICS_H_

#include "../main.h"


struct RGBColor {
    uint8_t r, g, b;
};

struct Position2d {
    int x, y;
};

struct Size2d {
    int width, height;
};


namespace simple_graphics {

    // Variables to be accessed from outside
    extern TTF_Font *font24;
    extern TTF_Font *font16;
    extern uint16_t  window_width;
    extern uint16_t  window_height;
    extern std::vector<SDL_Keycode> KEYS_PRESSED;

    // Window handling
    bool   init();
    bool   create_display(const char* title, uint16_t width = 800, uint16_t height = 600, uint32_t flags = 0);
    void   update_display();
    void   close_display();
    double limit_fps(uint fps = 60);

    // Graphics
    void fill_display(RGBColor color);
    void draw_rect(Position2d position, Size2d size, RGBColor color, bool filled);
    void draw_line(Position2d start, Position2d stop, RGBColor color);
    void draw_text(const char* text, Position2d position, TTF_Font *font, RGBColor color, bool aliasing);

}


#endif
