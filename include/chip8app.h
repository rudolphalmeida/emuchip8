//
// Created by Rudolph Almeida on 5/8/20.
//

#pragma once

#include <cstdlib>
#include <vector>

#include <SDL.h>

#include "chip8.h"

class Chip8App {
   private:
    const int WIDTH = 64;
    const int HEIGHT = 32;
    const int SCALE_FACTOR = 6;

    SDL_Window* window{};
    SDL_Renderer* renderer{};
    SDL_Texture* texture{};

    SDL_Event event{};
    bool quit;

    const char* rom_filename;
    std::vector<uint8_t> rom;
    uint32_t* pixels{};
    const uint8_t* keyboard_state;

    Chip8Interpreter chip8;

    static std::vector<uint8_t> read_rom(const char* filename);
    void init_SDL();

   public:
    Chip8App(int, char**);
    virtual ~Chip8App();

    void run();
};
