#include <raylib.h>
#include "../include/game.h"

int main() {
    // Initialize window
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Breakout");
    
    // Set target FPS
    SetTargetFPS(60);

    // Create game instance
    Game game;

    // Main game loop
    while (!WindowShouldClose()) {
        game.run();
    }

    // De-initialization
    CloseWindow();

    return 0;
} 