#include "chip8app.h"

int main(int argc, char** argv) {
    Chip8App chip8_app(argc, argv);
    chip8_app.run();

    return 0;
}
