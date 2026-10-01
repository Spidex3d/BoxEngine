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

    //m_entity = engine.CreateRuntimePlayer(glm::vec3(0.0f, 0.9f, 0.0f));
    m_entity = engine.CreateRuntimePlayer(glm::vec3(0.0f, 1.9f, 0.0f));

    if (!m_entity)
    {
        BOX_LOG_ERROR(
            "Player::Initialize failed to create Player"
        );

        return false;
    }

    m_verticalVelocity = 0.0f;
    m_grounded = false;

    BOX_LOG_INFO(
        "Player initialized"
    );

    return true;
}

// -----------------------------------------------------------
// Update the Player's position and apply gravity
// -----------------------------------------------------------

void Player::Update(BoxEngine& engine, float deltaTime)
{
    if (!m_entity)
    {
        return;
    }

    // -------------------------------------------------
    // Apply gravity
    // -------------------------------------------------

    m_verticalVelocity +=
        m_gravity * deltaTime;

    glm::vec3 position =
        m_entity->GetPosition();

    position.y +=
        m_verticalVelocity * deltaTime;

    // -------------------------------------------------
    // Temporary ground test
    // -------------------------------------------------

    const float groundY = 0.0f;

    // Our capsule is 1.8 high, so its centre
    // needs to sit 0.9 above the ground.
    const float playerHalfHeight = 0.4f;

    const float minimumY =
        groundY + playerHalfHeight;

    if (position.y <= minimumY)
    {
        position.y = minimumY;

        m_verticalVelocity = 0.0f;

        m_grounded = true;
    }
    else
    {
        m_grounded = false;
    }

    m_entity->SetPosition(
        position
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

void Player::Move(const glm::vec3& direction, float deltaTime)
{
    if (!m_entity)
    {
        return;
    }

    glm::vec3 position =
        m_entity->GetPosition();

    position +=
        direction *
        m_moveSpeed *
        deltaTime;

    m_entity->SetPosition(
        position
    );
}

void Player::Jump()
{
    if (!m_grounded)
    {
        return;
    }

    m_verticalVelocity = m_jumpSpeed;
    m_grounded = false;
}

