#include "simple_graphics.h"


namespace simple_graphics {


SDL_Window   *window       = nullptr;
SDL_Renderer *renderer     = nullptr;
SDL_Texture  *text_texture = nullptr;
TTF_Font     *font24       = nullptr;
TTF_Font     *font16       = nullptr;

std::vector<SDL_Keycode> KEYS_PRESSED;

uint16_t window_width = 0;
uint16_t window_height = 0;


// --- WINDOW HANDLING ---

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << RED << "[SG ERROR]" << CLEAR << " SDL could not be initialized." << std::endl;
        return false;
    }

    if (TTF_Init() == -1) {
        std::cout << RED << "[SG ERROR]" << CLEAR << " SDL_ttf could not be initialized." << std::endl;
        return false;
    }

    font24 = TTF_OpenFont("lib/gui/fonts/fira_code.ttf", 24);
    if (font24 == nullptr) {
        std::cout << RED << "[SG ERROR]" << CLEAR << " Could not load default font 24." << std::endl;
        return false;
    }

    font16 = TTF_OpenFont("lib/gui/fonts/fira_code.ttf", 16);
    if (font16 == nullptr) {
        std::cout << RED << "[SG ERROR]" << CLEAR << " Could not load default font 16." << std::endl;
        return false;
    }

    std::cout << GREEN << "[SG INFO]" << CLEAR << " SDL2 initialized." << std::endl;

    return true;
}


bool create_display(const char* title, uint16_t width, uint16_t height, uint32_t flags) {
    window_width  = width;
    window_height = height;

    window = SDL_CreateWindow(
            title,
            SDL_WINDOWPOS_CENTERED_DISPLAY(0),
            SDL_WINDOWPOS_CENTERED_DISPLAY(0),
            // SDL_WINDOWPOS_CENTERED_DISPLAY(0),
            // SDL_WINDOWPOS_CENTERED_DISPLAY(0),
            window_width, window_height,
            flags
    );

    if(!window) {
        std::cout << RED << "[SG ERROR]" << CLEAR << " Window could not be created." << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if(!renderer) {
        std::cout << RED << "[SG ERROR]" << CLEAR << " Could not create renderer." << std::endl;
        return false;
    }

    std::cout << GREEN << "[SG INFO]" << CLEAR << " SDL2 window created." << std::endl;

    return true;
}


double limit_fps(uint fps) {
    std::chrono::milliseconds target_fps(1000 / fps);
    static auto previous_frame_time = std::chrono::high_resolution_clock::now();
    auto current_frame_time         = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> frame_duration = current_frame_time - previous_frame_time;

    auto time_to_sleep = target_fps - frame_duration;

    if (time_to_sleep.count() > 0)
        std::this_thread::sleep_for(time_to_sleep);

    previous_frame_time = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> full_frame_duration = previous_frame_time - current_frame_time + frame_duration;
    return full_frame_duration.count();
}


void update_display() {
    if (window == nullptr || renderer == nullptr) {
        std::cout << RED << "[SG ERROR]" << CLEAR << " Cannot update nonexistent window and/or renderer." << std::endl;
        PROCESS_INTERRUPTED = true;
        return;
    }

    KEYS_PRESSED.clear();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if(event.type == SDL_QUIT) {
            PROCESS_INTERRUPTED = true;
        }

        if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
            SDL_Keycode key_pressed = event.key.keysym.sym;
            KEYS_PRESSED.push_back(key_pressed);
        }
    }

    SDL_RenderPresent(renderer);
}


void close_display() {
    SDL_DestroyRenderer(renderer);

    TTF_CloseFont(font24);
    TTF_CloseFont(font16);
    SDL_DestroyTexture(text_texture);

    SDL_DestroyWindow(window);

    TTF_Quit();
    SDL_Quit();

    std::cout << YELLOW << "[SG WARN]" << CLEAR << " SDL2 window closed!" << std::endl;
}



// --- WINDOW GRAPHICS ---

void fill_display(RGBColor color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 0);
    SDL_RenderClear(renderer);
}


void draw_rect(Position2d position, Size2d size, RGBColor color, bool filled) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 0);
    
    SDL_Rect rect;
    rect.x = position.x;
    rect.y = position.y;
    rect.w = size.width;
    rect.h = size.height;

    if (filled)
        SDL_RenderFillRect(renderer, &rect);
    else
        SDL_RenderDrawRect(renderer, &rect);
}


void draw_line(Position2d start, Position2d stop, RGBColor color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 0);
    SDL_RenderDrawLine(renderer, start.x, start.y, stop.x, stop.y);
}


void draw_text(const char* text, Position2d position, TTF_Font *font, RGBColor color, bool aliasing) {
    SDL_Surface* text_surface;

    if (aliasing)
        text_surface = TTF_RenderText_Blended(font, text, {color.r, color.g, color.b, 0});
    else
        text_surface = TTF_RenderText_Solid(font, text, {color.r, color.g, color.b, 0});

    if (text_surface == nullptr) {
        std::cout << RED << "[SG ERROR]" << CLEAR << " Unable to render text surface." << std::endl;
        return;
    }

    text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    if (text_texture == nullptr) {
        std::cout << RED << "[SG ERROR]" << CLEAR << " Unable to create texture from surface." << std::endl;
        return;
    };

    // Free the text surface now that we have a texture
    SDL_FreeSurface(text_surface);

    SDL_Rect render_quad = { position.x, position.y, text_surface->w, text_surface->h };
    SDL_RenderCopy(renderer, text_texture, nullptr, &render_quad);
}


} // namespace simple_graphics
