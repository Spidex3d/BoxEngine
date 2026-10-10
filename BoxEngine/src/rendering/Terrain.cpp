#include <rendering/Terrain.h>
#include <stb\stb_image.h>
#include "stb\stb_image_write.h"
#include <glm\glm.hpp>
#include <iostream>
#include <shader/Shader.h>


#include <filesystem>
namespace fs = std::filesystem;

bool Terrain::Initialize()
{
    return GenerateTerrainMesh();
}
// Generate a terrain mesh with the given width, depth, and scale taken from the height map
bool Terrain::GenerateTerrainMesh()
{

    //unsigned char* t_data = stbi_load("Textures/Terrain/Data/test_all_black_s.png",
    unsigned char* t_data = stbi_load("assets/textures/terrain/Data/test_all_black_T.png",
        &heightmapWidth, &heightmapHeight, &heightmapChannels, 0);
    if (!t_data) {
        std::cout << "Failed Loading the Heightmap " << std::endl;
        return false;

    }
    std::cout << "Loading the Heightmap of size " << heightmapHeight << " X " << heightmapWidth << std::endl;

    std::vector<float> heightMap(heightmapWidth * heightmapHeight);
    float y_Scale = 64.0f / 256.0f;
    float y_Shift = 16.0f;
    unsigned bytePerPixel = heightmapChannels;

    for (int i = 0; i < heightmapHeight; ++i) {
        for (int j = 0; j < heightmapWidth; ++j) {
            unsigned char* pixelOffset = t_data + (j + heightmapWidth * i) * bytePerPixel;

            unsigned char y = pixelOffset[0];
            heightMap[i * heightmapWidth + j] = static_cast<float>(y) * y_Scale - y_Shift;
        }
    }
    stbi_image_free(t_data);

    for (int i = 0; i < heightmapHeight; ++i) {
        for (int j = 0; j < heightmapWidth; ++j) {
            float x = -heightmapHeight / 2.0f + heightmapHeight * i / static_cast<float>(heightmapHeight);
            float y = heightMap[i * heightmapWidth + j];
            float z = -heightmapWidth / 2.0f + heightmapWidth * j / static_cast<float>(heightmapWidth);

            // Calculate normal using surrounding heights
            float hL = (j > 0) ? heightMap[i * heightmapWidth + (j - 1)] : y;
            float hR = (j < heightmapWidth - 1) ? heightMap[i * heightmapWidth + (j + 1)] : y;
            float hD = (i > 0) ? heightMap[(i - 1) * heightmapWidth + j] : y;
            float hU = (i < heightmapHeight - 1) ? heightMap[(i + 1) * heightmapWidth + j] : y;

            glm::vec3 normal = glm::normalize(glm::vec3(hL - hR, 2.0f, hD - hU)); // y is up

            //float u = (j / float(heightmapWidth - 1)) * tilingFactor;
            //float v = (i / float(heightmapHeight - 1)) * tilingFactor;
            const float u =
                static_cast<float>(j) /
                static_cast<float>(
                    heightmapWidth - 1
                    );

            const float v =
                static_cast<float>(i) /
                static_cast<float>(
                    heightmapHeight - 1
                    );

            // Push position
            m_vertices.push_back(x);
            m_vertices.push_back(y);
            m_vertices.push_back(z);

            // Push normal
            m_vertices.push_back(normal.x);
            m_vertices.push_back(normal.y);
            m_vertices.push_back(normal.z);
            // Push texture coordinates
            m_vertices.push_back(u);
            m_vertices.push_back(v);
            
        }
    }

    std::cout << "Loaded " << m_vertices.size() / 8 << " Vertices" << std::endl;

    for (unsigned i = 0; i < heightmapHeight - 1; i += rez) {
        for (unsigned j = 0; j < heightmapWidth; j += rez) {
            m_indices.push_back(
                j +
                heightmapWidth *
                (i + rez)
            );

            m_indices.push_back(
                j +
                heightmapWidth *
                i
            );

            /*for (unsigned k = 0; k < 2; k++) {
                m_indices.push_back(j + heightmapWidth * (i + k * rez));
            }*/
        }
    }


    std::cout << "Loaded " << m_indices.size() << " Indices" << std::endl;

    numStrips = (heightmapHeight - 1) / rez;
    vertsPerStrip = (heightmapWidth / rez) * 2 - 2; // Number of indices per triangle strip

    std::cout << "Created lattice of " << numStrips << " Strips with " << vertsPerStrip << " Triangles in each" << std::endl;
    std::cout << "Created " << numStrips * vertsPerStrip << " Triangles total" << std::endl;

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(float),
        m_vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);


    // -------------------------------------------------
    // Position
    // -------------------------------------------------

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(0));

    glEnableVertexAttribArray(0);


    // -------------------------------------------------
    // Normal
    // -------------------------------------------------

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));

    glEnableVertexAttribArray(1);


    // -------------------------------------------------
    // Texture coordinates
    // -------------------------------------------------

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(6 * sizeof(float)));

    glEnableVertexAttribArray(2);


    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);


    return true;
}

