#include <raylib.h>
#include "../include/game.h"
#include <emscripten.h>

Game* gameInstance = nullptr;

// Function to handle window resize
#ifdef __EMSCRIPTEN__
extern "C" {
#endif
    EMSCRIPTEN_KEEPALIVE
    void setWindowSize(int width, int height) {
        SetWindowSize(width, height);
        // Update game state if needed
        if (gameInstance) {
            // Force brick reinitialization with new dimensions
            gameInstance->initializeBricks();
            // Reset paddle and ball positions for new dimensions
            gameInstance->resetBallAndPaddle();
        }
    }
#ifdef __EMSCRIPTEN__
}
#endif

int main() {
    // Initialize with default size, will be updated by JavaScript
    const int defaultWidth = 800;
    const int defaultHeight = 600;
    
    InitWindow(defaultWidth, defaultHeight, "Breakout");
    SetTargetFPS(60);

    // Enable window resizing
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    
    // Create game instance and store pointer for resize handling
    Game game;
    gameInstance = &game;

    // Main game loop
    while (!WindowShouldClose()) {
        game.run();
    }

    // Cleanup
    gameInstance = nullptr;
    CloseWindow();

    return 0;
}