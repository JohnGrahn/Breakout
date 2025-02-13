#include "../include/game.h"
#include <cstdlib>  // For abs function

// Game destructor implementation
Game::~Game() = default;

// Paddle implementation
Game::Paddle::Paddle(float x, float y, float width, float height, float speed)
    : x(x), y(y), width(width), height(height), speed(speed) {}

void Game::Paddle::update(float deltaTime) {
    if (IsKeyDown(KEY_LEFT)) {
        x -= speed * deltaTime;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        x += speed * deltaTime;
    }
    
    // Keep paddle within screen bounds
    if (x < 0) {
        x = 0;
    }
    if (x + width > GetScreenWidth()) {
        x = GetScreenWidth() - width;
    }
}

void Game::Paddle::draw() {
    DrawRectangle(static_cast<int>(x), static_cast<int>(y), 
                 static_cast<int>(width), static_cast<int>(height), 
                 BLUE);  // Using BLUE color for the paddle
}

Rectangle Game::Paddle::getRect() const {
    return Rectangle{x, y, width, height};
}

void Game::Paddle::setX(float newX) {
    x = newX;
}

// Ball implementation
Game::Ball::Ball(float x, float y, float radius, float speedX, float speedY)
    : x(x), y(y), radius(radius), speedX(speedX), speedY(speedY) {}

void Game::Ball::update(float deltaTime) {
    x += speedX * deltaTime;
    y += speedY * deltaTime;

    // Bounce off screen edges
    if (x - radius < 0) {
        x = radius;  // Prevent sticking to the edge
        reverseX();
    }
    if (x + radius > GetScreenWidth()) {
        x = GetScreenWidth() - radius;  // Prevent sticking to the edge
        reverseX();
    }
    if (y - radius < 0) {
        y = radius;  // Prevent sticking to the edge
        reverseY();
    }
}

void Game::Ball::draw() {
    DrawCircle(static_cast<int>(x), static_cast<int>(y), radius, WHITE);
}

Vector2 Game::Ball::getPosition() const {
    return Vector2{x, y};
}

float Game::Ball::getRadius() const {
    return radius;
}

void Game::Ball::setPosition(float newX, float newY) {
    x = newX;
    y = newY;
}

void Game::Ball::reverseX() {
    speedX = -speedX;
}

void Game::Ball::reverseY() {
    speedY = -speedY;
}

// Brick implementation
Game::Brick::Brick(float x, float y, float width, float height, bool isAlive)
    : x(x), y(y), width(width), height(height), alive(isAlive) {}

void Game::Brick::draw() {
    if (alive) {
        DrawRectangle(static_cast<int>(x), static_cast<int>(y), 
                     static_cast<int>(width), static_cast<int>(height), 
                     color);  // Use the brick's color
    }
}

Rectangle Game::Brick::getRect() const {
    return Rectangle{x, y, width, height};
}

bool Game::Brick::isAlive() const {
    return alive;
}

void Game::Brick::destroy() {
    alive = false;
}

void Game::Brick::setColor(Color c) {
    color = c;
}

// Add this new method before Game implementation
void Game::initializeBricks() {
    const int rows = 8;
    const int cols = 14;
    const float brickSpacing = 2.0f;
    const float totalSpacing = brickSpacing * (cols + 1);
    const float brickWidth = (GetScreenWidth() - totalSpacing) / cols;
    const float brickHeight = 20.0f;

    bricks.clear();
    bricks.resize(rows);
    for (int i = 0; i < rows; i++) {
        bricks[i].reserve(cols);
        for (int j = 0; j < cols; j++) {
            float x = brickSpacing + j * (brickWidth + brickSpacing);
            float y = brickSpacing + i * (brickHeight + brickSpacing) + 50.0f;
            auto brick = std::make_unique<Brick>(x, y, brickWidth, brickHeight, true);
            
            // Set different colors for each pair of rows (from bottom to top)
            Color rowColors[8] = {
                GREEN,      // Row 1 (bottom)
                GREEN,      // Row 2
                YELLOW,     // Row 3
                YELLOW,     // Row 4
                ORANGE,     // Row 5
                ORANGE,     // Row 6
                RED,        // Row 7
                RED         // Row 8 (top)
            };
            brick->setColor(rowColors[i]);
            
            bricks[i].push_back(std::move(brick));
        }
    }
}

