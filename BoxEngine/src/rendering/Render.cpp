#include "rendering/Render.h"

#include <algorithm>
#include <cstddef>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/geometric.hpp>
#include <utility>
#include <cmath>
#include <limits>

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

void Render::SetScene(RenderScene scene)
{
    // Take ownership of our scene snapshot.
    m_scene = std::move(scene);

    const glm::mat4 viewProjection =
        m_scene.projection * m_scene.view;

    m_inverseViewProjection =
        glm::inverse(viewProjection);
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

bool Render::IsOccluded(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) const
{
    for (const RenderTriangle& triangle :
        m_scene.triangles)
    {
        float hitDistance = 0.0f;

        if (IntersectTriangle(
            rayOrigin,
            rayDirection,
            triangle,
            hitDistance))
        {
            return true;
        }
    }

    return false;
}


// ============================================
// START RENDER
// ============================================

void Render::StartRender()
{
    if (m_renderTexture == 0)
        return;

    std::fill(
        m_pixels.begin(),
        m_pixels.end(),
        0
    );

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
    }
}

// ============================================
// GENERATE ONE IMAGE ROW
// ============================================


void Render::RenderScanline(int y)
{
    // Simple light direction for our first test.
    const glm::vec3 lightDirection = glm::normalize(glm::vec3(-0.5f, 1.0f, 0.7f));

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

        float closestDistance = std::numeric_limits<float>::max();

        const RenderTriangle* closestTriangle = nullptr;

        // ========================================
        // TEST AGAINST SCENE TRIANGLES
        // ========================================

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

            //if (hitDistance >= closestDistance)
            //    continue;

            //closestDistance = hitDistance;

            //// ====================================
            //// SIMPLE FLAT SHADING
            //// ====================================

            //glm::vec3 normal =
            //    glm::cross(
            //        triangle.b - triangle.a,
            //        triangle.c - triangle.a
            //    );

            //const float normalLength =
            //    glm::length(normal);

            //if (normalLength > 0.000001f)
            //{
            //    normal /= normalLength;

            //    // Temporary two-sided shading.
            //    if (glm::dot(normal, rayDirection) > 0.0f)
            //        normal = -normal;

            //    const float diffuse =
            //        glm::max(
            //            glm::dot(normal, lightDirection),
            //            0.0f
            //        );

            //    const float brightness =
            //        0.25f + 0.75f * diffuse;

            //    pixelColor =
            //        triangle.color * brightness;
            //}
            //else
            //{
            //    pixelColor = triangle.color;
            //}
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
                    ambient + 0.80f * diffuse;

                pixelColor =
                    closestTriangle->color * brightness;
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



//void Render::RenderScanline(int y)
//{
//    for (int x = 0; x < m_width; ++x)
//    {
//        const float u =
//            static_cast<float>(x) /
//            static_cast<float>(m_width);
//
//        const float v =
//            static_cast<float>(y) /
//            static_cast<float>(m_height);
//
//        unsigned char r = 0;
//        unsigned char g = 0;
//        unsigned char b = 0;
//
//        // Simple test landscape.
//        if (v > 0.45f)
//        {
//            // Blue sky.
//            r = static_cast<unsigned char>(
//                35.0f + 65.0f * u);
//
//            g = static_cast<unsigned char>(
//                90.0f + 80.0f * v);
//
//            b = 210;
//        }
//        else
//        {
//            // Green ground.
//            r = 50;
//
//            g = static_cast<unsigned char>(
//                85.0f + 65.0f * v);
//
//            b = 45;
//        }
//
//        const std::size_t index =
//            (static_cast<std::size_t>(y) *
//                static_cast<std::size_t>(m_width) +
//                static_cast<std::size_t>(x)) * 4;
//
//        m_pixels[index + 0] = r;
//        m_pixels[index + 1] = g;
//        m_pixels[index + 2] = b;
//        m_pixels[index + 3] = 255;
//    }
//}

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
}
