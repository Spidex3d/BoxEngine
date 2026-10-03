#include "runtime/CharacterController.h"

#include <BoxEngine.h>
#include <entity/Entity.h>


void CharacterController::SetEntity(Entity* entity)
{
    m_entity = entity;
}


void CharacterController::Update(BoxEngine& engine, float deltaTime)
{
    if (!m_entity)
    {
        return;
    }

    // We will move gravity and ground detection
    // into here next.
}


void CharacterController::Move(BoxEngine& engine, const glm::vec3& direction, float deltaTime)
{
    if (!m_entity)
    {
        return;
    }

    // We will move the proper movement
    // system into here next.
}


void CharacterController::Jump()
{
    if (!m_grounded)
    {
        return;
    }

    m_verticalVelocity =
        m_jumpSpeed;

    m_grounded =
        false;
}