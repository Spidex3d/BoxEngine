#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>




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

    bool LoadSkyFolder(
        const std::string& folderPath
    );

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

    std::vector<SkyTexture>
        m_skyTextures;
};

//struct SkyTexture {
//    GLuint id;
//    //GLuint textureID;
//    std::string path;
//    GLuint frontFaceTexID;
//
//};
//
//class Sky
//{
//public:
//    bool Initialize();
//    void RenderSkyBox(Shader& shader, const glm::mat4& view, const glm::mat4& projection);
//
//    void Destroy();
//
//private:
//    bool CreateSkyBoxMesh();
//
//    unsigned char* ExtractFace(const unsigned char* src, int srcWidth, int srcHeight, int channels,
//		int faceRow, int faceCol, int faceSize = 512);
//
//   
//
//    std::vector<SkyTexture> loadSkyTextureFromFolder(const std::string& folderPath);
//
//    //bool LoadCubemap();
//
//private:
//    GLuint m_VAO = 0;
//    GLuint m_VBO = 0;
//	GLuint m_EBO = 0;
//    GLuint m_cubemapTexture = 0;
//};
