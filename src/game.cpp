#include "../include/game.h"

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
    DrawCircle(static_cast<int>(x), static_cast<int>(y), radius, RED);
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
                     GREEN);  // Using GREEN color for bricks
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

// Game implementation
Game::Game() {
    // Initialize paddle with reasonable values:
    // Position it at the bottom of the screen, centered horizontally
    const float paddleWidth = 100.0f;
    const float paddleHeight = 20.0f;
    const float paddleSpeed = 500.0f;  // pixels per second
    const float paddleY = GetScreenHeight() - 40.0f;  // 40 pixels from bottom
    const float paddleX = (GetScreenWidth() - paddleWidth) / 2.0f;

    paddle = std::make_unique<Paddle>(paddleX, paddleY, paddleWidth, paddleHeight, paddleSpeed);

    // Initialize ball in the middle of the screen with reasonable values
    const float ballRadius = 10.0f;
    const float ballSpeedX = 300.0f;  // pixels per second
    const float ballSpeedY = -300.0f;  // negative means moving up
    const float ballX = GetScreenWidth() / 2.0f;
    const float ballY = GetScreenHeight() / 2.0f;

    ball = std::make_unique<Ball>(ballX, ballY, ballRadius, ballSpeedX, ballSpeedY);

    // Initialize brick grid
    const int rows = 5;
    const int cols = 10;
    const float brickWidth = 70.0f;
    const float brickHeight = 30.0f;
    const float padding = 5.0f;  // Space between bricks
    const float offsetX = (GetScreenWidth() - (cols * (brickWidth + padding) - padding)) / 2.0f;
    const float offsetY = 50.0f;  // Start 50 pixels from the top

    // Resize the bricks vector to hold all rows
    bricks.resize(rows);

    // Create the grid of bricks
    for (int row = 0; row < rows; row++) {
        // Resize this row to hold all columns
        bricks[row].resize(cols);

        for (int col = 0; col < cols; col++) {
            float brickX = offsetX + col * (brickWidth + padding);
            float brickY = offsetY + row * (brickHeight + padding);
            bricks[row][col] = std::make_unique<Brick>(brickX, brickY, brickWidth, brickHeight, true);
        }
    }
}

Game::~Game() = default;

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
        if (abs(dx) * brickRect.height > abs(dy) * brickRect.width) {
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
                    // Only handle one collision per frame to prevent multiple bounces
                    return;
                }
            }
        }
    }
}

void Game::update(float deltaTime) {
    paddle->update(deltaTime);
    ball->update(deltaTime);
    
    checkPaddleCollision();
    checkBrickCollisions();
}

void Game::draw() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    // Draw all alive bricks
    for (const auto& row : bricks) {
        for (const auto& brick : row) {
            if (brick && brick->isAlive()) {
                brick->draw();
            }
        }
    }
    
    paddle->draw();
    ball->draw();
    
    EndDrawing();
}

void Game::run() {
    update(GetFrameTime());
    draw();
} 