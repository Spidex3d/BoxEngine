#include "rendering/Sky.h"
#include <Helpers.h>
#include <shader/Shader.h>
#include "stb/stb_image.h"
#include <iostream>
#include <vector>

#include <filesystem>
namespace fs = std::filesystem;


bool Sky::Initialize()
{
	
    return CreateSkyBoxMesh();
}
// progrsss bar for loading sky textures
bool Sky::BeginLoadSkyFolder(const std::string& folderPath)
{
    m_pendingSkyFiles.clear();

    m_nextSkyFile = 0;

    m_loadProgress = 0.0f;

    m_isLoading = false;


    if (!std::filesystem::exists(folderPath))
    {
        std::cout
            << "Sky folder does not exist: "
            << folderPath
            << "\n";

        return false;
    }


    for (const auto& entry :
        std::filesystem::directory_iterator(folderPath))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }

        const std::string ext =
            entry.path().extension().string();

        if (ext != ".png" &&
            ext != ".jpg" &&
            ext != ".bmp")
        {
            continue;
        }

        m_pendingSkyFiles.push_back(
            entry.path()
        );
    }


    if (m_pendingSkyFiles.empty())
    {
        std::cout
            << "No sky images found in: "
            << folderPath
            << "\n";

        return false;
    }


    m_isLoading = true;

    std::cout
        << "Sky loader found "
        << m_pendingSkyFiles.size()
        << " images\n";

    return true;
}

bool Sky::LoadNextSky()
{
    if (!m_isLoading)
    {
        return false;
    }


    if (m_nextSkyFile >=
        m_pendingSkyFiles.size())
    {
        m_isLoading = false;
        m_loadProgress = 1.0f;

        return false;
    }


    const std::string imagePath =
        m_pendingSkyFiles[
            m_nextSkyFile
        ].string();


    int width = 0;
    int height = 0;
    int channels = 0;


    unsigned char* data =
        stbi_load(
            imagePath.c_str(),
            &width,
            &height,
            &channels,
            0
        );


    if (!data)
    {
        std::cout
            << "Failed to load sky image: "
            << imagePath
            << "\n";

        ++m_nextSkyFile;

        m_loadProgress =
            static_cast<float>(
                m_nextSkyFile
                ) /
            static_cast<float>(
                m_pendingSkyFiles.size()
                );

        return true;
    }


    const int faceSize = 512;


    const std::vector<
        std::pair<int, int>>
        facePositions =
    {
        {0, 1}, // Top
        {1, 0}, // Left
        {1, 1}, // Front
        {1, 2}, // Right
        {1, 3}, // Back
        {2, 1}  // Bottom
    };


    const GLenum faceTargets[6] =
    {
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
    };


    GLuint cubeMapID = 0;

    glGenTextures(
        1,
        &cubeMapID
    );

    glBindTexture(
        GL_TEXTURE_CUBE_MAP,
        cubeMapID
    );


    GLuint previewTexID = 0;


    for (int i = 0;
        i < 6;
        ++i)
    {
        const auto [row, col] =
            facePositions[i];


        unsigned char* face =
            ExtractFace(
                data,
                width,
                height,
                channels,
                row,
                col,
                faceSize
            );


        glTexImage2D(
            faceTargets[i],
            0,
            channels == 4
            ? GL_RGBA
            : GL_RGB,
            faceSize,
            faceSize,
            0,
            channels == 4
            ? GL_RGBA
            : GL_RGB,
            GL_UNSIGNED_BYTE,
            face
        );


        // Front preview image.
        if (i == 2)
        {
            glGenTextures(
                1,
                &previewTexID
            );

            glBindTexture(
                GL_TEXTURE_2D,
                previewTexID
            );

            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                channels == 4
                ? GL_RGBA
                : GL_RGB,
                faceSize,
                faceSize,
                0,
                channels == 4
                ? GL_RGBA
                : GL_RGB,
                GL_UNSIGNED_BYTE,
                face
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
        }


        delete[] face;
    }


    glBindTexture(
        GL_TEXTURE_CUBE_MAP,
        cubeMapID
    );


    glTexParameteri(
        GL_TEXTURE_CUBE_MAP,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_CUBE_MAP,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_CUBE_MAP,
        GL_TEXTURE_WRAP_S,
        GL_CLAMP_TO_EDGE
    );

    glTexParameteri(
        GL_TEXTURE_CUBE_MAP,
        GL_TEXTURE_WRAP_T,
        GL_CLAMP_TO_EDGE
    );

    glTexParameteri(
        GL_TEXTURE_CUBE_MAP,
        GL_TEXTURE_WRAP_R,
        GL_CLAMP_TO_EDGE
    );


    stbi_image_free(
        data
    );


    m_skyTextures.push_back(
        {
            cubeMapID,
            imagePath,
            previewTexID
        }
    );


    // First loaded sky becomes active.
    if (m_skyTextures.size() == 1)
    {
        m_cubemapTexture =
            cubeMapID;

        m_selectedSkyIndex = 0;
    }


    ++m_nextSkyFile;


    m_loadProgress =
        static_cast<float>(
            m_nextSkyFile
            ) /
        static_cast<float>(
            m_pendingSkyFiles.size()
            );


    if (m_nextSkyFile >=
        m_pendingSkyFiles.size())
    {
        m_isLoading = false;

        m_loadProgress = 1.0f;

        std::cout
            << "Finished loading skies\n";
    }


    return true;
}

