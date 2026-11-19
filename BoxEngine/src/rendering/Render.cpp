#include "rendering/Render.h"

#include <algorithm>
#include <cstddef>
#include <array>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/geometric.hpp>
#include <utility>
#include <cmath>
#include <limits>
#include <numeric>

// ============================================
// INITIALIZE RENDERER
// ============================================

bool Render::Initialize(int width, int height)
{
    if (width <= 0 || height <= 0)
        return false;

    Destroy();

    m_width = width;
    m_height = height;

    // Four bytes per pixel: R, G, B, A.
    m_pixels.resize(
        static_cast<std::size_t>(width) *
        static_cast<std::size_t>(height) * 4,
        0
    );

    glGenTextures(1, &m_renderTexture);

    if (m_renderTexture == 0)
        return false;

    glBindTexture(GL_TEXTURE_2D, m_renderTexture);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        m_width,
        m_height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        m_pixels.data()
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_CLAMP_TO_EDGE
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_CLAMP_TO_EDGE
    );

    glBindTexture(GL_TEXTURE_2D, 0);

    m_isRendering = false;
    m_progress = 0.0f;
    m_currentScanline = -1;

    return true;
}


// ============================================
// SET RENDER SCENE
// ============================================

void Render::SetScene(RenderScene scene)
{
    // Take ownership of the scene snapshot.
    m_scene = std::move(scene);

    // ========================================
    // CAMERA
    // ========================================

    const glm::mat4 viewProjection =
        m_scene.projection * m_scene.view;

    m_inverseViewProjection =
        glm::inverse(viewProjection);

    // ========================================
    // CALCULATE SCENE BOUNDING BOX
    // ========================================

    m_sceneBounds = RenderAABB{};

    


    for (const RenderTriangle& triangle :
        m_scene.triangles)
    {
        ExpandBounds(m_sceneBounds, triangle.a);
        ExpandBounds(m_sceneBounds, triangle.b);
        ExpandBounds(m_sceneBounds, triangle.c);
    }

    // ============================================
    // BUILD BVH FOR CURRENT SCENE
    // ============================================

    BuildBVH();

}

std::size_t Render::GetBVHNodeCount() const
{
    return m_bvhNodes.size();
}

bool Render::IntersectTriangle(
    const glm::vec3& rayOrigin,
    const glm::vec3& rayDirection,
    const RenderTriangle& triangle,
    float& distance) const
{
    constexpr float epsilon = 0.000001f;

    const glm::vec3 edge1 =
        triangle.b - triangle.a;

    const glm::vec3 edge2 =
        triangle.c - triangle.a;

    const glm::vec3 p =
        glm::cross(rayDirection, edge2);

    const float determinant =
        glm::dot(edge1, p);

    // Nearly parallel to the triangle.
    if (std::abs(determinant) < epsilon)
        return false;

    const float invDet = 1.0f / determinant;

    const glm::vec3 t =
        rayOrigin - triangle.a;

    const float u =
        glm::dot(t, p) * invDet;

    if (u < 0.0f || u > 1.0f)
        return false;

    const glm::vec3 q =
        glm::cross(t, edge1);

    const float v =
        glm::dot(rayDirection, q) * invDet;

    if (v < 0.0f || u + v > 1.0f)
        return false;

    const float hitDistance =
        glm::dot(edge2, q) * invDet;

    if (hitDistance <= epsilon)
        return false;

    distance = hitDistance;

    return true;
}


// ============================================
// EXPAND A BOUNDING BOX
// ============================================

void Render::ExpandBounds(
    RenderAABB& bounds,
    const glm::vec3& point) const
{
    // First point initializes the box.
    if (!bounds.valid)
    {
        bounds.min = point;
        bounds.max = point;
        bounds.valid = true;
        return;
    }

    // Expand the minimum coordinates.
    bounds.min = glm::min(bounds.min, point);

    // Expand the maximum coordinates.
    bounds.max = glm::max(bounds.max, point);
}


// ============================================
// RAY - AABB INTERSECTION
// ============================================

