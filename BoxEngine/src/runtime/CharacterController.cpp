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

    if (foundGround && m_verticalVelocity <= 0.0f)
    {
        const float targetY =
            ground.point.y +
            m_capsuleHalfHeight;

        const float heightDifference =
            targetY -
            position.y;

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


    //if (foundGround)
    //{
    //    const float targetY =
    //        ground.point.y +
    //        m_capsuleHalfHeight;

    //   /* const float currentFeetY =
    //        position.y -
    //        m_capsuleHalfHeight;*/

    //    const float heightDifference =
    //        targetY -
    //        position.y;

    //    // -------------------------------------------------
    //    // Grounded / snapping to walkable ground
    //    // -------------------------------------------------

    //    if (heightDifference <=
    //        m_groundSnapDistance)
    //    {
    //        position.y =
    //            targetY;

    //        m_verticalVelocity =
    //            0.0f;

    //        m_grounded =
    //            true;
    //    }
    //    else
    //    {
    //        m_grounded =
    //            false;
    //    }
    //}
    //else
    //{
    //    m_grounded =
    //        false;
    //}

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

void CharacterController::Move(BoxEngine& engine, const glm::vec3& direction, float deltaTime)
{
    if (!m_entity)
    {
        return;
    }

    glm::vec3 position =
        m_entity->GetPosition();

    glm::vec3 movement =
        direction *
        m_moveSpeed *
        deltaTime;

    // Horizontal movement only.
    movement.y = 0.0f;

    glm::vec3 proposedPosition =
        position +
        movement;

    // -------------------------------------------------
    // Find actual ground underneath proposed position
    // -------------------------------------------------

    GroundHit ground;

    const bool foundGround =
        engine.FindGround(
            proposedPosition,
            m_capsuleHalfHeight +
            m_groundSnapDistance +
            m_maxStepHeight,
            ground
        );

    if (foundGround)
    {
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

        const bool walkable =
            slopeAngle <=
            m_maxSlopeAngle;

        if (walkable)
        {
            const float targetY =
                ground.point.y +
                m_capsuleHalfHeight;

            const float heightChange =
                targetY -
                position.y;

            // Allow normal slope following and small steps.
            if (heightChange <=
                m_maxStepHeight)
            {
                proposedPosition.y =
                    targetY;
            }
        }
    }

    // -------------------------------------------------
    // Check actual mesh collision
    // -------------------------------------------------

    CollisionHit hit;

    if (engine.CheckCharacterCollision(
        proposedPosition,
        m_capsuleRadius,
        m_capsuleHalfHeight,
        hit))
    {
        // -------------------------------------------------
        // Classify contact
        // -------------------------------------------------

        const bool groundLike =
            hit.normal.y > 0.5f;

        const bool ceilingLike =
            hit.normal.y < -0.5f;

        const bool wallLike =
            !groundLike &&
            !ceilingLike;

        // -------------------------------------------------
        // Wall collision
        // -------------------------------------------------

        if (wallLike)
        {
            // Remove vertical part of wall normal because
            // movement here is horizontal.
            glm::vec3 wallNormal =
                hit.normal;

            wallNormal.y = 0.0f;

            const float normalLength =
                glm::length(wallNormal);

            if (normalLength > 0.0001f)
            {
                wallNormal /=
                    normalLength;

                // Remove the part of movement going
                // directly into the wall.
                const float intoWall =
                    glm::dot(
                        movement,
                        wallNormal
                    );

                if (intoWall < 0.0f)
                {
                    movement -=
                        wallNormal *
                        intoWall;
                }

                // Rebuild proposed position using
                // the sliding movement.
                proposedPosition =
                    position +
                    movement;

                // Ground-follow again after sliding.
                GroundHit slideGround;

                if (engine.FindGround(
                    proposedPosition,
                    m_capsuleHalfHeight +
                    m_groundSnapDistance +
                    m_maxStepHeight,
                    slideGround))
                {
                    const float slideUpDot =
                        glm::clamp(
                            glm::dot(
                                slideGround.normal,
                                glm::vec3(
                                    0.0f,
                                    1.0f,
                                    0.0f
                                )
                            ),
                            -1.0f,
                            1.0f
                        );

                    const float slideSlope =
                        glm::degrees(
                            std::acos(
                                slideUpDot
                            )
                        );

                    if (slideSlope <=
                        m_maxSlopeAngle)
                    {
                        const float targetY =
                            slideGround.point.y +
                            m_capsuleHalfHeight;

                        const float heightChange =
                            targetY -
                            position.y;

                        if (heightChange <=
                            m_maxStepHeight)
                        {
                            proposedPosition.y =
                                targetY;
                        }
                    }
                }

                // -------------------------------------------------
                // Check again after sliding
                // -------------------------------------------------

                CollisionHit slideHit;

                if (engine.CheckCharacterCollision(
                    proposedPosition,
                    m_capsuleRadius,
                    m_capsuleHalfHeight,
                    slideHit))
                {
                    const bool stillWallLike =
                        std::abs(
                            slideHit.normal.y
                        ) <= 0.5f;

                    if (stillWallLike)
                    {
                        // Still blocked.
                        return;
                    }
                }
            }
            else
            {
                return;
            }
        }
    }

    // -------------------------------------------------
    // Apply movement
    // -------------------------------------------------

    m_entity->SetPosition(
        proposedPosition
    );
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