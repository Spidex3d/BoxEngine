#include "runtime/Collision.h"
#include <glm/glm.hpp>
#include <cmath>



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

bool Collision::RayVsTriangle(
    const glm::vec3& rayOrigin,
    const glm::vec3& rayDirection,
    const glm::vec3& v0,
    const glm::vec3& v1,
    const glm::vec3& v2,
    float& outDistance,
    glm::vec3& outHitPoint,
    glm::vec3& outNormal)
{
    constexpr float epsilon = 0.000001f;

    // -------------------------------------------------
    // Triangle edges
    // -------------------------------------------------

    const glm::vec3 edge1 =
        v1 - v0;

    const glm::vec3 edge2 =
        v2 - v0;

    // -------------------------------------------------
    // Begin Moller-Trumbore intersection test
    // -------------------------------------------------

    const glm::vec3 h =
        glm::cross(
            rayDirection,
            edge2
        );

    const float a =
        glm::dot(
            edge1,
            h
        );

    if (std::abs(a) < epsilon)
    {
        return false;
    }

    const float f =
        1.0f / a;

    const glm::vec3 s =
        rayOrigin - v0;

    const float u =
        f *
        glm::dot(
            s,
            h
        );

    if (u < 0.0f ||
        u > 1.0f)
    {
        return false;
    }

    const glm::vec3 q =
        glm::cross(
            s,
            edge1
        );

    const float v =
        f *
        glm::dot(
            rayDirection,
            q
        );

    if (v < 0.0f ||
        u + v > 1.0f)
    {
        return false;
    }

    const float distance =
        f *
        glm::dot(
            edge2,
            q
        );

    // -------------------------------------------------
    // Intersection must be in front of ray origin
    // -------------------------------------------------

    if (distance <= epsilon)
    {
        return false;
    }

    // -------------------------------------------------
    // Output result
    // -------------------------------------------------

    outDistance =
        distance;

    outHitPoint =
        rayOrigin +
        rayDirection *
        distance;

    outNormal =
        glm::normalize(
            glm::cross(
                edge1,
                edge2
            )
        );

    if (outNormal.y < 0.0f)
    {
        outNormal =
            -outNormal;
    }

    return true;
}