#include "runtime/Collision.h"
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>
#include <limits>


namespace
{
    glm::vec3 ClosestPointOnTriangle(
        const glm::vec3& point,
        const glm::vec3& a,
        const glm::vec3& b,
        const glm::vec3& c)
    {
        const glm::vec3 ab = b - a;
        const glm::vec3 ac = c - a;
        const glm::vec3 ap = point - a;

        const float d1 = glm::dot(ab, ap);
        const float d2 = glm::dot(ac, ap);

        if (d1 <= 0.0f &&
            d2 <= 0.0f)
        {
            return a;
        }

        const glm::vec3 bp = point - b;

        const float d3 = glm::dot(ab, bp);
        const float d4 = glm::dot(ac, bp);

        if (d3 >= 0.0f &&
            d4 <= d3)
        {
            return b;
        }

        const float vc =
            d1 * d4 -
            d3 * d2;

        if (vc <= 0.0f &&
            d1 >= 0.0f &&
            d3 <= 0.0f)
        {
            const float v =
                d1 / (d1 - d3);

            return a + v * ab;
        }

        const glm::vec3 cp = point - c;

        const float d5 = glm::dot(ab, cp);
        const float d6 = glm::dot(ac, cp);

        if (d6 >= 0.0f &&
            d5 <= d6)
        {
            return c;
        }

        const float vb =
            d5 * d2 -
            d1 * d6;

        if (vb <= 0.0f &&
            d2 >= 0.0f &&
            d6 <= 0.0f)
        {
            const float w =
                d2 / (d2 - d6);

            return a + w * ac;
        }

        const float va =
            d3 * d6 -
            d5 * d4;

        if (va <= 0.0f &&
            (d4 - d3) >= 0.0f &&
            (d5 - d6) >= 0.0f)
        {
            const float w =
                (d4 - d3) /
                ((d4 - d3) +
                    (d5 - d6));

            return b +
                w * (c - b);
        }

        const float denominator =
            1.0f /
            (va + vb + vc);

        const float v =
            vb * denominator;

        const float w =
            vc * denominator;

        return a +
            ab * v +
            ac * w;
    }


    void ClosestPointsSegmentSegment(
        const glm::vec3& p1,
        const glm::vec3& q1,
        const glm::vec3& p2,
        const glm::vec3& q2,
        glm::vec3& outPoint1,
        glm::vec3& outPoint2)
    {
        constexpr float epsilon =
            0.000001f;

        const glm::vec3 d1 =
            q1 - p1;

        const glm::vec3 d2 =
            q2 - p2;

        const glm::vec3 r =
            p1 - p2;

        const float a =
            glm::dot(d1, d1);

        const float e =
            glm::dot(d2, d2);

        const float f =
            glm::dot(d2, r);

        float s = 0.0f;
        float t = 0.0f;

        if (a <= epsilon &&
            e <= epsilon)
        {
            outPoint1 = p1;
            outPoint2 = p2;
            return;
        }

        if (a <= epsilon)
        {
            s = 0.0f;

            t =
                glm::clamp(
                    f / e,
                    0.0f,
                    1.0f
                );
        }
        else
        {
            const float c =
                glm::dot(d1, r);

            if (e <= epsilon)
            {
                t = 0.0f;

                s =
                    glm::clamp(
                        -c / a,
                        0.0f,
                        1.0f
                    );
            }
            else
            {
                const float b =
                    glm::dot(d1, d2);

                const float denominator =
                    a * e -
                    b * b;

                if (denominator != 0.0f)
                {
                    s =
                        glm::clamp(
                            (b * f -
                                c * e) /
                            denominator,
                            0.0f,
                            1.0f
                        );
                }

                t =
                    (b * s + f) /
                    e;

                if (t < 0.0f)
                {
                    t = 0.0f;

                    s =
                        glm::clamp(
                            -c / a,
                            0.0f,
                            1.0f
                        );
                }
                else if (t > 1.0f)
                {
                    t = 1.0f;

                    s =
                        glm::clamp(
                            (b - c) / a,
                            0.0f,
                            1.0f
                        );
                }
            }
        }

        outPoint1 =
            p1 + d1 * s;

        outPoint2 =
            p2 + d2 * t;
    }
}


//bool Collision::CapsuleVsAABB(
//    const glm::vec3& capsulePosition,
//    float capsuleRadius,
//    float capsuleHalfHeight,
//    const glm::vec3& boxMin,
//    const glm::vec3& boxMax)
//{
//    // -------------------------------------------------
//    // Vertical overlap
//    // -------------------------------------------------
//
//    const float playerMinY =
//        capsulePosition.y -
//        capsuleHalfHeight;
//
//    const float playerMaxY =
//        capsulePosition.y +
//        capsuleHalfHeight;
//
//    if (playerMaxY < boxMin.y ||
//        playerMinY > boxMax.y)
//    {
//        return false;
//    }
//
//    // -------------------------------------------------
//    // Closest point on box in X/Z
//    // -------------------------------------------------
//
//    const float closestX =
//        glm::clamp(
//            capsulePosition.x,
//            boxMin.x,
//            boxMax.x
//        );
//
//    const float closestZ =
//        glm::clamp(
//            capsulePosition.z,
//            boxMin.z,
//            boxMax.z
//        );
//
//    const float dx =
//        capsulePosition.x -
//        closestX;
//
//    const float dz =
//        capsulePosition.z -
//        closestZ;
//
//    const float distanceSquared =
//        dx * dx +
//        dz * dz;
//
//    return
//        distanceSquared <=
//        capsuleRadius * capsuleRadius;
//}

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

