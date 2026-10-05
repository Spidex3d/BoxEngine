#pragma once
#include <glm/glm.hpp>
#include <runtime/Collision.h>

class BoxEngine;
class Entity;

class CharacterController
{
public:

    void SetEntity(Entity* entity);

    void Update(BoxEngine& engine, float deltaTime);

    void Move(BoxEngine& engine, const glm::vec3& direction, float deltaTime);

    void Jump();

    bool IsGrounded() const
    {
        return m_grounded;
    }

private:

    Entity* m_entity = nullptr;

    float m_moveSpeed = 3.0f;

    float m_verticalVelocity = 0.0f;
    float m_gravity = -9.81f;

    float m_jumpSpeed = 5.0f;

    bool m_grounded = false;

	float m_maxSlopeAngle = 45.0f; // cant walk up slopes steeper than this angle

	float m_maxStepHeight = 0.4f; // maximum height of a step the character can walk up

    float m_groundSnapDistance = 0.3f;

    float m_capsuleHalfHeight = 0.9f;
    float m_capsuleRadius = 0.4f;
};
