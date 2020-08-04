#include <fstream>
#include <iostream>
#include <iterator>

#include <SDL.h>

#include "chip8.h"

Chip8Interpreter chip8;

std::vector<uint8_t> load_chip8_rom(const char* filename);

int main() {
    const int WIDTH = 64;
    const int HEIGHT = 32;
    const int SCALE_FACTOR = 6;

    // Graphics Initialization
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init error: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Chip-8 Emulator", 100, 100, WIDTH * SCALE_FACTOR,
                                          HEIGHT * SCALE_FACTOR, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow error: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    // Create renderer
    SDL_Renderer* renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        SDL_DestroyWindow(window);
        std::cerr << "SDL_CreateRenderer error: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    // Load ROM and initialize interpreter
    auto rom = load_chip8_rom("./roms/ibm.ch8");

    chip8.initialize();
    chip8.load(rom);

    bool quit = false;
    SDL_Event event;
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                             SDL_TEXTUREACCESS_STATIC, WIDTH, HEIGHT);

    auto* pixels = new uint32_t[WIDTH * HEIGHT];
    memset(pixels, 0, WIDTH * HEIGHT * sizeof(uint32_t));

    // Render Loop Goes Here
    while (!quit) {
        SDL_UpdateTexture(texture, nullptr, pixels, WIDTH * sizeof(uint32_t));

        // Events
        SDL_WaitEvent(&event);
        if (event.type == SDL_QUIT) {
            quit = true;
            continue;
        }

        // Perform one cycle of CHIP-8 per frame
        chip8.cycle();

        if (chip8.draw) {
            chip8.update_pixels(pixels);
        }

        SDL_RenderClear(renderer);
        // Will scale the texture automatically to fit window
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    delete[] pixels;
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

std::vector<uint8_t> load_chip8_rom(const char* filename) {
    // open the file:
    std::ifstream file(filename, std::ios::binary);

    // Stop eating new lines in binary mode!!!
    file.unsetf(std::ios::skipws);

    // get its size:
    std::streampos fileSize;

    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // reserve capacity
    std::vector<uint8_t> vec;
    vec.reserve(fileSize);

    // read the data:
    vec.insert(vec.begin(), std::istream_iterator<uint8_t>(file), std::istream_iterator<uint8_t>());

    return vec;
}