bool Collision::CapsuleVsTriangle(
    const glm::vec3& capsulePosition,
    float capsuleRadius,
    float capsuleHalfHeight,
    const glm::vec3& v0,
    const glm::vec3& v1,
    const glm::vec3& v2,
    CollisionHit& outHit)
{
    outHit = CollisionHit{};

    constexpr float epsilon =
        0.000001f;

    // -------------------------------------------------
    // Capsule internal line segment
    //
    // halfHeight includes the round ends, so subtract
    // the radius to find the straight section.
    // -------------------------------------------------

    const float segmentHalfHeight =
        std::max(0.0f, capsuleHalfHeight - capsuleRadius);

    const glm::vec3 segmentStart =
        capsulePosition +
        glm::vec3(
            0.0f,
            -segmentHalfHeight,
            0.0f
        );

    const glm::vec3 segmentEnd =
        capsulePosition +
        glm::vec3(
            0.0f,
            segmentHalfHeight,
            0.0f
        );

    glm::vec3 closestCapsulePoint =
        segmentStart;

    glm::vec3 closestTrianglePoint =
        v0;

    float minimumDistanceSquared =
        std::numeric_limits<float>::max();

    // Helper for keeping the closest candidate.
    auto consider =
        [&](
            const glm::vec3& capsulePoint,
            const glm::vec3& trianglePoint)
    {
        const glm::vec3 difference =
            capsulePoint -
            trianglePoint;

        const float distanceSquared =
            glm::dot(
                difference,
                difference
            );

        if (distanceSquared <
            minimumDistanceSquared)
        {
            minimumDistanceSquared =
                distanceSquared;

            closestCapsulePoint =
                capsulePoint;

            closestTrianglePoint =
                trianglePoint;
        }
    };

    // -------------------------------------------------
    // First check whether the capsule's centre segment
    // actually passes through the triangle.
    // -------------------------------------------------

    const glm::vec3 segmentVector =
        segmentEnd -
        segmentStart;

    const float segmentLength =
        glm::length(segmentVector);

    if (segmentLength > epsilon)
    {
        const glm::vec3 segmentDirection =
            segmentVector /
            segmentLength;

        float hitDistance = 0.0f;

        glm::vec3 hitPoint =
            glm::vec3(0.0f);

        glm::vec3 hitNormal =
            glm::vec3(0.0f);

        if (RayVsTriangle(
            segmentStart,
            segmentDirection,
            v0,
            v1,
            v2,
            hitDistance,
            hitPoint,
            hitNormal))
        {
            if (hitDistance <=
                segmentLength)
            {
                consider(
                    hitPoint,
                    hitPoint
                );
            }
        }
    }

    // -------------------------------------------------
    // Capsule segment endpoints against triangle face.
    // -------------------------------------------------

    consider(
        segmentStart,
        ClosestPointOnTriangle(
            segmentStart,
            v0,
            v1,
            v2
        )
    );

    consider(
        segmentEnd,
        ClosestPointOnTriangle(
            segmentEnd,
            v0,
            v1,
            v2
        )
    );

    // -------------------------------------------------
    // Capsule segment against triangle edges.
    // -------------------------------------------------

    glm::vec3 segmentPoint;
    glm::vec3 trianglePoint;

    ClosestPointsSegmentSegment(
        segmentStart,
        segmentEnd,
        v0,
        v1,
        segmentPoint,
        trianglePoint
    );

    consider(
        segmentPoint,
        trianglePoint
    );

    ClosestPointsSegmentSegment(
        segmentStart,
        segmentEnd,
        v1,
        v2,
        segmentPoint,
        trianglePoint
    );

    consider(
        segmentPoint,
        trianglePoint
    );

    ClosestPointsSegmentSegment(
        segmentStart,
        segmentEnd,
        v2,
        v0,
        segmentPoint,
        trianglePoint
    );

    consider(
        segmentPoint,
        trianglePoint
    );

    // -------------------------------------------------
    // Is the triangle inside the capsule radius?
    // -------------------------------------------------

    const float radiusSquared =
        capsuleRadius *
        capsuleRadius;

    if (minimumDistanceSquared >
        radiusSquared)
    {
        return false;
    }

    const float distance =
        std::sqrt(std::max(minimumDistanceSquared, 0.0f));

    // -------------------------------------------------
    // Collision normal
    // -------------------------------------------------

    glm::vec3 normal;

    if (distance > epsilon)
    {
        normal =
            (closestCapsulePoint -
                closestTrianglePoint) /
            distance;
    }
    else
    {
        // Capsule segment is directly touching/passing
        // through the triangle, so use triangle normal.
        normal =
            glm::normalize(
                glm::cross(
                    v1 - v0,
                    v2 - v0
                )
            );

        // Make it point toward the capsule.
        if (glm::dot(
            normal,
            capsulePosition -
            closestTrianglePoint) <
            0.0f)
        {
            normal =
                -normal;
        }
    }

    // -------------------------------------------------
    // Result
    // -------------------------------------------------

    outHit.hit =
        true;

    outHit.point =
        closestTrianglePoint;

    outHit.normal =
        normal;

    outHit.penetration =
        capsuleRadius -
        distance;

    return true;
}
