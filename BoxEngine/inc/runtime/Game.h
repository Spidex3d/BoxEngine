#pragma once
#include <runtime/Player.h>

class BoxEngine;


class Game
{
public:
    bool Initialize(BoxEngine& engine);
    void Update(BoxEngine& engine, float deltaTime);
    void Shutdown(BoxEngine& engine);

    Player& GetPlayer()
    {
        return m_player;
    }

private:

    Player m_player;
};
