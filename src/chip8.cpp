//
// Created by Rudolph Almeida on 26/7/20.
//

#include <iostream>

#include <chip8.h>

void unknown_instruction_handler();

unsigned char chip8_fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
    0x20, 0x60, 0x20, 0x20, 0x70,  // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
    0xF0, 0x80, 0xF0, 0x80, 0x80   // F
};

void Chip8Interpreter::initialize() {
    // Program counter always starts at 0x200
    pc = 0x200;

    // Reset registers
    opcode = 0;
    I = 0;
    sp = 0;

    // Clear stack
    for (uint16_t& i : stack) {
        i = 0;
    }

    clearScreen();

    // Clear registers
    for (uint8_t& i : V) {
        i = 0;
    }

    // Clear keyboard
    for (bool& k : key) {
        k = false;
    }

    // Load fontset at 0x50
    for (size_t i = 0; i < 80; i++) {
        memory[i + 0x50] = chip8_fontset[i];
    }

    programLoaded = false;
}

void Chip8Interpreter::clearScreen() {  // Clear screen
    for (bool& pixel : gfx) {
        pixel = false;
    }
}

void Chip8Interpreter::cycle() {
    // Do nothing if program is not loaded
    if (!programLoaded) {
        return;
    }

    // For the handful of instructions that skip updating the PC
    bool updatePC = true;

    // Fetch opcode
    opcode = memory[pc] << 8 | memory[pc + 1];

    // Calculate nibbles
    // X is the second nibble
    uint16_t X = (opcode & 0x0F00) >> 8;  // Used to lookup VX
    uint16_t Y = (opcode & 0x00F0) >> 4;  // Used to lookup VY
    uint16_t N = (opcode & 0x000F);
    uint16_t NN = (opcode & 0x00FF);
    uint16_t NNN = (opcode & 0x0FFF);

    // Decode and Execute opcode
    switch (opcode & 0xF000) {
        case 0x0000: {
            switch (opcode & 0x0FFF) {
                // Screen Clear
                case 0x00E0: {
                    clearScreen();
                    break;
                }
                default: {
                    unknown_instruction_handler();
                    break;
                }
            }

            break;
        }
        case 0x1000: {
            pc = NNN;
            updatePC = false;
            break;
        }
        case 0x6000: {
            V[X] = NN;
            break;
        }
        case 0xA000: {
            I = NNN;
            break;
        }
        case 0xD000: {
            uint8_t x = V[X] & 0x3F;
            uint8_t y = V[Y] & 0x1F;
            uint8_t height = N;
            uint8_t pixel;

            V[0xF] = 0;

            for (int yline = 0; yline < height; ++yline) {
                pixel = memory[I + yline];
                for (int xline = 0; xline < 8; ++xline) {
                    if ((pixel & (0x80 >> xline)) != 0) {
                        if (gfx[(x + xline + ((y + yline) * 64))] == 1) {
                            V[0xF] = 1;
                        }
                        gfx[x + xline + ((y + yline) * 64)] ^= 1;
                    }
                }
            }

            draw = true;
            break;
        }
        default: {
            unknown_instruction_handler();
            break;
        }
    }

    if (updatePC) {
        pc += 2;
    }

    // Update timers
    if (delay_timer > 0) {
        --delay_timer;
    }

    if (sound_timer > 0) {
        if (sound_timer == 1) {  // Just print and terminal beep
            std::cerr << "Beep!\a\n";
        }
        --sound_timer;
    }
}

void Chip8Interpreter::load(const std::vector<uint8_t>& buffer) {
    for (size_t i = 0; i < buffer.size(); i++) {
        memory[i + 0x200] = buffer[i];
    }

    programLoaded = true;
}

void Chip8Interpreter::update_pixels(uint32_t* pixels) {
    for (int w = 0; w < 64; ++w) {
        for (int h = 0; h < 32; ++h) {
            pixels[h * 64 + w] = gfx[h * 64 + w] ? ~0 : 0;
        }
    }
}

void unknown_instruction_handler() {
    std::cerr << "Unknown instruction...\n";
}
