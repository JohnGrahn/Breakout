#ifndef GAME_H
#define GAME_H

#include <raylib.h>
#include <memory>
#include <vector>

class Game {
public:
    struct SpeedConfig {
        // Base dimensions and speeds (unchanged by scaling)
        static constexpr float BASE_WINDOW_WIDTH = 800.0f;
        static constexpr float BASE_WINDOW_HEIGHT = 600.0f;
        static constexpr float PADDLE_BASE_SPEED = 500.0f;
        static constexpr float BALL_BASE_SPEED = 300.0f;
        static constexpr float BALL_SPEED_INCREMENT = 10.0f;

        // Get single scaling factor
        static float getSpeedScale() {
            return BASE_WINDOW_WIDTH / static_cast<float>(GetScreenWidth());
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

    private:
        float x;
        float y;
        float width;
        float height;
        float baseSpeed;  // Changed from speed to baseSpeed
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
        float getSpeedX() const { return baseSpeedX; }  // Return base speed
        float getSpeedY() const { return baseSpeedY; }  // Return base speed

    private:
        float x;
        float y;
        float radius;
        float baseSpeedX;  // Changed from speedX to baseSpeedX
        float baseSpeedY;  // Changed from speedY to baseSpeedY
    };

    class Brick {
    public:
        Brick(float x, float y, float width, float height, bool isAlive);
        void draw();
        Rectangle getRect() const;
        bool isAlive() const;
        void destroy();
        void setColor(Color c);

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

    std::unique_ptr<Paddle> paddle;
    std::unique_ptr<Ball> ball;
    std::vector<std::vector<std::unique_ptr<Brick>>> bricks;
    GameState state;
    bool gameOver;
    bool won;
    int score;
    int lives;
    static const int INITIAL_LIVES = 3;
    float ballSpeedTimer;
    static constexpr float SPEED_INCREASE_INTERVAL = 5.0f;
    static constexpr float BALL_SPEED_INCREMENT = 10.0f;
    static constexpr float MAX_BALL_SPEED = 1000.0f;
};

#endif // GAME_H
