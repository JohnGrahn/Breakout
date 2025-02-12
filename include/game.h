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

    private:
        float x;
        float y;
        float width;
        float height;
        bool alive;
    };

    Game();
    ~Game();
    void run();

private:
    void update(float deltaTime);
    void draw();
    void checkPaddleCollision();
    void checkBrickCollisions();
    bool checkBallBrickCollision(const Rectangle& brickRect); // Helper to determine collision side

    std::unique_ptr<Paddle> paddle;
    std::unique_ptr<Ball> ball;
    std::vector<std::vector<std::unique_ptr<Brick>>> bricks;
};

#endif // GAME_H 