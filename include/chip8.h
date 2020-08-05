//
// Created by Rudolph Almeida on 26/7/20.
//

#pragma once

#include <cstdint>
#include <stack>
#include <vector>

class Chip8Interpreter {
   private:
    uint16_t opcode{};  // Current opcode

    uint8_t memory[4096]{};
    uint8_t V[16]{};  // 16 registers

    uint16_t I{};   // Index register
    uint16_t pc{};  // Program Counter

    bool gfx[64 * 32]{};  // Graphics state

    // Timers
    uint8_t delay_timer{};
    uint8_t sound_timer{};

    std::stack<uint16_t> stack;

    bool key[16]{};  // Key states 0-F

    // Program loaded into memory?
    bool programLoaded{};

   public:
    Chip8Interpreter() = default;  // Do nothing constructor

    // Boolean to draw the screen. Set by 0x00E0 and 0xDXYN
    bool draw{};

    // Initialize registers and memory once
    void initialize();

    // One CPU cycle
    void cycle();

    // Load program
    void load(const std::vector<uint8_t>& buffer);

    void clearScreen();

    void update_pixels(uint32_t* p_int) const;
    bool* get_key_state();
};
