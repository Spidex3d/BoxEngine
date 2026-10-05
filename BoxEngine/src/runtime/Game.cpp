#include "runtime/Game.h"
#include <BoxEngine.h>
#include <entity/Entity.h>
#include <miniBoxLog.h>


bool Game::Initialize(BoxEngine& engine)
{
    // -------------------------------------------------
   // Create Player
   // -------------------------------------------------

    if (!m_player.Initialize(engine))
    {
        return false;
    }

    // -------------------------------------------------
    // Create first test collectible
    // -------------------------------------------------

    Entity* collectible = engine.CreateRuntimeCollectible(
            glm::vec3(0.0f, 0.5f, -3.0f));

    if (!collectible)
    {
        BOX_LOG_ERROR(
            "Failed to create test collectible");

        return false;
    }

    AddCollectible(collectible, CollectibleType::Water, 1);

    BOX_LOG_INFO("Test collectible created");

    return true;

}

void Game::Update(BoxEngine& engine, float deltaTime)
{
    m_player.Update(
        engine,
        deltaTime
    );

    Entity* playerEntity =
        m_player.GetEntity();

    if (!playerEntity)
    {
        return;
    }

    const glm::vec3 playerPosition =
        playerEntity->GetPosition();

    constexpr float collectRadius = 0.8f;

    for (Collectible& collectible :
        m_collectibles)
    {
        if (collectible.IsCollected())
        {
            continue;
        }

        Entity* entity =
            collectible.GetEntity();

        if (!entity)
        {
            continue;
        }

        const glm::vec3 itemPosition =
            entity->GetPosition();

        const float distance =
            glm::length(
                playerPosition -
                itemPosition
            );

        if (distance <= collectRadius)
        {
            collectible.Collect();

            BOX_LOG_INFO("Collected Water item!");
        }
    }
}

// ---------------------------------------------------------------
// Add a collectible to the game
// ---------------------------------------------------------------

void Game::AddCollectible(Entity* entity, CollectibleType type, int value)
{
    if (!entity)
    {
        return;
    }

    m_collectibles.emplace_back(entity, type, value);
}





void Game::Shutdown(BoxEngine& engine)
{
    m_player.Shutdown(engine);
}