// end progress bar for loading sky textures

bool Sky::LoadSkyFolder(const std::string& folderPath)
{
    m_skyTextures = loadSkyTextureFromFolder(folderPath);

    if (m_skyTextures.empty())
    {
        std::cout
            << "No sky textures found in: "
            << folderPath
            << "\n";

        return false;
    }


    m_selectedSkyIndex = 0;
    // For our first test,
    // automatically use the first sky.
    m_cubemapTexture = m_skyTextures[0].id;

    std::cout
        << "Loaded "
        << m_skyTextures.size()
        << " sky textures\n";

    std::cout
        << "Using sky: "
        << m_skyTextures[0].path
        << "\n";

    return true;
}

void Sky::SetSkyTexture(std::size_t index)
{
    if (index >= m_skyTextures.size())
    {
        return;
    }

    m_selectedSkyIndex = index;

    m_cubemapTexture = m_skyTextures[index].id;
}


void Sky::RenderSkyBox(Shader& shader, const glm::mat4& view, const glm::mat4& projection)
{
    if (m_cubemapTexture == 0)
    {
        return;
    }

    // -----------------------------------------
    // Skybox render state
    // -----------------------------------------

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    // We are inside the cube.
    glDisable(GL_CULL_FACE);


    // Remove camera translation.
    const glm::mat4 skyView =
        glm::mat4(
            glm::mat3(view)
        );


    shader.Use();

    shader.SetUniformInt(
        "skybox",
        0
    );

    shader.setMat4(
        "view",
        skyView
    );

    shader.setMat4(
        "projection",
        projection
    );


    glActiveTexture(
        GL_TEXTURE0
    );

    glBindTexture(
        GL_TEXTURE_CUBE_MAP,
        m_cubemapTexture
    );


    glBindVertexArray(
        m_VAO
    );

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);


    // -----------------------------------------
    // Restore normal scene state
    // -----------------------------------------

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void Sky::Destroy()
{
    if (m_VBO != 0)
    {
        glDeleteBuffers(1, &m_VBO);

        m_VBO = 0;
    }

    if (m_VAO != 0)
    {
        glDeleteVertexArrays(1, &m_VAO);

        m_VAO = 0;
    }

    if (m_EBO != 0)
    {
        glDeleteBuffers(1, &m_EBO);

        m_EBO = 0;
    }

    if (m_cubemapTexture != 0)
    {
        glDeleteTextures(1, &m_cubemapTexture);

        m_cubemapTexture = 0;
    }
}

