#include <rendering/Terrain.h>
#include <stb\stb_image.h>
#include "stb\stb_image_write.h"
#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <shader/Shader.h>
#include <cmath>

#include <filesystem>
namespace fs = std::filesystem;

bool Terrain::Initialize()
{
    
        if (!GenerateTerrainMesh())
        {
            return false;
        }

        m_terrainTexture =
            loadSaveTerrainTexture("assets/textures/terrain/grass.jpg");

        if (m_terrainTexture == 0)
        {
            return false;
        }

        return true;
   
    
    
    return GenerateTerrainMesh();



}
// Generate a terrain mesh with the given width, depth, and scale taken from the height map
bool Terrain::GenerateTerrainMesh()
{

    unsigned char* t_data = stbi_load("assets/textures/terrain/Data/test_all_black_T.png",
        &heightmapWidth, &heightmapHeight, &heightmapChannels, 0);
    if (!t_data) {
        std::cout << "Failed Loading the Heightmap " << std::endl;
        return false;

    }
    std::cout << "Loading the Heightmap of size " << heightmapHeight << " X " << heightmapWidth << std::endl;

    //std::vector<float> heightMap(heightmapWidth * heightmapHeight);
    m_heightMap.clear();

    m_heightMap.resize(heightmapWidth * heightmapHeight);

    float y_Scale = 64.0f / 256.0f;
    float y_Shift = 16.0f;
    unsigned bytePerPixel = heightmapChannels;

    for (int i = 0; i < heightmapHeight; ++i) {
        for (int j = 0; j < heightmapWidth; ++j) {
            unsigned char* pixelOffset = t_data + (j + heightmapWidth * i) * bytePerPixel;

            unsigned char y = pixelOffset[0];
            m_heightMap[i * heightmapWidth + j] = static_cast<float>(y) * y_Scale - y_Shift;
        }
    }
    stbi_image_free(t_data);

    for (int i = 0; i < heightmapHeight; ++i) {
        for (int j = 0; j < heightmapWidth; ++j) {
            float x = -heightmapHeight / 2.0f + heightmapHeight * i / static_cast<float>(heightmapHeight);
            float y = m_heightMap[i * heightmapWidth + j];
            float z = -heightmapWidth / 2.0f + heightmapWidth * j / static_cast<float>(heightmapWidth);

            // Calculate normal using surrounding heights
            float hL = (j > 0) ? m_heightMap[i * heightmapWidth + (j - 1)] : y;
            float hR = (j < heightmapWidth - 1) ? m_heightMap[i * heightmapWidth + (j + 1)] : y;
            float hD = (i > 0) ? m_heightMap[(i - 1) * heightmapWidth + j] : y;
            float hU = (i < heightmapHeight - 1) ? m_heightMap[(i + 1) * heightmapWidth + j] : y;

            glm::vec3 normal = glm::normalize(glm::vec3(hL - hR, 2.0f, hD - hU)); // y is up

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
    glm::mat4 model =  glm::translate(glm::mat4(1.0f), m_position);

    shader.setMat4("model", model);

    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, m_terrainTexture);

    shader.SetUniformInt(
        "terrainTexture",
        0
    );



    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glBindVertexArray(m_VAO);


    for (unsigned strip = 0; strip < numStrips; strip++) {
        glDrawElements(GL_TRIANGLE_STRIP, vertsPerStrip + 2,
            GL_UNSIGNED_INT, (void*)(sizeof(unsigned) * (vertsPerStrip + 2) * strip)); // Correct byte offset per strip  add + 2 to vertsPerStrip

    }
    glBindVertexArray(0);

   // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// use the heightmap to get the height at a given x, z coordinate for painting and collision detection
float Terrain::GetHeightAt(
    int x,
    int z
) const
{
    if (x < 0 || x >= heightmapWidth ||
        z < 0 || z >= heightmapHeight)
    {
        return 0.0f;
    }

    return m_heightMap[
        z * heightmapWidth + x
    ];
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
    //std::vector<float> heightMap(heightmapWidth * heightmapHeight);
    m_heightMap.clear();

    m_heightMap.resize(heightmapWidth * heightmapHeight);

    for (int i = 0; i < heightmapHeight; ++i) {
        for (int j = 0; j < heightmapWidth; ++j) {
            unsigned char y = newData[i * heightmapWidth + j];
            m_heightMap[i * heightmapWidth + j] = static_cast<float>(y) * y_Scale - y_Shift;
        }
    }

    stbi_image_free(newData);

    for (int i = 0; i < heightmapHeight; ++i) {
        for (int j = 0; j < heightmapWidth; ++j) {
            float x = -heightmapHeight / 2.0f + heightmapHeight * i / static_cast<float>(heightmapHeight);
            float y = m_heightMap[i * heightmapWidth + j];
            float z = -heightmapWidth / 2.0f + heightmapWidth * j / static_cast<float>(heightmapWidth);

            float hL = (j > 0) ? m_heightMap[i * heightmapWidth + (j - 1)] : y;
            float hR = (j < heightmapWidth - 1) ? m_heightMap[i * heightmapWidth + (j + 1)] : y;
            float hD = (i > 0) ? m_heightMap[(i - 1) * heightmapWidth + j] : y;
            float hU = (i < heightmapHeight - 1) ? m_heightMap[(i + 1) * heightmapWidth + j] : y;

            glm::vec3 normal = glm::normalize(glm::vec3(hL - hR, 2.0f, hD - hU));
            // The texture tiling Factor
           

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

           
        }
    }

    // Re-upload to GPU
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(float), m_vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned), m_indices.data(), GL_STATIC_DRAW);
	// Generate new terrain mesh
	

}