bool Render::IntersectAABB(
    const glm::vec3& rayOrigin,
    const glm::vec3& rayDirection,
    const RenderAABB& bounds,
    float maxDistance) const
{
    if (!bounds.valid)
        return false;

    constexpr float epsilon = 0.00000001f;

    float tMin = 0.0f;
    float tMax = maxDistance;

    // Test each axis: X, Y, Z.
    for (int axis = 0; axis < 3; ++axis)
    {
        const float origin = rayOrigin[axis];
        const float direction = rayDirection[axis];

        const float boxMin = bounds.min[axis];
        const float boxMax = bounds.max[axis];

        // Ray is parallel to this pair of faces.
        if (std::abs(direction) < epsilon)
        {
            // Outside the box: cannot intersect.
            if (origin < boxMin || origin > boxMax)
                return false;

            continue;
        }

        const float inverseDirection =
            1.0f / direction;

        float t1 =
            (boxMin - origin) * inverseDirection;

        float t2 =
            (boxMax - origin) * inverseDirection;

        // Ensure t1 is the entry distance.
        if (t1 > t2)
            std::swap(t1, t2);

        tMin = std::max(tMin, t1);
        tMax = std::min(tMax, t2);

        // Entry occurs after exit.
        if (tMin > tMax)
            return false;
    }


    if (tMax < 0.0f)
        return false;

    return tMax >= 0.0f;
}


// ============================================
// BUILD COMPLETE BVH
// ============================================

void Render::BuildBVH()
{
    m_bvhNodes.clear();
    m_triangleIndices.clear();

    const int triangleCount =
        static_cast<int>(m_scene.triangles.size());

    if (triangleCount == 0)
        return;

    // Create triangle indices:
    // 0, 1, 2, 3, 4, ...
    m_triangleIndices.resize(triangleCount);

    std::iota(
        m_triangleIndices.begin(),
        m_triangleIndices.end(),
        0
    );

    // Reserve enough space for the tree.
    // This also reduces memory reallocations.
    m_bvhNodes.reserve(
        static_cast<std::size_t>(triangleCount) * 2
    );

    // Build the root node and its children.
    BuildBVHNode(0, triangleCount);
}


// ============================================
// BUILD BVH NODE
// ============================================

int Render::BuildBVHNode(int start, int end)
{
    // Create a new node.
    const int nodeIndex =
        static_cast<int>(m_bvhNodes.size());

    m_bvhNodes.emplace_back();

    // ========================================
    // CALCULATE NODE BOUNDS
    // ========================================

    RenderAABB bounds;
    RenderAABB centroidBounds;

    for (int i = start; i < end; ++i)
    {
        const int triangleIndex =
            m_triangleIndices[i];

        const RenderTriangle& triangle =
            m_scene.triangles[triangleIndex];

        // Expand the node around all vertices.
        ExpandBounds(bounds, triangle.a);
        ExpandBounds(bounds, triangle.b);
        ExpandBounds(bounds, triangle.c);

        // Find the triangle's centre.
        const glm::vec3 centroid =
            (triangle.a + triangle.b + triangle.c)
            / 3.0f;

        ExpandBounds(centroidBounds, centroid);
    }

    // Store bounds and triangle range.
    m_bvhNodes[nodeIndex].bounds = bounds;
    m_bvhNodes[nodeIndex].start = start;
    m_bvhNodes[nodeIndex].count = end - start;

    const int triangleCount = end - start;

    // ========================================
    // LEAF NODE?
    // ========================================

    if (triangleCount <= m_maxLeafTriangles)
    {
        return nodeIndex;
    }

    // ========================================
    // CHOOSE SPLIT AXIS
    // ========================================

    const glm::vec3 extent =
        centroidBounds.max - centroidBounds.min;

    int axis = 0;

    if (extent.y > extent.x)
        axis = 1;

    if (extent.z > extent[axis])
        axis = 2;

    // If triangle centres are identical,
    // splitting by position is not useful.
    if (extent[axis] <= 0.000001f)
    {
        return nodeIndex;
    }

    // ========================================
    // SPLIT TRIANGLES IN TWO GROUPS
    // ========================================

    const int mid =
        start + triangleCount / 2;

    // Arrange triangle indices so the first
    // half has smaller centroids on this axis.
    std::nth_element(
        m_triangleIndices.begin() + start,
        m_triangleIndices.begin() + mid,
        m_triangleIndices.begin() + end,

        [this, axis](int a, int b)
    {
        const RenderTriangle& ta =
            m_scene.triangles[a];

        const RenderTriangle& tb =
            m_scene.triangles[b];

        const float centreA =
            (ta.a[axis] +
                ta.b[axis] +
                ta.c[axis]) / 3.0f;

        const float centreB =
            (tb.a[axis] +
                tb.b[axis] +
                tb.c[axis]) / 3.0f;

        return centreA < centreB;
    }
    );

    // ========================================
    // RECURSIVELY BUILD CHILD NODES
    // ========================================

    const int leftChild =
        BuildBVHNode(start, mid);

    const int rightChild =
        BuildBVHNode(mid, end);

    // IMPORTANT:
    // Access the parent by index rather than
    // retaining a reference across recursion.
    // The vector may grow during child creation.
    m_bvhNodes[nodeIndex].left = leftChild;
    m_bvhNodes[nodeIndex].right = rightChild;

    // Internal nodes don't directly store
    // triangles for traversal.
    m_bvhNodes[nodeIndex].count = 0;

    return nodeIndex;
}


