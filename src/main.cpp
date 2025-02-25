#include <raylib.h>
#include "../include/game.h"
#include <emscripten.h>
#include <algorithm>

Game* gameInstance = nullptr;

// Function to get optimal window size maintaining aspect ratio
void getOptimalWindowSize(int& width, int& height) {
    // Get the monitor's dimensions
    int monitorWidth = GetMonitorWidth(GetCurrentMonitor());
    int monitorHeight = GetMonitorHeight(GetCurrentMonitor());
    
    // Calculate scaling factors for both dimensions
    float scaleX = static_cast<float>(monitorWidth) / Game::SpeedConfig::VIRTUAL_WIDTH;
    float scaleY = static_cast<float>(monitorHeight) / Game::SpeedConfig::VIRTUAL_HEIGHT;
    
    // Use the smaller scale to maintain aspect ratio
    float scale = std::min(scaleX, scaleY);
    
    // Calculate the new dimensions
    width = static_cast<int>(Game::SpeedConfig::VIRTUAL_WIDTH * scale);
    height = static_cast<int>(Game::SpeedConfig::VIRTUAL_HEIGHT * scale);
}

// Function to handle window resize
#ifdef __EMSCRIPTEN__
extern "C" {
#endif
    EMSCRIPTEN_KEEPALIVE
    void setWindowSize(int width, int height) {
        SetWindowSize(width, height);
        // Update game state if needed
        if (gameInstance) {
            Game::SpeedConfig::updateVirtualDimensions();
            gameInstance->updateCamera();
            gameInstance->resetBallAndPaddle();
        }
    }
#ifdef __EMSCRIPTEN__
}
#endif

int main() {
    // Enable window resizing and MSAA (remove unsupported flag)
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    
    // Initialize with full screen size
    int monitorWidth = GetMonitorWidth(GetCurrentMonitor());
    int monitorHeight = GetMonitorHeight(GetCurrentMonitor());
    InitWindow(monitorWidth, monitorHeight, "Breakout");
    
    // Enable touch gesture detection with correct flags
    // GESTURE_SWIPE is not supported, use specific directions if needed
    SetGesturesEnabled(GESTURE_TAP | GESTURE_DRAG);
    
    // Set target FPS and enable VSync for smoother rendering
    SetTargetFPS(60);
    
    // Create game instance and store pointer for resize handling
    Game game;
    gameInstance = &game;

    // Main game loop
    while (!WindowShouldClose()) {
        // Check if window was resized
        if (IsWindowResized()) {
            Game::SpeedConfig::updateVirtualDimensions();
            gameInstance->updateCamera();
        }
        game.run();
    }

    // Cleanup
    gameInstance = nullptr;
    CloseWindow();

    return 0;
}