// Game implementation
Game::Game() {
    const float paddleWidth = 100.0f;
    const float paddleHeight = 20.0f;
    const float paddleY = GetScreenHeight() - 40.0f;
    const float ballRadius = 10.0f;

    paddle = std::make_unique<Paddle>(
        (GetScreenWidth() - paddleWidth) / 2,
        paddleY,
        paddleWidth,
        paddleHeight,
        500.0f
    );

    ball = std::make_unique<Ball>(
        GetScreenWidth() / 2,
        paddleY - ballRadius,
        ballRadius,
        300.0f,
        -300.0f
    );

    initializeBricks();

    state = GameState::START_SCREEN;
    gameOver = false;
    won = false;
    score = 0;
    lives = INITIAL_LIVES;
}

void Game::resetBallAndPaddle() {
    // Reset paddle position
    paddle->setX((GetScreenWidth() - paddle->getRect().width) / 2);
    
    // Reset ball position
    ball->setPosition(
        GetScreenWidth() / 2,
        paddle->getRect().y - ball->getRadius()
    );
}

void Game::update(float deltaTime) {
    // Handle state transitions
    if (IsKeyPressed(KEY_SPACE)) {
        if (state == GameState::START_SCREEN) {
            state = GameState::PLAYING;
        }
        else if (state == GameState::GAME_OVER || state == GameState::WON) {
            reset();
            state = GameState::PLAYING;
        }
    }

    if (IsKeyPressed(KEY_P) && (state == GameState::PLAYING || state == GameState::PAUSED)) {
        state = (state == GameState::PLAYING) ? GameState::PAUSED : GameState::PLAYING;
    }

    // Only update game logic if in PLAYING state
    if (state == GameState::PLAYING) {
        paddle->update(deltaTime);
        ball->update(deltaTime);

        checkPaddleCollision();
        checkBrickCollisions();

        // Check if ball went below paddle
        if (ball->getPosition().y + ball->getRadius() > GetScreenHeight()) {
            lives--;
            if (lives <= 0) {
                state = GameState::GAME_OVER;
                gameOver = true;
            } else {
                resetBallAndPaddle();
            }
        }

        // Check win condition
        bool allBricksDestroyed = true;
        for (const auto& row : bricks) {
            for (const auto& brick : row) {
                if (brick->isAlive()) {
                    allBricksDestroyed = false;
                    break;
                }
            }
            if (!allBricksDestroyed) break;
        }
        if (allBricksDestroyed) {
            state = GameState::WON;
            won = true;
        }
    }
}

