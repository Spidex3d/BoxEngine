#include "runtime/Player.h"

#include <BoxEngine.h>
#include <entity/Entity.h>
#include <miniBoxLog.h>


// -----------------------------------------------------------
// Initialize the Player by creating a runtime Entity
// -----------------------------------------------------------
bool Player::Initialize(BoxEngine& engine)
{
    if (m_entity)
    {
        return true;
    }
    m_entity = engine.CreateRuntimePlayer(glm::vec3(0.0f, 0.9f, 0.0f));

    if (!m_entity)
    {
        return false;
    }

    m_controller.SetEntity(m_entity);

   /* m_verticalVelocity = 0.0f;
    m_grounded = false;

    BOX_LOG_INFO(
        "Player initialized"
    );*/

    return true;
}

// -----------------------------------------------------------
// Update the Player's position and apply gravity
// -----------------------------------------------------------

void Player::Update(BoxEngine& engine, float deltaTime)
{
    m_controller.Update(
        engine,
        deltaTime
    );
}


// -----------------------------------------------------------
// Shutdown the Player by destroying the runtime Entity
// -----------------------------------------------------------
void Player::Shutdown(BoxEngine& engine)
{
    if (!m_entity)
    {
        return;
    }

    engine.DestroyRuntimeEntity(m_entity);

    m_entity = nullptr;

    m_verticalVelocity = 0.0f;
    m_grounded = false;

    BOX_LOG_INFO(
        "Player shutdown"
    );
}

// -----------------------------------------------------------
// Move the Player in the specified direction
// -----------------------------------------------------------

void Player::Move(BoxEngine& engine, const glm::vec3& direction, float deltaTime)
{
    m_controller.Move(
        engine,
        direction,
        deltaTime
    );
}
// -----------------------------------------------------------
// Make the Player jump
// -----------------------------------------------------------

void Player::Jump()
{

    m_controller.Jump();

}

