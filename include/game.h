#ifndef GAME_H
#define GAME_H

class Game {
public:
    Game();
    ~Game();

    void Initialize();
    void Update();
    void Draw();
    void Close();

private:
    // Game state variables will be added here later
};

#endif // GAME_H 