bool Sky::CreateSkyBoxMesh()
{
   
    // Skybox rendering comes next.
    float SkyBoxVertices[] =
    {
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f
    };

    unsigned int SkyBoxIndices[] =
    {
        // Right
        1, 2, 6,
        6, 5, 1,
        // Left
        0, 4, 7,
        7, 3, 0,
        // Top
        4, 5, 6,
        6, 7, 4,
        // Bottom
        0, 3, 2,
        2, 1, 0,
        // Back
        0, 1, 5,
        5, 4, 0,
        // Front
        3, 7, 6,
        6, 2, 3
    };

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SkyBoxVertices), &SkyBoxVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(SkyBoxIndices), &SkyBoxIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return true;
}

// Utility to copy one face from source to destination
unsigned char* Sky::ExtractFace(const unsigned char* src, int srcWidth, int srcHeight, int channels,
        int faceRow, int faceCol, int faceSize){

    unsigned char* face = new unsigned char[faceSize * faceSize * channels];

    for (int y = 0;
        y < faceSize;
        ++y)
    {
        for (int x = 0;
            x < faceSize;
            ++x)
        {
            const int srcX =
                faceCol * faceSize + x;

            const int srcY =
                faceRow * faceSize + y;

            for (int c = 0;
                c < channels;
                ++c)
            {
                face[
                    (y * faceSize + x) *
                        channels + c
                ] =
                    src[
                        (srcY * srcWidth + srcX) *
                            channels + c
                    ];
            }
        }
    }

    return face;
}


std::vector<SkyTexture> Sky::loadSkyTextureFromFolder(const std::string& folderPath) {
    
    std::vector<SkyTexture> m_skyTextures;

    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        if (!entry.is_regular_file()) continue;
        std::string ext = entry.path().extension().string();
        if (ext != ".png" && ext != ".jpg" && ext != ".bmp") continue;

        std::string imagePath = entry.path().string();
        int width, height, channels;
        unsigned char* data = stbi_load(imagePath.c_str(), &width, &height, &channels, 0);
        if (!data) {
            std::cout << "Failed to load image: " << imagePath << "\n";
            continue;
        }

        const int faceSize = 512;
        std::vector<std::pair<int, int>> facePositions = {
            {0, 1}, // Top
            {1, 0}, // Left
            {1, 1}, // Front
            {1, 2}, // Right
            {1, 3}, // Back
            {2, 1}  // Bottom
        };

        GLenum faceTargets[6] = {
            GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
        };

        GLuint cubeMapID;
        glGenTextures(1, &cubeMapID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapID);

        GLuint previewTexID = 0;

        for (int i = 0; i < 6; ++i) {
            auto [row, col] = facePositions[i];
            unsigned char* face = ExtractFace(data, width, height, channels, row, col, faceSize);

            // Upload to cubemap
            glTexImage2D(faceTargets[i], 0,
                channels == 4 ? GL_RGBA : GL_RGB,
                faceSize, faceSize, 0,
                channels == 4 ? GL_RGBA : GL_RGB,
                GL_UNSIGNED_BYTE, face);

            // Also create preview texture from FRONT face
            if (i == 3) { // Front face (index 2) right face (index 3)
                glGenTextures(1, &previewTexID);
                glBindTexture(GL_TEXTURE_2D, previewTexID);
                glTexImage2D(GL_TEXTURE_2D, 0,
                    channels == 4 ? GL_RGBA : GL_RGB,
                    faceSize, faceSize, 0,
                    channels == 4 ? GL_RGBA : GL_RGB,
                    GL_UNSIGNED_BYTE, face);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }

            delete[] face;
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        stbi_image_free(data);

        m_skyTextures.push_back({ cubeMapID, imagePath, previewTexID });
    }

    return m_skyTextures;
}