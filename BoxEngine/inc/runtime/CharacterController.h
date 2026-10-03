#pragma once
#include <glm/glm.hpp>

class BoxEngine;
class Entity;

struct GroundHit
{
    bool hit = false;

    glm::vec3 point =
        glm::vec3(0.0f);

    glm::vec3 normal =
        glm::vec3(0.0f, 1.0f, 0.0f);

    float distance = 0.0f;

    Entity* entity = nullptr;
};


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

    float m_maxSlopeAngle = 45.0f;

    float m_groundSnapDistance = 0.3f;

    float m_capsuleHalfHeight = 0.9f;
    float m_capsuleRadius = 0.4f;
};
