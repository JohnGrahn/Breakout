#include "../include/game.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

// Initialize static members
float Game::SpeedConfig::VIRTUAL_WIDTH = 800.0f;
float Game::SpeedConfig::VIRTUAL_HEIGHT = 600.0f;

Game::~Game() = default;

// Paddle implementation
Game::Paddle::Paddle(float x, float y, float width, float height, float speed)
    : x(x), y(y), width(width), height(height), baseSpeed(speed),
      baseWidth(width), baseHeight(height) {}

void Game::Paddle::update(float deltaTime) {
    float scaledSpeed = baseSpeed * SpeedConfig::getWidthScale();
    if (IsKeyDown(KEY_LEFT)) {
        x -= scaledSpeed * deltaTime;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        x += scaledSpeed * deltaTime;
    }
    
    clampToScreen();
}

void Game::Paddle::updateDimensions() {
    width = baseWidth * SpeedConfig::getWidthScale();
    height = baseHeight * SpeedConfig::getHeightScale();
    clampToScreen();
}

void Game::Paddle::clampToScreen() {
    x = std::max(0.0f, std::min(x, SpeedConfig::VIRTUAL_WIDTH - width));
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
    : x(x), y(y), radius(radius), baseRadius(radius),
      baseSpeedX(speedX), baseSpeedY(speedY), spin(0.0f) {}

void Game::Ball::updateDimensions() {
    // Use the average of width and height scale for the radius
    float scale = (SpeedConfig::getWidthScale() + SpeedConfig::getHeightScale()) * 0.5f;
    radius = baseRadius * scale;
    clampToScreen();
}

void Game::Ball::update(float deltaTime) {
    float widthScale = SpeedConfig::getWidthScale();
    float heightScale = SpeedConfig::getHeightScale();
    
    // Apply spin influence to the velocity
    float spinInfluence = spin * SPIN_INFLUENCE;
    x += (baseSpeedX + baseSpeedX * spinInfluence) * widthScale * deltaTime;
    y += baseSpeedY * heightScale * deltaTime;
    
    // Decay spin over time
    applySpinDecay(deltaTime);
    clampToScreen();
}

void Game::Ball::clampToScreen() {
    // Bounce off screen edges
    if (x - radius < 0) {
        x = radius;
        reverseX();
    }
    if (x + radius > SpeedConfig::VIRTUAL_WIDTH) {
        x = SpeedConfig::VIRTUAL_WIDTH - radius;
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

void Game::Ball::setVelocity(float angleInRadians, float speed) {
    baseSpeedX = speed * cos(angleInRadians);
    baseSpeedY = speed * sin(angleInRadians);
}

void Game::Ball::addSpin(float spinValue) {
    spin = std::clamp(spin + spinValue, -MAX_SPIN, MAX_SPIN);
}

void Game::Ball::applySpinDecay(float deltaTime) {
    if (spin > 0.0f) {
        spin = std::max(0.0f, spin - SPIN_DECAY * deltaTime);
    } else if (spin < 0.0f) {
        spin = std::min(0.0f, spin + SPIN_DECAY * deltaTime);
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
    
    const float brickSpacing = SpeedConfig::VIRTUAL_WIDTH * 0.003f;
    const float totalSpacing = brickSpacing * (cols + 1);
    const float brickWidth = (SpeedConfig::VIRTUAL_WIDTH - totalSpacing) / cols;
    const float brickHeight = SpeedConfig::VIRTUAL_HEIGHT * 0.033f;

    bricks.clear();
    bricks.resize(rows);
    
    for (int i = 0; i < rows; i++) {
        bricks[i].reserve(cols);
        for (int j = 0; j < cols; j++) {
            float x = brickSpacing + j * (brickWidth + brickSpacing);
            float y = brickSpacing + i * (brickHeight + brickSpacing) + (SpeedConfig::VIRTUAL_HEIGHT * 0.083f);
            
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
    SpeedConfig::updateVirtualDimensions();
    
    // Initialize paddle with dimensions relative to base window size
    const float paddleWidth = SpeedConfig::BASE_WINDOW_WIDTH * 0.125f;
    const float paddleHeight = SpeedConfig::BASE_WINDOW_HEIGHT * 0.033f;
    const float paddleY = SpeedConfig::VIRTUAL_HEIGHT * 0.9f;
    
    paddle = std::make_unique<Paddle>(
        (SpeedConfig::VIRTUAL_WIDTH - paddleWidth * SpeedConfig::getWidthScale()) / 2,
        paddleY,
        paddleWidth,
        paddleHeight,
        SpeedConfig::PADDLE_BASE_SPEED
    );

    // Initialize ball with radius relative to base window size
    const float ballRadius = SpeedConfig::BASE_WINDOW_WIDTH * 0.0125f;
    
    ball = std::make_unique<Ball>(
        SpeedConfig::VIRTUAL_WIDTH / 2,
        paddleY - ballRadius * SpeedConfig::getHeightScale(),
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

    updateCamera();
}

void Game::updateCamera() {
    // Update virtual dimensions
    SpeedConfig::updateVirtualDimensions();
    
    // Update game objects with new dimensions
    if (paddle) {
        paddle->updateDimensions();
    }
    if (ball) {
        ball->updateDimensions();
    }
    
    // Update brick positions and sizes without reinitializing
    const int rows = 8;
    const int cols = 14;
    
    const float brickSpacing = SpeedConfig::VIRTUAL_WIDTH * 0.003f;
    const float totalSpacing = brickSpacing * (cols + 1);
    const float brickWidth = (SpeedConfig::VIRTUAL_WIDTH - totalSpacing) / cols;
    const float brickHeight = SpeedConfig::VIRTUAL_HEIGHT * 0.033f;

    for (int i = 0; i < rows && i < bricks.size(); i++) {
        for (int j = 0; j < cols && j < bricks[i].size(); j++) {
            if (bricks[i][j]) {
                float x = brickSpacing + j * (brickWidth + brickSpacing);
                float y = brickSpacing + i * (brickHeight + brickSpacing) + (SpeedConfig::VIRTUAL_HEIGHT * 0.083f);
                
                // Store the current brick's state before replacing it
                bool isAlive = bricks[i][j]->isAlive();
                Color color = bricks[i][j]->getColor();
                
                // Create new brick with preserved state
                auto newBrick = std::make_unique<Brick>(x, y, brickWidth, brickHeight, isAlive);
                newBrick->setColor(color);
                
                // Replace the old brick
                bricks[i][j] = std::move(newBrick);
            }
        }
    }
    
    // No need for camera scaling since we're using screen coordinates directly
    camera.offset = Vector2{0, 0};
    camera.target = Vector2{0, 0};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
}

void Game::resetBallAndPaddle() {
    // Use base window dimensions for consistent sizing
    const float paddleWidth = SpeedConfig::BASE_WINDOW_WIDTH * 0.125f;
    const float paddleHeight = SpeedConfig::BASE_WINDOW_HEIGHT * 0.033f;
    const float paddleY = SpeedConfig::VIRTUAL_HEIGHT * 0.9f;
    
    paddle = std::make_unique<Paddle>(
        (SpeedConfig::VIRTUAL_WIDTH - paddleWidth * SpeedConfig::getWidthScale()) / 2,
        paddleY,
        paddleWidth,
        paddleHeight,
        SpeedConfig::PADDLE_BASE_SPEED
    );
    
    // Use base window dimensions for consistent ball sizing
    const float ballRadius = SpeedConfig::BASE_WINDOW_WIDTH * 0.0125f;
    ball = std::make_unique<Ball>(
        SpeedConfig::VIRTUAL_WIDTH / 2,
        paddleY - ballRadius * SpeedConfig::getHeightScale(),
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
        
        // Calculate hit position relative to paddle center (-1 to 1)
        float hitPosition = (ballPos.x - (paddleRect.x + paddleRect.width / 2)) / (paddleRect.width / 2);
        
        // Calculate reflection angle based on hit position
        float baseAngle = -PI / 2;  // Straight up
        float maxAngleOffset = PI / 3;  // 60 degrees max deflection
        float angle = baseAngle + (hitPosition * maxAngleOffset);
        
        // Calculate speed based on current ball speed
        float currentSpeed = sqrt(ball->getSpeedX() * ball->getSpeedX() + 
                                ball->getSpeedY() * ball->getSpeedY());
        
        // Set new velocity based on calculated angle
        ball->setVelocity(angle, currentSpeed);
        
        // Add spin based on hit position and current paddle movement
        float spinFactor = hitPosition;  // -1 to 1 based on hit position
        if (IsKeyDown(KEY_LEFT)) spinFactor -= 0.5f;
        if (IsKeyDown(KEY_RIGHT)) spinFactor += 0.5f;
        ball->addSpin(spinFactor * 0.5f);
        
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
        
        // Calculate normalized collision angle
        float angle = atan2(dy, dx);
        
        // Calculate current ball speed
        float currentSpeed = sqrt(ball->getSpeedX() * ball->getSpeedX() + 
                                ball->getSpeedY() * ball->getSpeedY());
        
        // Add slight randomization to prevent chain reactions (±5 degrees)
        float randomAngle = angle + (((float)rand() / RAND_MAX) * 0.174533f - 0.0872665f);
        
        // Determine if this is a corner collision
        bool isCornerCollision = (fabs(dx) > brickRect.width * 0.4f && 
                                fabs(dy) > brickRect.height * 0.4f);
        
        if (isCornerCollision) {
            // For corner collisions, reflect based on the collision angle
            ball->setVelocity(randomAngle, currentSpeed);
            
            // Add slight spin based on which corner was hit
            float spinFactor = (dx > 0) ? 0.2f : -0.2f;
            ball->addSpin(spinFactor);
        } else {
            // For edge collisions, use improved reflection
            if (fabs(dx) * brickRect.height > fabs(dy) * brickRect.width) {
                ball->reverseX();
                // Add spin based on the vertical position of the hit
                float spinFactor = (dy > 0) ? 0.1f : -0.1f;
                ball->addSpin(spinFactor);
            } else {
                ball->reverseY();
                // Add spin based on the horizontal position of the hit
                float spinFactor = (dx > 0) ? -0.1f : 0.1f;
                ball->addSpin(spinFactor);
            }
        }
        
        validateGameObjects();
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
        validateGameObjects();

        ballSpeedTimer += deltaTime;
        if (ballSpeedTimer >= SPEED_INCREASE_INTERVAL) {
            ball->increaseSpeed(BALL_SPEED_INCREMENT);
            ball->clampSpeed(MAX_BALL_SPEED);
            ballSpeedTimer = 0.0f;
        }

        checkPaddleCollision();
        checkBrickCollisions();

        if (ball->getPosition().y + ball->getRadius() > SpeedConfig::VIRTUAL_HEIGHT) {
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

    // Update camera before drawing
    updateCamera();
    BeginMode2D(camera);

    // Calculate font sizes relative to screen height with a maximum size
    const float maxFontSize = SpeedConfig::VIRTUAL_HEIGHT * 0.067f;
    const float fontSize = std::min(maxFontSize, SpeedConfig::VIRTUAL_HEIGHT * 0.067f);
    const float smallFontSize = std::min(maxFontSize * 0.5f, SpeedConfig::VIRTUAL_HEIGHT * 0.033f);
    
    // Calculate base text size for score and lives
    const float baseTextSize = SpeedConfig::BASE_WINDOW_HEIGHT * 0.033f;
    const float scaledTextSize = baseTextSize * SpeedConfig::getHeightScale();
    const float maxHUDTextSize = SpeedConfig::VIRTUAL_HEIGHT * 0.05f;  // Maximum 5% of screen height
    const float hudTextSize = std::min(scaledTextSize, maxHUDTextSize);

    switch (state) {
        case GameState::START_SCREEN: {
            const char* title = "BREAKOUT";
            const char* instructions = "Press SPACE to Start";

            // Ensure text fits within screen width
            float titleScale = 1.0f;
            int titleWidth = MeasureText(title, fontSize);
            if (titleWidth > SpeedConfig::VIRTUAL_WIDTH * 0.8f) {
                titleScale = (SpeedConfig::VIRTUAL_WIDTH * 0.8f) / titleWidth;
            }

            DrawText(title,
                    (SpeedConfig::VIRTUAL_WIDTH - titleWidth * titleScale) / 2,
                    SpeedConfig::VIRTUAL_HEIGHT / 3,
                    fontSize * titleScale, WHITE);

            int instructionsWidth = MeasureText(instructions, smallFontSize);
            DrawText(instructions,
                    (SpeedConfig::VIRTUAL_WIDTH - instructionsWidth) / 2,
                    SpeedConfig::VIRTUAL_HEIGHT / 2,
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

            // Draw score and lives with padding from screen edges
            const float edgePadding = SpeedConfig::VIRTUAL_WIDTH * 0.02f;
            const char* scoreText = TextFormat("Score: %d", score);
            const char* livesText = TextFormat("Lives: %d", lives);
            
            // Calculate text widths for positioning
            int scoreWidth = MeasureText(scoreText, hudTextSize);
            int livesWidth = MeasureText(livesText, hudTextSize);
            
            // Ensure text doesn't overlap by adjusting position if needed
            float scoreX = edgePadding;
            float livesX = SpeedConfig::VIRTUAL_WIDTH - livesWidth - edgePadding;
            
            // If texts would overlap, adjust positions
            float minSpacing = SpeedConfig::VIRTUAL_WIDTH * 0.05f;  // Minimum 5% screen width between texts
            if (scoreX + scoreWidth + minSpacing > livesX) {
                // Center both texts with spacing between them
                float totalWidth = scoreWidth + minSpacing + livesWidth;
                scoreX = (SpeedConfig::VIRTUAL_WIDTH - totalWidth) / 2;
                livesX = scoreX + scoreWidth + minSpacing;
            }
            
            DrawText(scoreText, scoreX, edgePadding, hudTextSize, WHITE);
            DrawText(livesText, livesX, edgePadding, hudTextSize, WHITE);

            if (state == GameState::PAUSED) {
                const char* pausedText = "PAUSED";
                float textScale = 1.0f;
                int textWidth = MeasureText(pausedText, fontSize);
                if (textWidth > SpeedConfig::VIRTUAL_WIDTH * 0.8f) {
                    textScale = (SpeedConfig::VIRTUAL_WIDTH * 0.8f) / textWidth;
                }

                DrawText(pausedText,
                        (SpeedConfig::VIRTUAL_WIDTH - textWidth * textScale) / 2,
                        SpeedConfig::VIRTUAL_HEIGHT / 2,
                        fontSize * textScale, YELLOW);
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

            // Calculate text scale to ensure it fits
            float textScale = 1.0f;
            int textWidth = MeasureText(text, fontSize);
            if (textWidth > SpeedConfig::VIRTUAL_WIDTH * 0.8f) {
                textScale = (SpeedConfig::VIRTUAL_WIDTH * 0.8f) / textWidth;
            }

            DrawText(text,
                    (SpeedConfig::VIRTUAL_WIDTH - textWidth * textScale) / 2,
                    SpeedConfig::VIRTUAL_HEIGHT / 2,
                    fontSize * textScale,
                    state == GameState::GAME_OVER ? RED : GREEN);
            break;
        }
    }

    EndMode2D();
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
