#pragma once
#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// ============================================
// BoxRenderer v0.1 18/11/26
// Progressive CPU Image Renderer
// ============================================
struct RenderTriangle
{
    glm::vec3 a{ 0.0f };
    glm::vec3 b{ 0.0f };
    glm::vec3 c{ 0.0f };

    glm::vec3 color{ 0.8f, 0.8f, 0.8f };
};

// A copy of the scene data used during rendering.
struct RenderScene
{
    glm::mat4 view{ 1.0f };
    glm::mat4 projection{ 1.0f };

    std::vector<RenderTriangle> triangles;
};

class Render
{
public:

    Render() = default;
    ~Render() = default;

    bool Initialize(int width, int height);

    void SetScene(RenderScene scene);
    

    void StartRender();
    void Update();
    void CancelRender();

    bool IsRendering() const;
    float GetProgress() const;

    GLuint GetRenderTexture() const;

    void Destroy();

private:
    // Camera and geometry snapshot.
    RenderScene m_scene;

    // Combined inverse camera matrix.
    glm::mat4 m_inverseViewProjection{ 1.0f };

    // Ray-triangle intersection.
    bool IntersectTriangle(
        const glm::vec3& rayOrigin,
        const glm::vec3& rayDirection,
        const RenderTriangle& triangle,
        float& distance) const;

    bool IsOccluded(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) const;


    // Generate one horizontal row of pixels.
    void RenderScanline(int y);

    // Upload the completed row to OpenGL.
    void UploadScanline(int y);

private:

    int m_width = 0;
    int m_height = 0;

    // RGBA image: 4 bytes per pixel.
    std::vector<unsigned char> m_pixels;

    GLuint m_renderTexture = 0;

    bool m_isRendering = false;

    // Current row being rendered.
    int m_currentScanline = -1;

    float m_progress = 0.0f;

    // Number of rows processed per update.
    int m_scanlinesPerUpdate = 4;
};
