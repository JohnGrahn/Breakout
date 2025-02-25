#ifndef GAME_H
#define GAME_H

#include <raylib.h>
#include <memory>
#include <vector>
#include <algorithm>

class Game {
public:
    struct SpeedConfig {
        // Base dimensions and speeds (unchanged by scaling)
        static constexpr float BASE_WINDOW_WIDTH = 800.0f;
        static constexpr float BASE_WINDOW_HEIGHT = 600.0f;
        static constexpr float PADDLE_BASE_SPEED = 500.0f;
        static constexpr float BALL_BASE_SPEED = 300.0f;
        static constexpr float BALL_SPEED_INCREMENT = 10.0f;
        
        // Dynamic virtual dimensions that change with screen size
        static float VIRTUAL_WIDTH;
        static float VIRTUAL_HEIGHT;

        // Get scale factors
        static float getWidthScale() {
            return VIRTUAL_WIDTH / BASE_WINDOW_WIDTH;
        }
        
        static float getHeightScale() {
            return VIRTUAL_HEIGHT / BASE_WINDOW_HEIGHT;
        }
        
        static void updateVirtualDimensions() {
            VIRTUAL_WIDTH = static_cast<float>(GetScreenWidth());
            VIRTUAL_HEIGHT = static_cast<float>(GetScreenHeight());
        }
    };

    class Paddle {
    public:
        Paddle(float x, float y, float width, float height, float speed);
        void update(float deltaTime);
        void draw();
        Rectangle getRect() const;
        void setX(float newX);
        void clampToScreen();
        float getBaseSpeed() const { return baseSpeed; }
        void updateDimensions();  // New method to update dimensions when screen changes

    private:
        float x;
        float y;
        float width;
        float height;
        float baseSpeed;
        float baseWidth;  // Store original width for scaling
        float baseHeight; // Store original height for scaling
    };

    class Ball {
    public:
        Ball(float x, float y, float radius, float speedX, float speedY);
        void update(float deltaTime);
        void draw();
        Vector2 getPosition() const;
        float getRadius() const;
        void setPosition(float x, float y);
        void reverseX();
        void reverseY();
        void increaseSpeed(float increment);
        void setSpeed(float newSpeedX, float newSpeedY);
        void clampSpeed(float maxSpeed);
        void clampToScreen();
        float getSpeedX() const { return baseSpeedX; }
        float getSpeedY() const { return baseSpeedY; }
        void updateDimensions();
        void setVelocity(float angleInRadians, float speed);
        void addSpin(float spinValue);
        void applySpinDecay(float deltaTime);

    private:
        float x;
        float y;
        float radius;
        float baseRadius;
        float baseSpeedX;
        float baseSpeedY;
        float spin;
        static constexpr float SPIN_DECAY = 2.0f;
        static constexpr float MAX_SPIN = 1.0f;
        static constexpr float SPIN_INFLUENCE = 0.3f;
    };

    class Brick {
    public:
        Brick(float x, float y, float width, float height, bool isAlive);
        void draw();
        Rectangle getRect() const;
        bool isAlive() const;
        void destroy();
        void setColor(Color c);
        Color getColor() const { return color; }

    private:
        float x;
        float y;
        float width;
        float height;
        bool alive;
        Color color;
    };

    Game();
    ~Game();
    void run();
    void reset();
    void initializeBricks();
    void resetBallAndPaddle();
    void updateCamera();

private:
    enum class GameState {
        START_SCREEN,
        PLAYING,
        PAUSED,
        GAME_OVER,
        WON
    };

    void update(float deltaTime);
    void draw();
    void checkPaddleCollision();
    void checkBrickCollisions();
    bool checkBallBrickCollision(const Rectangle& brickRect);
    void validateGameObjects();

private: // Added private section for camera
    Camera2D camera;

public:
    std::unique_ptr<Paddle> paddle;
    std::unique_ptr<Ball> ball;
    std::vector<std::vector<std::unique_ptr<Brick>>> bricks;
    GameState state;
    bool gameOver;
    bool won;
    bool ballAttached;  // Tracks whether the ball is attached to the paddle
    int score;
    int lives;
    static const int INITIAL_LIVES = 3;
    float ballSpeedTimer;
    static constexpr float SPEED_INCREASE_INTERVAL = 5.0f;
    static constexpr float BALL_SPEED_INCREMENT = 10.0f;
    static constexpr float MAX_BALL_SPEED = 1000.0f;
};

#endif // GAME_H