// ============================================
// BVH - TRACE CLOSEST TRIANGLE
// ============================================

bool Render::TraceClosest(
    const glm::vec3& rayOrigin,
    const glm::vec3& rayDirection,
    float& closestDistance,
    int& closestTriangleIndex) const
{
    if (m_bvhNodes.empty())
        return false;

    bool hit = false;

    // Fixed-size traversal stack.
    // Our median-split BVH is balanced, so
    // 128 entries is ample for practical scenes.
    std::array<int, 128> stack;
    int stackPointer = 0;

    // Start at the root node.
    stack[stackPointer++] = 0;

    while (stackPointer > 0)
    {
        const int nodeIndex =
            stack[--stackPointer];

        const BVHNode& node =
            m_bvhNodes[nodeIndex];

        // ====================================
        // TEST NODE BOUNDING BOX
        // ====================================

        if (!IntersectAABB(
            rayOrigin,
            rayDirection,
            node.bounds,
            closestDistance))
        {
            continue;
        }

        // ====================================
        // LEAF NODE - TEST TRIANGLES
        // ====================================

        if (node.IsLeaf())
        {
            for (int i = 0; i < node.count; ++i)
            {
                const int triangleIndex =
                    m_triangleIndices[
                        node.start + i
                    ];

                const RenderTriangle& triangle =
                    m_scene.triangles[triangleIndex];

                float distance = 0.0f;

                if (!IntersectTriangle(
                    rayOrigin,
                    rayDirection,
                    triangle,
                    distance))
                {
                    continue;
                }

                if (distance < closestDistance)
                {
                    closestDistance = distance;

                    closestTriangleIndex =
                        triangleIndex;

                    hit = true;
                }
            }
        }

        else
        {
            // ====================================
            // INTERNAL NODE - VISIT CHILDREN
            // ====================================

            if (node.left >= 0)
            {
                stack[stackPointer++] = node.left;
            }

            if (node.right >= 0)
            {
                stack[stackPointer++] = node.right;
            }
        }
    }

    return hit;
}


// ============================================
// BVH - TRACE ANY TRIANGLE
// ============================================

bool Render::TraceAny(
    const glm::vec3& rayOrigin,
    const glm::vec3& rayDirection) const
{
    if (m_bvhNodes.empty())
        return false;

    std::array<int, 128> stack;
    int stackPointer = 0;

    stack[stackPointer++] = 0;

    while (stackPointer > 0)
    {
        const int nodeIndex =
            stack[--stackPointer];

        const BVHNode& node =
            m_bvhNodes[nodeIndex];

        // ====================================
        // TEST NODE BOUNDING BOX
        // ====================================

        if (!IntersectAABB(
            rayOrigin,
            rayDirection,
            node.bounds,
            std::numeric_limits<float>::max()))
        {
            continue;
        }

        // ====================================
        // LEAF NODE
        // ====================================

        if (node.IsLeaf())
        {
            for (int i = 0; i < node.count; ++i)
            {
                const int triangleIndex =
                    m_triangleIndices[
                        node.start + i
                    ];

                const RenderTriangle& triangle =
                    m_scene.triangles[triangleIndex];

                float distance = 0.0f;

                if (IntersectTriangle(
                    rayOrigin,
                    rayDirection,
                    triangle,
                    distance))
                {
                    // Found something blocking
                    // the light. Stop immediately.
                    return true;
                }
            }
        }

        else
        {
            // ====================================
            // INTERNAL NODE
            // ====================================

            if (node.left >= 0)
            {
                stack[stackPointer++] = node.left;
            }

            if (node.right >= 0)
            {
                stack[stackPointer++] = node.right;
            }
        }
    }

    return false;
}


// =======================================================================================================
// SAMPLE CPU ALBEDO TEXTURE
//==============================================================================================


