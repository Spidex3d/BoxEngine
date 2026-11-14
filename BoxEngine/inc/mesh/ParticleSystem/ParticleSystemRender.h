#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <array>
#include <cstddef>
#include <map>
#include <memory>
#include <tuple>
#include <vector>
#include <filesystem>
#include <string>

#include <fileManager/mbxManager.h>

#include <mesh/ParticleSystem/GrassBlade.h>

class Entity;
class Shader;
class GrassClump;

// Each scatter entry owns its own two grass meshes and GPU instance buffers.
class ParticleSystemRender
{
public:
    ParticleSystemRender() = default;
    ~ParticleSystemRender() = default;

    // Existing interface retained: BoxEngine call does not change.
    void RenderGrass(
        const std::vector<std::unique_ptr<Entity>>& entities,
        GrassBlade& grassBlade,
        GrassBlade& brokenGrassBlade,
        const GrassClump& grassClump,
        Shader& shader,
        const glm::mat4& view,
        const glm::mat4& projection
    );

    void ClearGrassCache();

    // ========================================
    // Rock Asset Loading
    // ========================================

    bool LoadSmallRock();

    bool HasSmallRock() const
    {
        return m_smallRockLoaded;
    }
    // Create GPU buffers for our saved rock.
    bool CreateSmallRockBuffers();

    // Draw four rocks on each selected scatter face.
    void RenderRocks(
        const std::vector<std::unique_ptr<Entity>>& entities,
        Shader& shader,
        const glm::mat4& view,
        const glm::mat4& projection,
        const glm::vec3& cameraPosition,
        GLuint rockAlbedoTexture
    );

    std::filesystem::path GetSmallRockAlbedoPath() const;

    // ============================================
    // ROCK LIBRARY
    // ============================================

    bool LoadRockLibrary(const std::filesystem::path& folderPath);

    // Create OpenGL buffers for every rock
    // in our Rock Library.
    bool CreateRockLibraryBuffers();

    std::size_t GetRockLibraryCount() const
    {
        return m_rockLibrary.size();
    }

    // Release GPU buffers on shutdown.
    void ClearRockCache();



private:
    using GrassKey = std::tuple<int, std::size_t, std::size_t>;

    struct GrassVariant
    {
        std::unique_ptr<GrassBlade> normal;
        std::unique_ptr<GrassBlade> broken;
        std::array<float, 4> shape{};
        bool geometryReady = false;
        bool forceUpload = true;

        // Last matrices retained on the GPU.
        std::vector<glm::mat4> cachedNormal;
        std::vector<glm::mat4> cachedBroken;

        // Rebuilt each frame for comparison, not uploaded unless changed.
        std::vector<glm::mat4> pendingNormal;
        std::vector<glm::mat4> pendingBroken;
    };

    std::map<GrassKey, GrassVariant> m_grassVariants;

    // ========================================
    // ROCK ASSET CACHE
    // ========================================

    // Our first saved rock model.
    MBXImportData m_smallRockAsset;

    // ============================================
    // ROCK ASSET
    // ============================================

    struct RockAsset
    {
        std::string name;
        std::filesystem::path filePath;

        MBXImportData data;

        // =====================================
        // OPENGL BUFFERS
        // =====================================

        GLuint VAO = 0;
        GLuint VBO = 0;
        GLuint instanceVBO = 0;

        GLsizei vertexCount = 0;

        bool gpuReady = false;

        // =====================================
        // INSTANCE MATRICES
        // =====================================

        std::vector<glm::mat4> instanceMatrices;
    };

    // ============================================
    // ROCK / GRASS EXCLUSION
      // ============================================

    struct RockExclusionZone
    {
        int entityID = 0;
        std::size_t faceIndex = 0;

        glm::vec3 position{ 0.0f };
        float radius = 0.0f;
    };

    // Rebuilt from the rock instances each frame.
    std::vector<RockExclusionZone> m_rockExclusionZones;

    // All loaded rock models.
    std::vector<RockAsset> m_rockLibrary;

    // True once the MBX file has loaded.
    bool m_smallRockLoaded = false;

    // ============================================
    // ROCK GPU BUFFERS
    // ============================================

    GLuint m_rockVAO = 0;
    GLuint m_rockVBO = 0;
    GLuint m_rockInstanceVBO = 0;

	//int rockIndex = 0;
     // int rocksPerFace = 0;

    GLsizei m_rockVertexCount = 0;

    bool m_rockBuffersReady = false;

};

