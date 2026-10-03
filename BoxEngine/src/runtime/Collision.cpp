#include "runtime/Collision.h"
#include <glm/glm.hpp>


#include "runtime/Collision.h"

#include <glm/glm.hpp>


bool Collision::CapsuleVsAABB(
    const glm::vec3& capsulePosition,
    float capsuleRadius,
    float capsuleHalfHeight,
    const glm::vec3& boxMin,
    const glm::vec3& boxMax)
{
    // -------------------------------------------------
    // Vertical overlap
    // -------------------------------------------------

    const float playerMinY =
        capsulePosition.y -
        capsuleHalfHeight;

    const float playerMaxY =
        capsulePosition.y +
        capsuleHalfHeight;

    if (playerMaxY < boxMin.y ||
        playerMinY > boxMax.y)
    {
        return false;
    }

    // -------------------------------------------------
    // Closest point on box in X/Z
    // -------------------------------------------------

    const float closestX =
        glm::clamp(
            capsulePosition.x,
            boxMin.x,
            boxMax.x
        );

    const float closestZ =
        glm::clamp(
            capsulePosition.z,
            boxMin.z,
            boxMax.z
        );

    const float dx =
        capsulePosition.x -
        closestX;

    const float dz =
        capsulePosition.z -
        closestZ;

    const float distanceSquared =
        dx * dx +
        dz * dz;

    return
        distanceSquared <=
        capsuleRadius * capsuleRadius;
}