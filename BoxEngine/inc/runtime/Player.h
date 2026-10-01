#pragma once
#include <glm/glm.hpp>

class BoxEngine;
class Entity;

class Player
{
public:
    bool Initialize(BoxEngine& engine);
    void Update(BoxEngine& engine, float deltaTime);
    void Shutdown(BoxEngine& engine);

    void Move(
        const glm::vec3& direction,
        float deltaTime
    );

    void Jump();
	
    Entity* GetEntity()
    {
        return m_entity;
    }

private:
    Entity* m_entity = nullptr;

    float m_moveSpeed = 3.0f;

    float m_verticalVelocity = 0.0f;
    float m_gravity = -9.81f;

    float m_jumpSpeed = 5.0f;

    bool m_grounded = false;
};
