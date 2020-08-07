//
// Created by Rudolph Almeida on 5/8/20.
//

#include <ctime>
#include <fstream>
#include <iostream>
#include <iterator>

#include "chip8app.h"

Chip8App::Chip8App(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: emuchip8 <rom file>" << std::endl;
        std::exit(-1);
    }

    std::srand(std::time(nullptr));

    rom_filename = argv[1];
    rom = read_rom(rom_filename);

    quit = false;

    init_SDL();

    keyboard_state = SDL_GetKeyboardState(nullptr);

    chip8.initialize();
    chip8.load(rom);
}

void Chip8App::init_SDL() {
    // Graphics Initialization
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init error: " << SDL_GetError() << "\n";
        std::exit(-1);
    }

    window = SDL_CreateWindow("Chip-8 Emulator", 100, 100, WIDTH * SCALE_FACTOR,
                              HEIGHT * SCALE_FACTOR, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow error: " << SDL_GetError() << "\n";
        SDL_Quit();
        std::exit(-1);
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        SDL_DestroyWindow(window);
        std::cerr << "SDL_CreateRenderer error: " << SDL_GetError() << "\n";
        SDL_Quit();
        std::exit(-1);
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, WIDTH,
                                HEIGHT);

    pixels = new uint32_t[WIDTH * HEIGHT];
    memset(pixels, 0, WIDTH * HEIGHT * sizeof(uint32_t));
}

Chip8App::~Chip8App() {
    delete[] pixels;

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Chip8App::run() {
    while (!quit) {
        bool* key_state = chip8.get_key_state();

        // Events
        if (SDL_PollEvent(&event) == 1) {
            if (event.type == SDL_QUIT) {
                quit = true;
                continue;
            }
            /*
             * Key layout
             * |1|2|3|4|  ->  |1|2|3|C|
             * |Q|W|E|R|  ->  |4|5|6|D|
             * |A|S|D|F|  ->  |7|8|9|E|
             * |Z|X|C|V|  ->  |A|0|B|F|
             * */
            key_state[1] = keyboard_state[SDL_SCANCODE_1] != 0;
            key_state[2] = keyboard_state[SDL_SCANCODE_2] != 0;
            key_state[3] = keyboard_state[SDL_SCANCODE_3] != 0;
            key_state[12] = keyboard_state[SDL_SCANCODE_4] != 0;
            key_state[4] = keyboard_state[SDL_SCANCODE_Q] != 0;
            key_state[5] = keyboard_state[SDL_SCANCODE_W] != 0;
            key_state[6] = keyboard_state[SDL_SCANCODE_E] != 0;
            key_state[13] = keyboard_state[SDL_SCANCODE_R] != 0;
            key_state[7] = keyboard_state[SDL_SCANCODE_A] != 0;
            key_state[8] = keyboard_state[SDL_SCANCODE_S] != 0;
            key_state[9] = keyboard_state[SDL_SCANCODE_D] != 0;
            key_state[14] = keyboard_state[SDL_SCANCODE_F] != 0;
            key_state[10] = keyboard_state[SDL_SCANCODE_Z] != 0;
            key_state[0] = keyboard_state[SDL_SCANCODE_X] != 0;
            key_state[11] = keyboard_state[SDL_SCANCODE_C] != 0;
            key_state[15] = keyboard_state[SDL_SCANCODE_V] != 0;
        }

        // Perform one cycle of CHIP-8 per frame
        chip8.cycle();

        if (chip8.draw) {
            chip8.update_pixels(pixels);
            SDL_UpdateTexture(texture, nullptr, pixels, WIDTH * sizeof(uint32_t));
        }

        SDL_RenderClear(renderer);
        // Will scale the texture automatically to fit window
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }
}

std::vector<uint8_t> Chip8App::read_rom(const char* filename) {
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
