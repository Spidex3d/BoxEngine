#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <filesystem>

class Shader;

struct SkyTexture
{
    GLuint id = 0;
    std::string path;
    GLuint frontFaceTexID = 0;
};

class Sky
{
public:

    bool Initialize();

    bool LoadSkyFolder(const std::string& folderPath);

    void RenderSkyBox(
        Shader& shader,
        const glm::mat4& view,
        const glm::mat4& projection
    );

    void Destroy();

    const std::vector<SkyTexture>&
        GetSkyTextures() const
    {
        return m_skyTextures;
    }

    void SetSkyTexture(std::size_t index);

    std::size_t GetSelectedSkyIndex() const
    {
        return m_selectedSkyIndex;
    }

    bool HasLoadedSkies() const
    {
        return !m_skyTextures.empty();
    }

    void ClearSky()
    {
        m_cubemapTexture = 0;
    }

    bool HasActiveSky() const
    {
        return m_cubemapTexture != 0;
    }

	// progess bar for loading sky textures
    bool BeginLoadSkyFolder(const std::string& folderPath);

    bool LoadNextSky();

    bool IsLoading() const
    {
        return m_isLoading;
    }

    float GetLoadProgress() const
    {
        return m_loadProgress;
    }



private:

    bool CreateSkyBoxMesh();

    unsigned char* ExtractFace(
        const unsigned char* src,
        int srcWidth,
        int srcHeight,
        int channels,
        int faceRow,
        int faceCol,
        int faceSize = 512
    );

    std::vector<SkyTexture>
        loadSkyTextureFromFolder(
            const std::string& folderPath
        );

private:

    GLuint m_VAO = 0;
    GLuint m_VBO = 0;
    GLuint m_EBO = 0;

    GLuint m_cubemapTexture = 0;

    std::vector<SkyTexture>m_skyTextures;

    std::size_t m_selectedSkyIndex = 0;

	// progress bar for loading sky textures
    std::vector<std::filesystem::path>m_pendingSkyFiles;

    std::size_t m_nextSkyFile = 0;

    bool m_isLoading = false;

    float m_loadProgress = 0.0f;


};