void Game::draw() {
    BeginDrawing();
    ClearBackground(BLACK);

    switch (state) {
        case GameState::START_SCREEN: {
            const char* title = "BREAKOUT";
            const char* instructions = "Press SPACE to Start";
            
            int titleFontSize = 60;
            int instructionsFontSize = 30;
            
            int titleWidth = MeasureText(title, titleFontSize);
            int instructionsWidth = MeasureText(instructions, instructionsFontSize);
            
            // Draw title
            DrawText(title, 
                    (GetScreenWidth() - titleWidth) / 2,
                    GetScreenHeight() / 3,
                    titleFontSize, WHITE);
            
            // Draw instructions
            DrawText(instructions,
                    (GetScreenWidth() - instructionsWidth) / 2,
                    GetScreenHeight() / 2,
                    instructionsFontSize, GRAY);
            break;
        }
        
        case GameState::PLAYING:
        case GameState::PAUSED: {
            // Draw game elements
            paddle->draw();
            ball->draw();
            
            for (const auto& row : bricks) {
                for (const auto& brick : row) {
                    brick->draw();
                }
            }

            // Draw score
            DrawText(TextFormat("Score: %d", score), 10, 10, 20, WHITE);
            
            // Draw lives
            DrawText(TextFormat("Lives: %d", lives), GetScreenWidth() - 100, 10, 20, WHITE);

            // If paused, draw pause message
            if (state == GameState::PAUSED) {
                const char* pausedText = "PAUSED";
                int textWidth = MeasureText(pausedText, 40);
                DrawText(pausedText,
                        (GetScreenWidth() - textWidth) / 2,
                        GetScreenHeight() / 2,
                        40, YELLOW);
            }
            break;
        }
        
        case GameState::GAME_OVER: {
            // Draw game elements in background
            paddle->draw();
            ball->draw();
            for (const auto& row : bricks) {
                for (const auto& brick : row) {
                    brick->draw();
                }
            }

            const char* gameOverText = "Game Over! Press SPACE to restart";
            int textWidth = MeasureText(gameOverText, 40);
            DrawText(gameOverText,
                    (GetScreenWidth() - textWidth) / 2,
                    GetScreenHeight() / 2,
                    40, RED);
            break;
        }
        
        case GameState::WON: {
            // Draw game elements in background
            paddle->draw();
            ball->draw();
            for (const auto& row : bricks) {
                for (const auto& brick : row) {
                    brick->draw();
                }
            }

            const char* winText = "You Won! Press SPACE to restart";
            int textWidth = MeasureText(winText, 40);
            DrawText(winText,
                    (GetScreenWidth() - textWidth) / 2,
                    GetScreenHeight() / 2,
                    40, GREEN);
            break;
        }
    }

    EndDrawing();
}

void Game::reset() {
    // Reset game state
    state = GameState::PLAYING;
    gameOver = false;
    won = false;
    score = 0;
    lives = INITIAL_LIVES;
    
    // Reset paddle and ball positions
    resetBallAndPaddle();

    // Reinitialize all bricks
    initializeBricks();
}

void Game::checkPaddleCollision() {
    Vector2 ballPos = ball->getPosition();
    float ballRadius = ball->getRadius();
    Rectangle paddleRect = paddle->getRect();

    if (CheckCollisionCircleRec(ballPos, ballRadius, paddleRect)) {
        // Move ball above paddle to prevent sticking
        ball->setPosition(ballPos.x, paddleRect.y - ballRadius);
        
        // Reverse vertical direction
        ball->reverseY();

        // Optional: Add some horizontal influence based on where the ball hits the paddle
        float hitPosition = (ballPos.x - paddleRect.x) / paddleRect.width;
        if (hitPosition < 0.33f) {
            // Hit left side - add leftward influence
            ball->reverseX();
        } else if (hitPosition > 0.66f) {
            // Hit right side - add rightward influence
            ball->reverseX();
        }
    }
}

bool Game::checkBallBrickCollision(const Rectangle& brickRect) {
    Vector2 ballPos = ball->getPosition();
    float ballRadius = ball->getRadius();

    if (CheckCollisionCircleRec(ballPos, ballRadius, brickRect)) {
        // Calculate collision point relative to brick center
        float brickCenterX = brickRect.x + brickRect.width / 2.0f;
        float brickCenterY = brickRect.y + brickRect.height / 2.0f;
        
        // Calculate collision angle
        float dx = ballPos.x - brickCenterX;
        float dy = ballPos.y - brickCenterY;
        
        // Determine if collision is more horizontal or vertical
        if (std::abs(dx) * brickRect.height > std::abs(dy) * brickRect.width) {
            // Horizontal collision (sides)
            ball->reverseX();
        } else {
            // Vertical collision (top/bottom)
            ball->reverseY();
        }
        
        return true;
    }
    return false;
}

void Game::checkBrickCollisions() {
    for (auto& row : bricks) {
        for (auto& brick : row) {
            if (brick && brick->isAlive()) {
                if (checkBallBrickCollision(brick->getRect())) {
                    brick->destroy();
                    score += 100;  // Increment score when brick is destroyed
                    // Only handle one collision per frame to prevent multiple bounces
                    return;
                }
            }
        }
    }
}

void Game::run() {
    update(GetFrameTime());
    draw();
} 