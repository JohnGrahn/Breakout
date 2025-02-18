#include "../include/game.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

Game::~Game() = default;

// Paddle implementation
Game::Paddle::Paddle(float x, float y, float width, float height, float speed)
    : x(x), y(y), width(width), height(height), baseSpeed(speed) {}

void Game::Paddle::update(float deltaTime) {
    float speedScale = SpeedConfig::getSpeedScale();
    float scaledSpeed = baseSpeed * speedScale;
    
    if (IsKeyDown(KEY_LEFT)) {
        x -= scaledSpeed * deltaTime;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        x += scaledSpeed * deltaTime;
    }
    
    clampToScreen();
}

void Game::Paddle::clampToScreen() {
    x = std::max(0.0f, std::min(x, static_cast<float>(GetScreenWidth() - width)));
}

void Game::Paddle::draw() {
    DrawRectangle(static_cast<int>(x), static_cast<int>(y), 
                 static_cast<int>(width), static_cast<int>(height), 
                 BLUE);
}

Rectangle Game::Paddle::getRect() const {
    return Rectangle{x, y, width, height};
}

void Game::Paddle::setX(float newX) {
    x = newX;
    clampToScreen();
}

// Ball implementation
Game::Ball::Ball(float x, float y, float radius, float speedX, float speedY)
    : x(x), y(y), radius(radius), baseSpeedX(speedX), baseSpeedY(speedY) {}

void Game::Ball::update(float deltaTime) {
    float speedScale = SpeedConfig::getSpeedScale();
    x += baseSpeedX * deltaTime * speedScale;
    y += baseSpeedY * deltaTime * speedScale;
    clampToScreen();
}

void Game::Ball::clampToScreen() {
    // Bounce off screen edges
    if (x - radius < 0) {
        x = radius;
        reverseX();
    }
    if (x + radius > GetScreenWidth()) {
        x = GetScreenWidth() - radius;
        reverseX();
    }
    if (y - radius < 0) {
        y = radius;
        reverseY();
    }
    // Don't clamp bottom edge - that's for life loss detection
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
    clampToScreen();
}

void Game::Ball::reverseX() {
    baseSpeedX = -baseSpeedX;
}

void Game::Ball::reverseY() {
    baseSpeedY = -baseSpeedY;
}

void Game::Ball::increaseSpeed(float increment) {
    if (baseSpeedX > 0) baseSpeedX += increment;
    else baseSpeedX -= increment;
    if (baseSpeedY > 0) baseSpeedY += increment;
    else baseSpeedY -= increment;
}

void Game::Ball::setSpeed(float newSpeedX, float newSpeedY) {
    baseSpeedX = newSpeedX;
    baseSpeedY = newSpeedY;
}

void Game::Ball::clampSpeed(float maxSpeed) {
    float currentSpeed = sqrt(baseSpeedX * baseSpeedX + baseSpeedY * baseSpeedY);
    if (currentSpeed > maxSpeed) {
        float scale = maxSpeed / currentSpeed;
        baseSpeedX *= scale;
        baseSpeedY *= scale;
    }
}

// Brick implementation remains the same...
Game::Brick::Brick(float x, float y, float width, float height, bool isAlive)
    : x(x), y(y), width(width), height(height), alive(isAlive) {}

