#include <raylib.h>
#include "../include/game.h"

int main() {
    // Initialize window
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Breakout");
    
    // Set target FPS
    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose()) {
        // Update
        
        // Draw
        BeginDrawing();
        ClearBackground(BLACK);
        
        EndDrawing();
    }

    // De-initialization
    CloseWindow();

    return 0;
} 