bool Terrain::GetHeightAtWorldPosition(float worldX, float worldZ, float& outHeight) const
{
    if (m_heightMap.empty() ||
        heightmapWidth <= 1 ||
        heightmapHeight <= 1)
    {
        return false;
    }

    // Convert world position into terrain-local position.
    const float localX =
        worldX - m_position.x;

    const float localZ =
        worldZ - m_position.z;


    // Convert terrain-local coordinates into
    // heightmap coordinates.
    const float mapX =
        localX +
        static_cast<float>(heightmapHeight) * 0.5f;

    const float mapZ =
        localZ +
        static_cast<float>(heightmapWidth) * 0.5f;


    // Outside terrain.
    if (mapX < 0.0f ||
        mapZ < 0.0f ||
        mapX >= static_cast<float>(heightmapHeight - 1) ||
        mapZ >= static_cast<float>(heightmapWidth - 1))
    {
        return false;
    }


    const int x0 =
        static_cast<int>(std::floor(mapX));

    const int z0 =
        static_cast<int>(std::floor(mapZ));

    const int x1 = x0 + 1;
    const int z1 = z0 + 1;


    const float tx =
        mapX - static_cast<float>(x0);

    const float tz =
        mapZ - static_cast<float>(z0);


    const float h00 =
        m_heightMap[
            x0 * heightmapWidth + z0
        ];

    const float h10 =
        m_heightMap[
            x1 * heightmapWidth + z0
        ];

    const float h01 =
        m_heightMap[
            x0 * heightmapWidth + z1
        ];

    const float h11 =
        m_heightMap[
            x1 * heightmapWidth + z1
        ];


    // Bilinear interpolation.
    const float h0 =
        glm::mix(h00, h10, tx);

    const float h1 =
        glm::mix(h01, h11, tx);

    const float height =
        glm::mix(h0, h1, tz);


    // Convert local terrain height back into world Y.
    outHeight =
        height + m_position.y;

    return true;
}

