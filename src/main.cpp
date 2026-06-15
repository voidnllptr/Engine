#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <vulkan/vulkan.h>
#include "core/Engine.hpp"

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    try {
        Core::Engine engine;
        engine.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return -1;
    }
    catch (...) {
        std::cerr << "Unknown fatal error" << std::endl;
        return -1;
    }

    return 0;
}