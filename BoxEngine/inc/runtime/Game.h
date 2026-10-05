#pragma once
#include <runtime/Player.h>
#include <runtime/Collectible.h>

#include <vector>

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

	// Add a collectible to the game
    void AddCollectible(Entity* entity, CollectibleType type, int value = 1);



private:

    Player m_player;

 

    std::vector<Collectible>m_collectibles;
};