void Game::Brick::draw() {
    if (alive) {
        DrawRectangle(static_cast<int>(x), static_cast<int>(y), 
                     static_cast<int>(width), static_cast<int>(height), 
                     color);
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

// Game implementation
void Game::initializeBricks() {
    const int rows = 8;
    const int cols = 14;
    const float screenWidth = static_cast<float>(GetScreenWidth());
    const float screenHeight = static_cast<float>(GetScreenHeight());
    
    const float brickSpacing = screenWidth * 0.003f;
    const float totalSpacing = brickSpacing * (cols + 1);
    const float brickWidth = (screenWidth - totalSpacing) / cols;
    const float brickHeight = screenHeight * 0.033f;

    bricks.clear();
    bricks.resize(rows);
    
    for (int i = 0; i < rows; i++) {
        bricks[i].reserve(cols);
        for (int j = 0; j < cols; j++) {
            float x = brickSpacing + j * (brickWidth + brickSpacing);
            float y = brickSpacing + i * (brickHeight + brickSpacing) + (screenHeight * 0.083f);
            
            auto brick = std::make_unique<Brick>(x, y, brickWidth, brickHeight, true);
            
            Color rowColors[8] = {
                GREEN, GREEN,     // Bottom rows
                YELLOW, YELLOW,   // Middle rows
                ORANGE, ORANGE,   // Upper middle rows
                RED, RED         // Top rows
            };
            brick->setColor(rowColors[i]);
            
            bricks[i].push_back(std::move(brick));
        }
    }
}

Game::Game() : ballSpeedTimer(0.0f) {
    const float screenWidth = static_cast<float>(GetScreenWidth());
    const float screenHeight = static_cast<float>(GetScreenHeight());
    
    // Initialize paddle with relative dimensions
    const float paddleWidth = screenWidth * 0.125f;
    const float paddleHeight = screenHeight * 0.033f;
    const float paddleY = screenHeight * 0.9f;
    
    paddle = std::make_unique<Paddle>(
        (screenWidth - paddleWidth) / 2,
        paddleY,
        paddleWidth,
        paddleHeight,
        SpeedConfig::PADDLE_BASE_SPEED
    );

    // Initialize ball with relative dimensions
    const float ballRadius = screenWidth * 0.0125f;
    
    ball = std::make_unique<Ball>(
        screenWidth / 2,
        paddleY - ballRadius,
        ballRadius,
        SpeedConfig::BALL_BASE_SPEED,
        -SpeedConfig::BALL_BASE_SPEED
    );

    initializeBricks();
    
    state = GameState::START_SCREEN;
    gameOver = false;
    won = false;
    score = 0;
    lives = INITIAL_LIVES;
}

void Game::resetBallAndPaddle() {
    const float screenWidth = static_cast<float>(GetScreenWidth());
    const float screenHeight = static_cast<float>(GetScreenHeight());
    
    // Update paddle width and position
    const float paddleWidth = screenWidth * 0.125f;
    const float paddleHeight = screenHeight * 0.033f;
    const float paddleY = screenHeight * 0.9f;
    
    paddle = std::make_unique<Paddle>(
        (screenWidth - paddleWidth) / 2,
        paddleY,
        paddleWidth,
        paddleHeight,
        SpeedConfig::PADDLE_BASE_SPEED
    );
    
    // Update ball radius and position
    const float ballRadius = screenWidth * 0.0125f;
    ball = std::make_unique<Ball>(
        screenWidth / 2,
        paddleY - ballRadius,
        ballRadius,
        SpeedConfig::BALL_BASE_SPEED,
        -SpeedConfig::BALL_BASE_SPEED
    );
    
    ballSpeedTimer = 0.0f;
}

void Game::validateGameObjects() {
    if (paddle) {
        paddle->clampToScreen();
    }
    if (ball) {
        ball->clampToScreen();
    }
}

void Game::checkPaddleCollision() {
    Vector2 ballPos = ball->getPosition();
    float ballRadius = ball->getRadius();
    Rectangle paddleRect = paddle->getRect();

    if (CheckCollisionCircleRec(ballPos, ballRadius, paddleRect)) {
        // Move ball above paddle to prevent sticking
        ball->setPosition(ballPos.x, paddleRect.y - ballRadius);
        
        // Calculate bounce angle based on hit position
        float hitPosition = (ballPos.x - paddleRect.x) / paddleRect.width;
        
        // Use base speeds for bounce (scaling will be applied in update)
        if (hitPosition < 0.33f) {
            ball->setSpeed(-SpeedConfig::BALL_BASE_SPEED, -SpeedConfig::BALL_BASE_SPEED);
        } else if (hitPosition > 0.66f) {
            ball->setSpeed(SpeedConfig::BALL_BASE_SPEED, -SpeedConfig::BALL_BASE_SPEED);
        } else {
            // Middle hit - maintain current direction but use base speed
            float currentSpeed = ball->getSpeedX();
            float direction = currentSpeed > 0 ? 1.0f : -1.0f;
            ball->setSpeed(SpeedConfig::BALL_BASE_SPEED * direction, -SpeedConfig::BALL_BASE_SPEED);
        }
        
        validateGameObjects();
    }
}

void Game::checkBrickCollisions() {
    for (auto& row : bricks) {
        for (auto& brick : row) {
            if (brick && brick->isAlive()) {
                if (checkBallBrickCollision(brick->getRect())) {
                    brick->destroy();
                    score += 100;
                    validateGameObjects();  // Ensure ball stays in bounds after collision
                    return;
                }
            }
        }
    }
}

bool Game::checkBallBrickCollision(const Rectangle& brickRect) {
    Vector2 ballPos = ball->getPosition();
    float ballRadius = ball->getRadius();

    if (CheckCollisionCircleRec(ballPos, ballRadius, brickRect)) {
        float brickCenterX = brickRect.x + brickRect.width / 2.0f;
        float brickCenterY = brickRect.y + brickRect.height / 2.0f;
        
        float dx = ballPos.x - brickCenterX;
        float dy = ballPos.y - brickCenterY;
        
        if (fabs(dx) * brickRect.height > fabs(dy) * brickRect.width) {
            ball->reverseX();
        } else {
            ball->reverseY();
        }
        
        validateGameObjects();  // Ensure ball stays in bounds after collision
        return true;
    }
    return false;
}

void Game::update(float deltaTime) {
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

    if (state == GameState::PLAYING) {
        paddle->update(deltaTime);
        ball->update(deltaTime);
        validateGameObjects();  // Validate after movement

        ballSpeedTimer += deltaTime;
        if (ballSpeedTimer >= SPEED_INCREASE_INTERVAL) {
            ball->increaseSpeed(BALL_SPEED_INCREMENT);
            ball->clampSpeed(MAX_BALL_SPEED);
            ballSpeedTimer = 0.0f;
        }

        checkPaddleCollision();
        checkBrickCollisions();

        if (ball->getPosition().y + ball->getRadius() > GetScreenHeight()) {
            lives--;
            if (lives <= 0) {
                state = GameState::GAME_OVER;
                gameOver = true;
            } else {
                resetBallAndPaddle();
            }
        }

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

    const float screenWidth = static_cast<float>(GetScreenWidth());
    const float screenHeight = static_cast<float>(GetScreenHeight());
    const float fontSize = screenHeight * 0.067f;
    const float smallFontSize = screenHeight * 0.033f;

    switch (state) {
        case GameState::START_SCREEN: {
            const char* title = "BREAKOUT";
            const char* instructions = "Press SPACE to Start";
            
            int titleWidth = MeasureText(title, fontSize);
            int instructionsWidth = MeasureText(instructions, smallFontSize);
            
            DrawText(title, 
                    (screenWidth - titleWidth) / 2,
                    screenHeight / 3,
                    fontSize, WHITE);
            
            DrawText(instructions,
                    (screenWidth - instructionsWidth) / 2,
                    screenHeight / 2,
                    smallFontSize, GRAY);
            break;
        }
        
        case GameState::PLAYING:
        case GameState::PAUSED: {
            paddle->draw();
            ball->draw();
            
            for (const auto& row : bricks) {
                for (const auto& brick : row) {
                    brick->draw();
                }
            }

            DrawText(TextFormat("Score: %d", score),
                    10,
                    10,
                    smallFontSize, WHITE);
            
            DrawText(TextFormat("Lives: %d", lives),
                    screenWidth - MeasureText(TextFormat("Lives: %d", lives), smallFontSize) - 10,
                    10,
                    smallFontSize, WHITE);

            if (state == GameState::PAUSED) {
                const char* pausedText = "PAUSED";
                int textWidth = MeasureText(pausedText, fontSize);
                DrawText(pausedText,
                        (screenWidth - textWidth) / 2,
                        screenHeight / 2,
                        fontSize, YELLOW);
            }
            break;
        }
        
        case GameState::GAME_OVER:
        case GameState::WON: {
            paddle->draw();
            ball->draw();
            for (const auto& row : bricks) {
                for (const auto& brick : row) {
                    brick->draw();
                }
            }

            const char* text = state == GameState::GAME_OVER ?
                "Game Over! Press SPACE to restart" :
                "You Won! Press SPACE to restart";
            
            int textWidth = MeasureText(text, fontSize);
            DrawText(text,
                    (screenWidth - textWidth) / 2,
                    screenHeight / 2,
                    fontSize,
                    state == GameState::GAME_OVER ? RED : GREEN);
            break;
        }
    }

    EndDrawing();
}

void Game::reset() {
    state = GameState::PLAYING;
    gameOver = false;
    won = false;
    score = 0;
    lives = INITIAL_LIVES;
    
    resetBallAndPaddle();
    initializeBricks();
}

void Game::run() {
    update(GetFrameTime());
    draw();
}