glm::vec3 Render::SampleTexture(const RenderTexture& texture, const glm::vec2& uv) const
{
    if (!texture.IsValid())
    {
        return glm::vec3(1.0f);
    }

    // ============================================
    // REPEAT UV COORDINATES
    // ============================================

    const float u = uv.x - std::floor(uv.x);
    const float v = uv.y - std::floor(uv.y);

    // Convert UV into texture pixel coordinates.
    // Subtracting 0.5 puts integer coordinates
    // at the centres of texture pixels.
    const float x = u * texture.width - 0.5f;
    const float y = v * texture.height - 0.5f;

    const int x0 = static_cast<int>(std::floor(x));
    const int y0 = static_cast<int>(std::floor(y));

    const int x1 = x0 + 1;
    const int y1 = y0 + 1;

    // Fractional position between four pixels.
    const float tx = x - static_cast<float>(x0);
    const float ty = y - static_cast<float>(y0);

    // ============================================
    // REPEAT-WRAP PIXEL COORDINATES
    // ============================================

    auto wrap = [](int value, int size) -> int
    {
        return ((value % size) + size) % size;
    };

    const int px0 = wrap(x0, texture.width);
    const int px1 = wrap(x1, texture.width);

    const int py0 = wrap(y0, texture.height);
    const int py1 = wrap(y1, texture.height);

    // ============================================
    // READ A TEXTURE PIXEL
    // ============================================

    auto readPixel =
        [&](int px, int py) -> glm::vec3
    {
        const std::size_t index =
            (static_cast<std::size_t>(py) *
                static_cast<std::size_t>(texture.width) +
                static_cast<std::size_t>(px)) * 4;

        return glm::vec3(
            texture.pixels[index + 0] / 255.0f,
            texture.pixels[index + 1] / 255.0f,
            texture.pixels[index + 2] / 255.0f
        );
    };

    // ============================================
    // GET FOUR NEIGHBOURING PIXELS
    // ============================================

    const glm::vec3 c00 = readPixel(px0, py0);
    const glm::vec3 c10 = readPixel(px1, py0);

    const glm::vec3 c01 = readPixel(px0, py1);
    const glm::vec3 c11 = readPixel(px1, py1);

    // ============================================
    // BILINEAR INTERPOLATION
    // ============================================

    // Blend horizontally.
    const glm::vec3 top =
        glm::mix(c00, c10, tx);

    const glm::vec3 bottom =
        glm::mix(c01, c11, tx);

    // Blend vertically.
    return glm::mix(top, bottom, ty);
}


// ============================================
// SHADOW RAY - BVH ACCELERATED
// ============================================

bool Render::IsOccluded(
    const glm::vec3& rayOrigin,
    const glm::vec3& rayDirection) const
{
    return TraceAny(
        rayOrigin,
        rayDirection
    );
}


// ============================================
// START RENDER
// ============================================

void Render::StartRender()
{
    if (m_renderTexture == 0)
        return;

    // Start a new render timer.
    m_renderElapsedSeconds = 0.0;
    m_renderStartTime = RenderClock::now();


    std::fill(m_pixels.begin(), m_pixels.end(), 0);

    // OpenGL textures store the bottom row first.
    // Start at the top and move downwards.
    m_currentScanline = m_height - 1;

    m_progress = 0.0f;
    m_isRendering = true;

    glBindTexture(GL_TEXTURE_2D, m_renderTexture);

    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        0, 0,
        m_width,
        m_height,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        m_pixels.data()
    );

    glBindTexture(GL_TEXTURE_2D, 0);
}

// ============================================
// UPDATE RENDER
// ============================================

void Render::Update()
{
    if (!m_isRendering)
        return;

    

    // Render a few horizontal rows each frame.
    // This keeps the editor responsive.

    for (int i = 0;
        i < m_scanlinesPerUpdate;
        ++i)
    {
        if (m_currentScanline < 0)
        {
            m_isRendering = false;
            m_progress = 1.0f;
            break;
        }

        RenderScanline(m_currentScanline);
        UploadScanline(m_currentScanline);

        --m_currentScanline;
    }

    const int completedRows =
        m_height - 1 - m_currentScanline;

    m_progress =
        static_cast<float>(completedRows) /
        static_cast<float>(m_height);

    if (m_currentScanline < 0)
    {
        m_isRendering = false;
        m_progress = 1.0f;

        // Record the final render time.
        m_renderElapsedSeconds =
            std::chrono::duration<double>(
                RenderClock::now() - m_renderStartTime
            ).count();
    }


    // Update the elapsed rendering time.
    m_renderElapsedSeconds = std::chrono::duration<double>(
        RenderClock::now() - m_renderStartTime).count();
}

