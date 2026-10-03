#pragma once
#include <glm/glm.hpp>


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