bool Terrain::Raycast(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, glm::vec3& outHitPosition) const
{
    if (m_heightMap.empty())
    {
        return false;
    }

    constexpr float maxDistance = 1000.0f;
    constexpr float stepSize = 0.5f;

    glm::vec3 previousPoint =
        rayOrigin;

    bool previousValid = false;
   // float previousTerrainHeight = 0.0f;


    for (float distance = 0.0f;
        distance < maxDistance;
        distance += stepSize)
    {
        const glm::vec3 point =
            rayOrigin +
            rayDirection * distance;


        float terrainHeight = 0.0f;

        if (!GetHeightAtWorldPosition(
            point.x,
            point.z,
            terrainHeight))
        {
            previousValid = false;
            continue;
        }


        // Has the ray crossed the terrain surface?
        if (point.y <= terrainHeight)
        {
            if (!previousValid)
            {
                outHitPosition =
                    glm::vec3(
                        point.x,
                        terrainHeight,
                        point.z
                    );

                return true;
            }


            // Refine the hit position between
            // the previous and current ray points.
            glm::vec3 low =
                previousPoint;

            glm::vec3 high =
                point;

            for (int i = 0; i < 8; ++i)
            {
                const glm::vec3 mid =
                    (low + high) * 0.5f;

                float midHeight = 0.0f;

                if (!GetHeightAtWorldPosition(
                    mid.x,
                    mid.z,
                    midHeight))
                {
                    break;
                }

                if (mid.y > midHeight)
                {
                    low = mid;
                }
                else
                {
                    high = mid;
                }
            }


            float finalHeight = 0.0f;

            GetHeightAtWorldPosition(
                high.x,
                high.z,
                finalHeight
            );


            outHitPosition =
                glm::vec3(
                    high.x,
                    finalHeight,
                    high.z
                );

            return true;
        }


        previousPoint =
            point;

       // previousTerrainHeight = terrainHeight;

        previousValid = true;
    }


    return false;
}
// raise the terrain at a given world position with a given radius and strength
void Terrain::RaiseTerrain(const glm::vec3& worldPosition, float radius, float strength)
{
    if (m_heightMap.empty() ||
        heightmapWidth <= 0 ||
        heightmapHeight <= 0)
    {
        return;
    }

    // Convert world position into terrain-local position.
    const float localX =
        worldPosition.x - m_position.x;

    const float localZ =
        worldPosition.z - m_position.z;


    // Convert local terrain position into heightmap coordinates.
    const float mapX =
        localX +
        static_cast<float>(heightmapHeight) * 0.5f;

    const float mapZ =
        localZ +
        static_cast<float>(heightmapWidth) * 0.5f;


    const int centerX =
        static_cast<int>(
            std::round(mapX)
            );

    const int centerZ =
        static_cast<int>(
            std::round(mapZ)
            );


    const int brushRadius =
        static_cast<int>(
            std::ceil(radius)
            );


    for (int z = -brushRadius;
        z <= brushRadius;
        ++z)
    {
        for (int x = -brushRadius;
            x <= brushRadius;
            ++x)
        {
            const int sampleX =
                centerX + x;

            const int sampleZ =
                centerZ + z;


            if (sampleX < 0 ||
                sampleX >= heightmapHeight ||
                sampleZ < 0 ||
                sampleZ >= heightmapWidth)
            {
                continue;
            }


            const float distance =
                std::sqrt(
                    static_cast<float>(
                        x * x + z * z
                        )
                );


            if (distance > radius)
            {
                continue;
            }


            const float t =
                1.0f -
                distance / radius;


            // Smoothstep falloff.
            const float falloff =
                t * t *
                (3.0f - 2.0f * t);


            const std::size_t index =
                static_cast<std::size_t>(
                    sampleX *
                    heightmapWidth +
                    sampleZ
                    );


            m_heightMap[index] +=
                strength * falloff;
        }
    }
}
// lower the terrain at a given world position with a given radius and strength
void Terrain::LowerTerrain(const glm::vec3& worldPosition, float radius, float strength)
{
    if (m_heightMap.empty() ||
        heightmapWidth <= 0 ||
        heightmapHeight <= 0)
    {
        return;
    }

    // Convert world position into terrain-local position.
    const float localX =
        worldPosition.x - m_position.x;

    const float localZ =
        worldPosition.z - m_position.z;


    // Convert local terrain position into heightmap coordinates.
    const float mapX =
        localX +
        static_cast<float>(heightmapHeight) * 0.5f;

    const float mapZ =
        localZ +
        static_cast<float>(heightmapWidth) * 0.5f;


    const int centerX =
        static_cast<int>(
            std::round(mapX)
            );

    const int centerZ =
        static_cast<int>(
            std::round(mapZ)
            );


    const int brushRadius =
        static_cast<int>(
            std::ceil(radius)
            );


    for (int z = -brushRadius;
        z <= brushRadius;
        ++z)
    {
        for (int x = -brushRadius;
            x <= brushRadius;
            ++x)
        {
            const int sampleX =
                centerX + x;

            const int sampleZ =
                centerZ + z;


            if (sampleX < 0 ||
                sampleX >= heightmapHeight ||
                sampleZ < 0 ||
                sampleZ >= heightmapWidth)
            {
                continue;
            }


            const float distance =
                std::sqrt(
                    static_cast<float>(
                        x * x + z * z
                        )
                );


            if (distance > radius)
            {
                continue;
            }


            const float t =
                1.0f -
                distance / radius;


            // Smoothstep falloff.
            const float falloff =
                t * t *
                (3.0f - 2.0f * t);


            const std::size_t index =
                static_cast<std::size_t>(
                    sampleX *
                    heightmapWidth +
                    sampleZ
                    );


            m_heightMap[index] -= strength * falloff;
        }
    }
  
}

void Terrain::UpdateMeshFromHeightMap()
{
    if (m_heightMap.empty() ||
        m_vertices.empty())
    {
        return;
    }

    std::size_t vertexIndex = 0;

    for (int i = 0;
        i < heightmapHeight;
        ++i)
    {
        for (int j = 0;
            j < heightmapWidth;
            ++j)
        {
            const std::size_t heightIndex =
                static_cast<std::size_t>(
                    i * heightmapWidth + j
                    );

            // Each vertex = 8 floats:
            // x y z nx ny nz u v
            const std::size_t base =
                vertexIndex * 8;

            m_vertices[base + 1] =
                m_heightMap[heightIndex];

            ++vertexIndex;
        }
    }


    glBindBuffer(
        GL_ARRAY_BUFFER,
        m_VBO
    );

    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        m_vertices.size() *
        sizeof(float),
        m_vertices.data()
    );

    glBindBuffer(
        GL_ARRAY_BUFFER,
        0
    );
}

void Terrain::Destroy()
{
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteTextures(1, &m_terrainTexture);
}
