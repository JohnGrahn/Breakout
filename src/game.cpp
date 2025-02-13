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

// Game implementation
Game::Game() : gameOver(false), won(false), score(0) {
    reset();
}

void Game::reset() {
    gameOver = false;
    won = false;
    score = 0;

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
    const int rows = 8;  // 8 rows to match the image
    const int cols = 14; // 14 columns to match the image
    const float padding = 2.0f;  // Reduced padding between bricks
    
    // Calculate brick dimensions to fit screen width
    const float brickWidth = (GetScreenWidth() - (cols + 1) * padding) / cols;
    const float brickHeight = 20.0f;  // Fixed height for bricks
    
    // Define colors for each row (from bottom to top)
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

    // Resize the bricks vector to hold all rows
    bricks.resize(rows);

    // Create the grid of bricks
    const float startY = 50.0f;  // Start position from top
    
    for (int row = 0; row < rows; row++) {
        bricks[row].resize(cols);
        float currentY = startY + row * (brickHeight + padding);

        for (int col = 0; col < cols; col++) {
            float currentX = padding + col * (brickWidth + padding);
            bricks[row][col] = std::make_unique<Brick>(currentX, currentY, brickWidth, brickHeight, true);
            bricks[row][col]->setColor(rowColors[row]);
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
                    score += 100;  // Increment score when brick is destroyed
                    // Only handle one collision per frame to prevent multiple bounces
                    return;
                }
            }
        }
    }
}

void Game::update(float deltaTime) {
    if (gameOver || won) {
        if (IsKeyPressed(KEY_R)) {
            reset();
        }
        return;
    }

    paddle->update(deltaTime);
    ball->update(deltaTime);
    
    // Check if ball goes below paddle
    if (ball->getPosition().y - ball->getRadius() > GetScreenHeight()) {
        gameOver = true;
        return;
    }

    checkPaddleCollision();
    checkBrickCollisions();

    // Check win condition
    bool allBricksDestroyed = true;
    for (const auto& row : bricks) {
        for (const auto& brick : row) {
            if (brick && brick->isAlive()) {
                allBricksDestroyed = false;
                break;
            }
        }
        if (!allBricksDestroyed) break;
    }
    if (allBricksDestroyed) {
        won = true;
    }
}

void Game::draw() {
    BeginDrawing();
    ClearBackground(BLACK);
    
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

    // Draw score in white
    DrawText(TextFormat("Score: %d", score), 10, 10, 20, WHITE);

    // Draw game over message
    if (gameOver) {
        const char* gameOverText = "Game Over! Press R to restart";
        int fontSize = 40;
        Vector2 textSize = MeasureTextEx(GetFontDefault(), gameOverText, fontSize, 1);
        DrawText(gameOverText, 
                (GetScreenWidth() - textSize.x) / 2,
                (GetScreenHeight() - textSize.y) / 2,
                fontSize, RED);
    }

    // Draw win message
    if (won) {
        const char* winText = "You Win! Press R to restart";
        int fontSize = 40;
        Vector2 textSize = MeasureTextEx(GetFontDefault(), winText, fontSize, 1);
        DrawText(winText,
                (GetScreenWidth() - textSize.x) / 2,
                (GetScreenHeight() - textSize.y) / 2,
                fontSize, GREEN);
    }
    
    EndDrawing();
}

void Game::run() {
    update(GetFrameTime());
    draw();
} 