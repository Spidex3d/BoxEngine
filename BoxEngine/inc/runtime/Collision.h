#pragma once
#include <glm/glm.hpp>

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



class Collision
{
public:

    static bool CapsuleVsAABB(
        const glm::vec3& capsulePosition,
        float capsuleRadius,
        float capsuleHalfHeight,
        const glm::vec3& boxMin,
        const glm::vec3& boxMax
    );

    
};