// ============================================
// GENERATE ONE IMAGE ROW
// ============================================


void Render::RenderScanline(int y)
{
    // ============================================
    // SCENE DIRECTIONAL LIGHT
    // ============================================

    const glm::vec3 lightDirection = glm::normalize(m_scene.lightDirection);

    // Simple light direction for our first test.
    //const glm::vec3 lightDirection = glm::normalize(glm::vec3(-0.5f, 1.0f, 0.7f));

    for (int x = 0; x < m_width; ++x)
    {
        // ========================================
        // GENERATE CAMERA RAY
        // ========================================

        const float ndcX =
            (2.0f * (static_cast<float>(x) + 0.5f) /
                static_cast<float>(m_width)) - 1.0f;

        const float ndcY =
            (2.0f * (static_cast<float>(y) + 0.5f) /
                static_cast<float>(m_height)) - 1.0f;

        // Unproject a point on the near plane.
        glm::vec4 nearPoint =
            m_inverseViewProjection *
            glm::vec4(ndcX, ndcY, -1.0f, 1.0f);

        // Unproject a point on the far plane.
        glm::vec4 farPoint =
            m_inverseViewProjection *
            glm::vec4(ndcX, ndcY, 1.0f, 1.0f);

        nearPoint /= nearPoint.w;
        farPoint /= farPoint.w;

        const glm::vec3 rayOrigin =
            glm::vec3(nearPoint);

        const glm::vec3 rayDirection =
            glm::normalize(
                glm::vec3(farPoint - nearPoint)
            );

        // ========================================
        // DEFAULT BACKGROUND
        // ========================================

        glm::vec3 pixelColor(0.15f, 0.20f, 0.27f);


        // ========================================
        // TRACE CAMERA RAY USING BVH
        // ========================================

        float closestDistance =
            std::numeric_limits<float>::max();

        int closestTriangleIndex = -1;

        const bool hitGeometry =
            TraceClosest(
                rayOrigin,
                rayDirection,
                closestDistance,
                closestTriangleIndex
            );

        const RenderTriangle* closestTriangle = nullptr;

        if (hitGeometry && closestTriangleIndex >= 0)
        {
            closestTriangle =
                &m_scene.triangles[closestTriangleIndex];
        }

        // ========================================
        // TEST AGAINST SCENE TRIANGLES
        // ========================================
        if (IntersectAABB(rayOrigin,
            rayDirection, m_sceneBounds, closestDistance))
        {

            for (const RenderTriangle& triangle :
                m_scene.triangles)
            {
                float hitDistance = 0.0f;

                if (!IntersectTriangle(
                    rayOrigin,
                    rayDirection,
                    triangle,
                    hitDistance))
                {
                    continue;
                }

                if (hitDistance >= closestDistance)
                    continue;

                closestDistance = hitDistance;
                closestTriangle = &triangle;

            }
        }

        // ========================================
        // SHADE THE CLOSEST HIT
        // ========================================

        if (closestTriangle != nullptr)
        {
            glm::vec3 normal =
                glm::cross(
                    closestTriangle->b - closestTriangle->a,
                    closestTriangle->c - closestTriangle->a
                );

            const float normalLength =
                glm::length(normal);

            if (normalLength > 0.000001f)
            {
                normal /= normalLength;

                // Two-sided shading for now.
                if (glm::dot(normal, rayDirection) > 0.0f)
                    normal = -normal;

                // Calculate hit position.
                const glm::vec3 hitPosition =
                    rayOrigin +
                    rayDirection * closestDistance;


                // ============================================
                // GET SURFACE ALBEDO COLOUR
                // ============================================

                // Start with the assigned material colour.
                glm::vec3 surfaceColor = closestTriangle->color;

                // Check whether this triangle has a texture.
                const int textureIndex =
                    closestTriangle->textureIndex;

                if (textureIndex >= 0 &&
                    static_cast<std::size_t>(textureIndex) <
                    m_scene.textures.size())
                {
                    // ========================================
                    // CALCULATE BARYCENTRIC COORDINATES
                    // ========================================

                    const glm::vec3 edge0 =
                        closestTriangle->b - closestTriangle->a;

                    const glm::vec3 edge1 =
                        closestTriangle->c - closestTriangle->a;

                    const glm::vec3 point =
                        hitPosition - closestTriangle->a;

                    const float d00 = glm::dot(edge0, edge0);
                    const float d01 = glm::dot(edge0, edge1);
                    const float d11 = glm::dot(edge1, edge1);

                    const float d20 = glm::dot(point, edge0);
                    const float d21 = glm::dot(point, edge1);

                    const float denominator =
                        d00 * d11 - d01 * d01;

                    if (std::abs(denominator) > 0.00000001f)
                    {
                        const float weightB =
                            (d11 * d20 - d01 * d21) /
                            denominator;

                        const float weightC =
                            (d00 * d21 - d01 * d20) /
                            denominator;

                        const float weightA =
                            1.0f - weightB - weightC;

                        // ====================================
                        // INTERPOLATE UV COORDINATES
                        // ====================================

                        const glm::vec2 uv =
                            closestTriangle->uvA * weightA +
                            closestTriangle->uvB * weightB +
                            closestTriangle->uvC * weightC;

                        // ====================================
                        // SAMPLE TEXTURE
                        // ====================================

                        const glm::vec3 textureColor =
                            SampleTexture(
                                m_scene.textures[textureIndex],
                                uv
                            );

                        // Texture multiplied by material tint.
                        surfaceColor *= textureColor;
                    }
                }





                // Small offset to prevent the shadow
                // ray from hitting the same surface.
                constexpr float shadowBias = 0.001f;

                const glm::vec3 shadowOrigin =
                    hitPosition + normal * shadowBias;

                // Cast a ray toward the light.
                const bool inShadow =
                    IsOccluded(
                        shadowOrigin,
                        lightDirection
                    );

                const float ambient = 0.20f;

                float diffuse = 0.0f;

                if (!inShadow)
                {
                    diffuse =
                        glm::max(
                            glm::dot(normal, lightDirection),
                            0.0f
                        );
                }

                const float brightness =
                    ambient + 0.80f * diffuse * m_scene.lightIntensity;

                pixelColor = surfaceColor * brightness;
            }
            else
            {
                pixelColor = closestTriangle->color;
            }
        }

        // ========================================
        // WRITE PIXEL TO CPU BUFFER
        // ========================================

        pixelColor =
            glm::clamp(
                pixelColor,
                glm::vec3(0.0f),
                glm::vec3(1.0f)
            );

        const std::size_t index =
            (static_cast<std::size_t>(y) *
                static_cast<std::size_t>(m_width) +
                static_cast<std::size_t>(x)) * 4;

        m_pixels[index + 0] =
            static_cast<unsigned char>(pixelColor.r * 255.0f);

        m_pixels[index + 1] =
            static_cast<unsigned char>(pixelColor.g * 255.0f);

        m_pixels[index + 2] =
            static_cast<unsigned char>(pixelColor.b * 255.0f);

        m_pixels[index + 3] = 255;
    }
}


