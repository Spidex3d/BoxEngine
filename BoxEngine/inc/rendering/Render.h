#pragma once
#include <glad/glad.h>
#include <vector>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// ============================================
// BoxRenderer v0.1 18/11/26
// BoxRenderer v0.2 — BVH Acceleration. 19/11/26
// Progressive CPU Image Renderer
// ============================================
struct RenderTriangle
{
    glm::vec3 a{ 0.0f };
    glm::vec3 b{ 0.0f };
    glm::vec3 c{ 0.0f };

    glm::vec3 color{ 0.8f, 0.8f, 0.8f };

    // UV coordinates for triangle corners.
    glm::vec2 uvA{ 0.0f };
    glm::vec2 uvB{ 0.0f };
    glm::vec2 uvC{ 0.0f };

    // -1 means no texture assigned.
    int textureIndex = -1;
};


// ============================================
// CPU TEXTURE DATA
// ============================================

struct RenderTexture
{
    int width = 0;
    int height = 0;

    // RGBA pixels, four bytes per pixel.
    // Row zero represents the bottom of
    // the OpenGL texture.
    std::vector<unsigned char> pixels;

    bool IsValid() const
    {
        return width > 0 &&
            height > 0 &&
            pixels.size() ==
            static_cast<std::size_t>(width) *
            static_cast<std::size_t>(height) * 4;
    }
};


// ============================================
// AXIS ALIGNED BOUNDING BOX
// ============================================

struct RenderAABB
{
    glm::vec3 min{ 0.0f };
    glm::vec3 max{ 0.0f };

    bool valid = false;
};


// ============================================
// BVH NODE
// ============================================

struct BVHNode
{
    RenderAABB bounds;

    // Child node indices.
    // -1 means no child.
    int left = -1;
    int right = -1;

    // Range in our triangle index array.
    int start = 0;
    int count = 0;

    // A leaf contains triangles rather than
    // child nodes.
    bool IsLeaf() const
    {
        return left == -1 && right == -1;
    }
};



// A copy of the scene data used during rendering.
struct RenderScene
{
    glm::mat4 view{ 1.0f };
    glm::mat4 projection{ 1.0f };

    std::vector<RenderTriangle> triangles;

   // ============================================
   // LIGHTING
   // ============================================

   // Direction pointing FROM the surface
   // TOWARDS the light.
    glm::vec3 lightDirection{ -0.5f, 1.0f, 0.7f };
    float lightIntensity = 1.0f;

    // CPU copies of scene albedo textures.
    std::vector<RenderTexture> textures;
};

class Render
{
public:

    Render() = default;
    ~Render() = default;

    bool Initialize(int width, int height);

    void SetScene(RenderScene scene);
    
    std::size_t GetBVHNodeCount() const;

    void StartRender();
    void Update();
    void CancelRender();

    bool IsRendering() const;
    float GetProgress() const;

  
    GLuint GetRenderTexture() const;

    // ============================================
    // RENDER TIMER
    // ============================================

    double GetRenderTime() const;

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

    // ============================================
    // BVH ACCELERATION
    // ============================================

    // Bounding box containing the entire scene.
    RenderAABB m_sceneBounds;

    // Expand a box to include a point.
    void ExpandBounds(
        RenderAABB& bounds,
        const glm::vec3& point) const;

    // Test a ray against a bounding box.
    bool IntersectAABB(
        const glm::vec3& rayOrigin,
        const glm::vec3& rayDirection,
        const RenderAABB& bounds,
        float maxDistance) const;
    
    // ============================================
    // BVH HIERARCHY
    // ============================================

    // All the BVH nodes.
    std::vector<BVHNode> m_bvhNodes;

    // Triangle indices, arranged by BVH grouping.
    std::vector<int> m_triangleIndices;

    // Maximum triangles stored in one leaf.
    int m_maxLeafTriangles = 4;

    // Build the entire BVH.
    void BuildBVH();

    // Recursively build one node and its children.
    int BuildBVHNode(int start, int end);


    // ============================================
    // BVH RAY TRAVERSAL
    // ============================================

    // Find the closest triangle hit by a ray.
    bool TraceClosest(
        const glm::vec3& rayOrigin,
        const glm::vec3& rayDirection,
        float& closestDistance,
        int& closestTriangleIndex) const;

    // Return true on the first triangle hit.
    // Used for shadow rays.
    bool TraceAny(
        const glm::vec3& rayOrigin,
        const glm::vec3& rayDirection) const;

	// Sample a texture at given UV coordinates.
    glm::vec3 SampleTexture(
        const RenderTexture& texture,
        const glm::vec2& uv) const;

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


    // ============================================
    // RENDER TIMER
    // ============================================

    using RenderClock = std::chrono::steady_clock;

    RenderClock::time_point m_renderStartTime{};

    double m_renderElapsedSeconds = 0.0;
};
