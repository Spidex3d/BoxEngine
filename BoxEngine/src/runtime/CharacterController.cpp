#include "runtime/CharacterController.h"

#include <BoxEngine.h>
#include <entity/Entity.h>
#include <cmath>
#include <miniBoxLog.h>


void CharacterController::SetEntity(Entity* entity)
{
    m_entity = entity;
}

void CharacterController::Update(
    BoxEngine& engine,
    float deltaTime)
{
    if (!m_entity)
    {
        return;
    }

    glm::vec3 position = m_entity->GetPosition();

    GroundHit ground;

    const bool foundGround =
        engine.FindGround(
            position,
            m_capsuleHalfHeight +
            m_groundSnapDistance,
            ground
        );

    if (foundGround)
    {
        const float targetY =
            ground.point.y +
            m_capsuleHalfHeight;

       /* const float currentFeetY =
            position.y -
            m_capsuleHalfHeight;*/

        const float heightDifference =
            targetY -
            position.y;

        // -------------------------------------------------
        // Grounded / snapping to walkable ground
        // -------------------------------------------------

        if (heightDifference <=
            m_groundSnapDistance)
        {
            position.y =
                targetY;

            m_verticalVelocity =
                0.0f;

            m_grounded =
                true;
        }
        else
        {
            m_grounded =
                false;
        }
    }
    else
    {
        m_grounded =
            false;
    }

    // -------------------------------------------------
    // Gravity
    // -------------------------------------------------

    if (!m_grounded)
    {
        m_verticalVelocity +=
            m_gravity *
            deltaTime;

        position.y +=
            m_verticalVelocity *
            deltaTime;
    }

    m_entity->SetPosition(position);
}

void CharacterController::Move(
    BoxEngine& engine,
    const glm::vec3& direction,
    float deltaTime)
{
    if (!m_entity)
    {
        return;
    }

    glm::vec3 position =
        m_entity->GetPosition();

    const glm::vec3 movement =
        direction *
        m_moveSpeed *
        deltaTime;

    // -------------------------------------------------
    // Proposed horizontal movement
    // -------------------------------------------------

    glm::vec3 proposedPosition =
        position;

    proposedPosition.x +=
        movement.x;

    proposedPosition.z +=
        movement.z;

    // -------------------------------------------------
    // Find actual triangle ground at new X/Z position
    // -------------------------------------------------

    GroundHit ground;

    const bool foundGround =
        engine.FindGround(
            proposedPosition,
            m_capsuleHalfHeight +
            m_groundSnapDistance +
            0.5f,
            ground
        );

    const Entity* groundEntity =
        nullptr;

    if (foundGround)
    {
        groundEntity =
            ground.entity;

        // ---------------------------------------------
        // Work out slope angle
        // ---------------------------------------------

        const float upDot =
            glm::clamp(
                glm::dot(
                    ground.normal,
                    glm::vec3(0.0f, 1.0f, 0.0f)
                ),
                -1.0f,
                1.0f
            );

        const float slopeAngle =
            glm::degrees(
                std::acos(upDot)
            );

        const bool walkable = slopeAngle <= m_maxSlopeAngle;

        if (walkable)
        {
            const float targetY =
                ground.point.y +
                m_capsuleHalfHeight;

            const float heightChange =
                targetY -
                position.y;

            // For now allow modest changes in terrain height.
            if (heightChange <=
                m_groundSnapDistance + 0.5f)
            {
                proposedPosition.y =
                    targetY;
            }
        }
    }

    // -------------------------------------------------
    // Check walls/objects, but ignore supporting ground
    // -------------------------------------------------

    if (!engine.PlayerCollidesAt(
        proposedPosition,
        groundEntity))
    {
        m_entity->SetPosition(
            proposedPosition
        );
    }
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