// ============================================
// UPLOAD ONE SCANLINE
// ============================================

void Render::UploadScanline(int y)
{
    const std::size_t offset =
        static_cast<std::size_t>(y) *
        static_cast<std::size_t>(m_width) * 4;

    glBindTexture(
        GL_TEXTURE_2D,
        m_renderTexture
    );

    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        0,
        y,
        m_width,
        1,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        m_pixels.data() + offset
    );

    glBindTexture(GL_TEXTURE_2D, 0);
}

// ============================================
// CANCEL RENDER
// ============================================

void Render::CancelRender()
{
    m_isRendering = false;

    // Reset progress.
    m_currentScanline = -1;
    m_progress = 0.0f;

    // Reset render timer.
    m_renderElapsedSeconds = 0.0;
}

// ============================================
// RENDER STATUS
// ============================================

bool Render::IsRendering() const
{
    return m_isRendering;
}

float Render::GetProgress() const
{
    return m_progress;
}

GLuint Render::GetRenderTexture() const
{
    return m_renderTexture;
}

double Render::GetRenderTime() const
{
    return m_renderElapsedSeconds;
}

// ============================================
// CLEANUP
// ============================================

void Render::Destroy()
{
    m_isRendering = false;

    if (m_renderTexture != 0)
    {
        glDeleteTextures(1, &m_renderTexture);
        m_renderTexture = 0;
    }

    m_pixels.clear();

    m_width = 0;
    m_height = 0;
    m_currentScanline = -1;
    m_progress = 0.0f;

    m_renderElapsedSeconds = 0.0;
}
