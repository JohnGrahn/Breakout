#ifndef GAME_H
#define GAME_H

#include <raylib.h>
#include <memory>
#include <vector>

class Game {
public:
    class Paddle {
    public:
        Paddle(float x, float y, float width, float height, float speed);
        void update(float deltaTime);
        void draw();
        Rectangle getRect() const;
        void setX(float newX);

    private:
        float x;
        float y;
        float width;
        float height;
        float speed;
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
        void clampSpeed(float maxSpeed); // new method

    private:
        float x;
        float y;
        float radius;
        float speedX;
        float speedY;
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
    bool checkBallBrickCollision(const Rectangle& brickRect); // Helper to determine collision side
    void resetBallAndPaddle();  // New method to reset positions
    void initializeBricks();    // New method to initialize bricks

    std::unique_ptr<Paddle> paddle;
    std::unique_ptr<Ball> ball;
    std::vector<std::vector<std::unique_ptr<Brick>>> bricks;
    GameState state;
    bool gameOver;
    bool won;
    int score;
    int lives;  // Number of lives remaining
    static const int INITIAL_LIVES = 3;  // Starting number of lives
    float ballSpeedTimer;
    static constexpr float SPEED_INCREASE_INTERVAL = 5.0f;  // seconds
    static constexpr float BALL_SPEED_INCREMENT = 10.0f;
    static constexpr float MAX_BALL_SPEED = 1000.0f;
};

#endif // GAME_H
