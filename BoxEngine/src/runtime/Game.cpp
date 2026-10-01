#include "runtime/Game.h"


bool Game::Initialize(BoxEngine& engine)
{
    return m_player.Initialize(engine);
}


void Game::Update(BoxEngine& engine, float deltaTime)
{
    m_player.Update(engine, deltaTime);
}


void Game::Shutdown(BoxEngine& engine)
{
    m_player.Shutdown(engine);
}
