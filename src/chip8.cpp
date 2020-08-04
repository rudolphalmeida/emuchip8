//
// Created by Rudolph Almeida on 26/7/20.
//

#include <iostream>

#include <chip8.h>

void unknown_instruction_handler(uint16_t);

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

    // Clear stack
    while (!stack.empty()) {
        stack.pop();
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

    draw = false;

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
                    draw = true;

                    break;
                }
                case 0x00EE: {
                    pc = stack.top();  // Return from co-routine
                    stack.pop();

                    break;
                }
                default: {
                    unknown_instruction_handler(opcode);
                    break;
                }
            }

            break;
        }
        case 0x1000: {  // Jump to NNN
            pc = NNN;
            updatePC = false;
            break;
        }
        case 0x2000: {
            stack.push(pc);  // Push current PC to stack and jump to NNN
            pc = NNN;
            updatePC = false;
            break;
        }
        case 0x3000: {  // Jump if equal (VX and NN)
            if (NN == V[X]) {
                pc += 2;  // Effective increment of 4 after switch
            }

            break;
        }
        case 0x4000: {  // Jump if not equal (VX and NN)
            if (NN != V[X]) {
                pc += 2;  // Effective increment of 4 after switch
            }

            break;
        }
        case 0x5000: {  // Jump if VX and VY are equal
            if (V[X] == V[Y]) {
                pc += 2;  // Effective increment of 4 after switch
            }

            break;
        }
        case 0x6000: {  // Set VX to NN
            V[X] = NN;
            break;
        }
        case 0x7000: {  // Add NN to VX
            V[X] += NN;
            break;
        }
        case 0x8000: {               // Arithmetic and Logical instructions
            switch (opcode & 0xF) {  // Based on last nibble
                case 0x0: {          // Set
                    V[X] = V[Y];
                    break;
                }
                case 0x1: {  // Logical OR
                    V[X] |= V[Y];
                    break;
                }
                case 0x2: {  // Logical AND
                    V[X] &= V[Y];
                    break;
                }
                case 0x3: {  // Logical XOR
                    V[X] ^= V[Y];
                    break;
                }
                case 0x4: {  // Add
                    int sum = V[X] + V[Y];
                    if (sum > 255) {  // Overflow. Carry must be set
                        V[X] = 255;
                        V[15] = 1;  // Set carry flag VF
                    } else {        // No overflow
                        V[X] = sum;
                        V[15] = 0;  // Reset carry flag VF
                    }

                    break;
                }
                case 0x5: {             // Subtract VY from VX and set to VX
                    if (V[X] > V[Y]) {  // Carry flag should be set
                        V[X] -= V[Y];
                        V[15] = 1;  // Set carry flag VF
                    } else {
                        V[X] -= V[Y];
                        V[15] = 0;  // Reset carry flag VF
                    }

                    break;
                }
                case 0x7: {             // Subtract VX from VY and set to VX
                    if (V[Y] > V[X]) {  // Carry flag should be set
                        V[X] = V[Y] - V[X];
                        V[15] = 1;  // Set carry flag VF
                    } else {
                        V[X] = V[Y] - V[X];
                        V[15] = 0;  // Reset carry flag VF
                    }

                    break;
                }
                case 0x6: {  // Shift right
                    V[X] = V[Y];
                    V[15] = V[X] & 0b00000001;  // The bit that will be shifted out
                    V[X] >>= 1;

                    break;
                }
                case 0xE: {  // Shift left
                    V[X] = V[Y];
                    V[15] = V[X] & 0b10000000;  // The bit that will be shifted out
                    V[X] <<= 1;
                }
                default: {
                    unknown_instruction_handler(opcode);
                    break;
                }
            }

            break;
        }
        case 0x9000: {  // Jump if VX and VY are not equal
            if (V[X] != V[Y]) {
                pc += 2;  // Effective increment of 4 after switch
            }

            break;
        }
        case 0xA000: {  // Set index register
            I = NNN;
            break;
        }
        case 0xB000: {        // Jump with offset
            pc = NNN + V[0];  // Different in CHIP-48 and SUPER-CHIP
            updatePC = false;

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
            unknown_instruction_handler(opcode);
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
    // Load program at 0x200
    for (size_t i = 0; i < buffer.size(); i++) {
        memory[i + 0x200] = buffer[i];
    }

    programLoaded = true;
}

void Chip8Interpreter::update_pixels(uint32_t* pixels) {
    for (int w = 0; w < 64; ++w) {
        for (int h = 0; h < 32; ++h) {
            // Set to all 1 if corresponding gfx is true else 0
            pixels[h * 64 + w] = gfx[h * 64 + w] ? ~0 : 0;
        }
    }
}

void unknown_instruction_handler(uint16_t opcode) {
    std::cerr << "Unknown instruction: " << std::hex << opcode << std::dec << "\n";
}