void Terrain::RenderTerrain(Shader& shader, const glm::mat4& view, const glm::mat4& projection)
{
	// sort out the shader and set the uniforms
    shader.Use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setMat4("model", glm::mat4(1.0f));

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glBindVertexArray(m_VAO);


    for (unsigned strip = 0; strip < numStrips; strip++) {
        glDrawElements(GL_TRIANGLE_STRIP, vertsPerStrip + 2,
            GL_UNSIGNED_INT, (void*)(sizeof(unsigned) * (vertsPerStrip + 2) * strip)); // Correct byte offset per strip  add + 2 to vertsPerStrip

    }
    glBindVertexArray(0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

std::vector<TerrainImage> Terrain::LoadTerrainTextureFromFolder(const std::string & folderPath)
{
    std::vector<TerrainImage> textures;
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            if (ext == ".png" || ext == ".jpg" || ext == ".bmp") {
                GLuint id = loadSaveTerrainTexture(entry.path().string());
                textures.push_back({ id, entry.path().string() });
            }
        }
    }
    return textures;


    return std::vector<TerrainImage>();
}

unsigned int Terrain::loadSaveTerrainTexture(const std::string& terrainPath)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(terrainPath.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = (nrComponents == 1) ? GL_RED :
            (nrComponents == 3) ? GL_RGB :
            (nrComponents == 4) ? GL_RGBA : GL_RGB;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(data);
    }
    else {
        std::cout << "Texture failed to load: " << terrainPath << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void Terrain::ReloadTerrainMeshFromFile(const std::string& heightmapPath)
{
    // Clear existing vertex/index buffers
    m_vertices.clear();
    m_indices.clear();

    int channels;
    unsigned char* newData = stbi_load(heightmapPath.c_str(), &heightmapWidth, &heightmapHeight, &channels, 1);
    if (!newData) {
        std::cerr << "Failed to reload heightmap: " << heightmapPath << "\n";
        return;
    }

    // Rebuild vertex data (same logic you used in your constructor)
    float y_Scale = 64.0f / 256.0f;
    float y_Shift = 16.0f;
    std::vector<float> heightMap(heightmapWidth * heightmapHeight);

    for (int i = 0; i < heightmapHeight; ++i) {
        for (int j = 0; j < heightmapWidth; ++j) {
            unsigned char y = newData[i * heightmapWidth + j];
            heightMap[i * heightmapWidth + j] = static_cast<float>(y) * y_Scale - y_Shift;
        }
    }

    stbi_image_free(newData);

    for (int i = 0; i < heightmapHeight; ++i) {
        for (int j = 0; j < heightmapWidth; ++j) {
            float x = -heightmapHeight / 2.0f + heightmapHeight * i / static_cast<float>(heightmapHeight);
            float y = heightMap[i * heightmapWidth + j];
            float z = -heightmapWidth / 2.0f + heightmapWidth * j / static_cast<float>(heightmapWidth);

            float hL = (j > 0) ? heightMap[i * heightmapWidth + (j - 1)] : y;
            float hR = (j < heightmapWidth - 1) ? heightMap[i * heightmapWidth + (j + 1)] : y;
            float hD = (i > 0) ? heightMap[(i - 1) * heightmapWidth + j] : y;
            float hU = (i < heightmapHeight - 1) ? heightMap[(i + 1) * heightmapWidth + j] : y;

            glm::vec3 normal = glm::normalize(glm::vec3(hL - hR, 2.0f, hD - hU));
            // The texture tiling Factor
           // float u = (j / float(heightmapWidth - 1)) * tilingFactor;
            //float v = (i / float(heightmapHeight - 1)) * tilingFactor;

            const float u = static_cast<float>(j) / static_cast<float>(heightmapWidth - 1);

            const float v = static_cast<float>(i) / static_cast<float>(heightmapHeight - 1);

            m_vertices.push_back(x);
            m_vertices.push_back(y);
            m_vertices.push_back(z);
            m_vertices.push_back(normal.x);
            m_vertices.push_back(normal.y);
            m_vertices.push_back(normal.z);
            m_vertices.push_back(u);
            m_vertices.push_back(v);
        }
    }

    for (unsigned i = 0; i < heightmapHeight - 1; i += rez) {
        for (unsigned j = 0; j < heightmapWidth; j += rez) {
            m_indices.push_back(
                j +
                heightmapWidth *
                (i + rez)
            );

            m_indices.push_back(
                j +
                heightmapWidth *
                i
            );

            /*for (unsigned k = 0; k < 2; ++k) {
                m_indices.push_back(j + heightmapWidth * (i + k * rez));
            }*/
        }
    }

    // Re-upload to GPU
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(float), m_vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned), m_indices.data(), GL_STATIC_DRAW);
	// Generate new terrain mesh
	

}

void Terrain::Destroy()
{
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteTextures(1, &m_terrainTexture);
}
