#ifndef _MAIN_H_
#define _MAIN_H_

#include <iostream>
#include <chrono>
#include <thread>
#include <signal.h>
#include <cstdint>
#include <map>
#include <string>
#include <cstring>
#include <vector>
#include <mutex>
#include <algorithm>
#include <fftw3.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <fstream>
#include <alsa/asoundlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define CLEAR   "\e[0;0m"
#define GREEN   "\e[0;32m"
#define YELLOW  "\e[0;33m"
#define RED     "\e[0;31m"
#define MAGENTA "\e[0;95m"


// --- ONLY EDIT LINES BELOW THIS! ---


// The Audio Visualizer window resolution
#define WIDTH  1920
#define HEIGHT 1080

// Uncomment the following line if you want the Audio Visualizer to be in fullscreen mode.
//#define FLAGS SDL_WINDOW_FULLSCREEN_DESKTOP

// Only change the input device if you know what you are doing.
#define INPUT_DEVICE "hw:Loopback,1"

// The output device can be set to "default" or "hw:1,0" or "hw:[device name],0" etc., depending on
// the output device id and/or name. If set to "default" the program might not want to start or it
// might crash, so manually setting the correct output deivce is recommended. To see the list of
// available devices and their ids on your computer, type 'aplay -l' in your terminal.
#define OUTPUT_DEVICE "hw:X,0"


// --- ONLY EDIT LINES ABOVE THIS! ---


#ifndef FLAGS
#define FLAGS 0
#endif

extern volatile bool PROCESS_INTERRUPTED;

#endif
