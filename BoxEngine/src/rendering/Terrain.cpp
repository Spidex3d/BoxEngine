#include <rendering/Terrain.h>
#include <stb\stb_image.h>
#include "stb\stb_image_write.h"
#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <shader/Shader.h>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <miniBoxLog.h>

#include <cstring>
#include <cmath>
#include <utility>
#include <fstream>
#include <cstdint>
#include <limits>
#include <system_error>
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


        m_grassTexture =
            loadSaveTerrainTexture(
                "assets/textures/terrain/grass.jpg"
            );

        m_rockTexture =
            loadSaveTerrainTexture(
                "assets/textures/terrain/concrete.jpg"
            );

        m_dirtTexture =
            loadSaveTerrainTexture(
                "assets/textures/terrain/Soil.jpg"
            );

        if (m_grassTexture == 0 ||
            m_rockTexture == 0 ||
            m_dirtTexture == 0)
        {
            return false;
        }
		// Load the rock normal map texture
        m_rockNormalTexture =
            loadSaveTerrainTexture(
                "assets/textures/terrain/concrete-norm.jpg"
            );

        if (m_rockNormalTexture == 0)
        {
            BOX_LOG_ERROR("Failed to load rock normal map");     
            return false;
        }

		// Load the dirt normal map texture
        m_dirtNormalTexture =
            loadSaveTerrainTexture(
                "assets/textures/terrain/dirt-norm.jpg"
            );

        if (m_dirtNormalTexture == 0)
        {
            BOX_LOG_ERROR("Failed to load dirt normal map");     
            return false;
        }
		// Load the grass normal map texture
        m_grassNormalTexture =
            loadSaveTerrainTexture(
                "assets/textures/terrain/grass-norm.jpg"
            );

        if (m_grassNormalTexture == 0)
        {
            BOX_LOG_ERROR("Failed to load grass normal map");     
            return false;
        }
		
    return true;



}
// Generate a terrain mesh with the given width, depth, and scale taken from the height map
bool Terrain::GenerateTerrainMesh()
{

    // Clear previous terrain mesh data.
    m_vertices.clear();
    m_indices.clear();
    m_heightMap.clear();

    unsigned char* t_data = stbi_load("assets/textures/terrain/Data/test_all_black_T.png",
        &heightmapWidth, &heightmapHeight, &heightmapChannels, 0); // test_all_black_T
    if (!t_data) {
        std::cout << "Failed Loading the Heightmap " << std::endl;
        return false;

    }
    std::cout << "Loading the Heightmap of size " << heightmapHeight << " X " << heightmapWidth << std::endl;

   
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
    glBindTexture(
        GL_TEXTURE_2D,
        m_grassTexture
    );

    shader.SetUniformInt(
        "texture_grass",
        0
    );

	// Rock texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_rockTexture);

    shader.SetUniformInt("texture_rock", 1);

    // Normal map for rock texture
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m_rockNormalTexture);
    shader.SetUniformInt("normal_rock", 4);

	// Normal map for dirt texture
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, m_dirtNormalTexture);
	shader.SetUniformInt("normal_dirt", 5);

	// Normal map for grass texture
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, m_grassNormalTexture);
	shader.SetUniformInt("normal_grass", 6);


    glActiveTexture(GL_TEXTURE2);
    glBindTexture(
        GL_TEXTURE_2D,
        m_dirtTexture
    );

    shader.SetUniformInt(
        "texture_dirt",
        2
    );

    // ###########################################
    shader.SetUniformFloat(
        "rockSlopeStart",
        m_rockSlopeStart
    );

    shader.SetUniformFloat(
        "rockSlopeEnd",
        m_rockSlopeEnd
    );

    shader.SetUniformFloat(
        "dirtHeightStart",
        m_dirtHeightStart
    );

    shader.SetUniformFloat(
        "dirtHeightEnd",
        m_dirtHeightEnd
    );

    shader.SetUniformFloat(
        "dirtAmount",
        m_dirtAmount
    );

    shader.SetUniformFloat(
        "textureTiling",
        m_textureTiling
    );

    shader.SetUniformFloat(
        "rockNormalStrength",
        m_rockNormalStrength
    );

    shader.SetUniformFloat(
        "dirtNormalStrength",
        m_dirtNormalStrength
    );

    shader.SetUniformFloat(
        "grassNormalStrength",
        m_grassNormalStrength
    );



    // ###########################################
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
float Terrain::GetHeightAt(int x, int z) const
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

unsigned int Terrain::loadSaveTerrainTexture(
    const std::string& terrainPath)
{
    int width = 0;
    int height = 0;
    int nrComponents = 0;

    unsigned char* data =
        stbi_load(
            terrainPath.c_str(),
            &width,
            &height,
            &nrComponents,
            0
        );

    if (!data)
    {
        std::cout
            << "Texture failed to load: "
            << terrainPath
            << std::endl;

        return 0;
    }

    GLenum format = GL_RGB;

    if (nrComponents == 1)
    {
        format = GL_RED;
    }
    else if (nrComponents == 3)
    {
        format = GL_RGB;
    }
    else if (nrComponents == 4)
    {
        format = GL_RGBA;
    }
    else
    {
        std::cout
            << "Unsupported texture format: "
            << terrainPath
            << " channels: "
            << nrComponents
            << std::endl;

        stbi_image_free(data);
        return 0;
    }

    GLuint textureID = 0;

    glGenTextures(
        1,
        &textureID
    );

    glBindTexture(
        GL_TEXTURE_2D,
        textureID
    );

    glPixelStorei(
        GL_UNPACK_ALIGNMENT,
        1
    );

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data
    );

    glGenerateMipmap(
        GL_TEXTURE_2D
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_REPEAT
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_REPEAT
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR_MIPMAP_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
    );

    glBindTexture(
        GL_TEXTURE_2D,
        0
    );

    stbi_image_free(data);

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

void Terrain::SmoothTerrain(const glm::vec3& worldPosition, float radius, float strength)
{
    if (m_heightMap.empty())
    {
        return;
    }

    const float localX =
        worldPosition.x - m_position.x;

    const float localZ =
        worldPosition.z - m_position.z;

    const float mapX =
        localX +
        static_cast<float>(heightmapHeight) * 0.5f;

    const float mapZ =
        localZ +
        static_cast<float>(heightmapWidth) * 0.5f;

    const int centerX =
        static_cast<int>(std::round(mapX));

    const int centerZ =
        static_cast<int>(std::round(mapZ));

    const int brushRadius =
        static_cast<int>(std::ceil(radius));

    std::vector<float> original =
        m_heightMap;

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

            if (sampleX <= 0 ||
                sampleX >= heightmapHeight - 1 ||
                sampleZ <= 0 ||
                sampleZ >= heightmapWidth - 1)
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

            const float falloff =
                t * t *
                (3.0f - 2.0f * t);

            float average = 0.0f;

            for (int nz = -1;
                nz <= 1;
                ++nz)
            {
                for (int nx = -1;
                    nx <= 1;
                    ++nx)
                {
                    average +=
                        original[
                            (sampleX + nx) *
                                heightmapWidth +
                                (sampleZ + nz)
                        ];
                }
            }

            average /= 9.0f;

            const std::size_t index =
                sampleX *
                heightmapWidth +
                sampleZ;

            m_heightMap[index] =
                glm::mix(
                    original[index],
                    average,
                    strength * falloff
                );
        }
    }
}

void Terrain::FlattenTerrain(const glm::vec3& worldPosition, float radius, float strength, float targetHeight)
{
    if (m_heightMap.empty())
    {
        return;
    }

    const float localX =
        worldPosition.x - m_position.x;

    const float localZ =
        worldPosition.z - m_position.z;

    const float mapX =
        localX +
        static_cast<float>(heightmapHeight) * 0.5f;

    const float mapZ =
        localZ +
        static_cast<float>(heightmapWidth) * 0.5f;

    const int centerX =
        static_cast<int>(std::round(mapX));

    const int centerZ =
        static_cast<int>(std::round(mapZ));

    const int brushRadius =
        static_cast<int>(std::ceil(radius));

    // targetHeight is world-space.
    // Convert back to terrain-local height.
    const float localTargetHeight =
        targetHeight - m_position.y;

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

            const float falloff =
                t * t *
                (3.0f - 2.0f * t);

            const std::size_t index =
                static_cast<std::size_t>(
                    sampleX *
                    heightmapWidth +
                    sampleZ
                    );

            const float blend =
                glm::clamp(
                    strength * falloff,
                    0.0f,
                    1.0f
                );

            m_heightMap[index] =
                glm::mix(
                    m_heightMap[index],
                    localTargetHeight,
                    blend
                );
        }
    }
}

void Terrain::NoiseTerrain(const glm::vec3& worldPosition, float radius, float strength)
{
    if (m_heightMap.empty())
    {
        return;
    }

    const float localX =
        worldPosition.x - m_position.x;

    const float localZ =
        worldPosition.z - m_position.z;

    const float mapX =
        localX +
        static_cast<float>(heightmapHeight) * 0.5f;

    const float mapZ =
        localZ +
        static_cast<float>(heightmapWidth) * 0.5f;

    const int centerX =
        static_cast<int>(std::round(mapX));

    const int centerZ =
        static_cast<int>(std::round(mapZ));

    const int brushRadius =
        static_cast<int>(std::ceil(radius));

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

            const float falloff =
                t * t *
                (3.0f - 2.0f * t);

            const float randomValue =
                static_cast<float>(
                    std::rand()
                    ) /
                static_cast<float>(
                    RAND_MAX
                    );

            // Convert 0..1 into -1..1
            const float noise =
                randomValue * 2.0f - 1.0f;

            const std::size_t index =
                static_cast<std::size_t>(
                    sampleX *
                    heightmapWidth +
                    sampleZ
                    );

            m_heightMap[index] +=
                noise *
                strength *
                falloff;
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

            const std::size_t vertexIndex =
                heightIndex;

            const std::size_t base =
                vertexIndex * 8;


            // -------------------------------------------------
            // Update vertex height
            // -------------------------------------------------

            m_vertices[base + 1] =
                m_heightMap[heightIndex];


            // -------------------------------------------------
            // Find neighbouring height samples
            // -------------------------------------------------

            const int left =
                std::max(j - 1, 0);

            const int right =
                std::min(
                    j + 1,
                    heightmapWidth - 1
                );

            const int down =
                std::max(i - 1, 0);

            const int up =
                std::min(
                    i + 1,
                    heightmapHeight - 1
                );


            const float hL =
                m_heightMap[
                    i * heightmapWidth +
                        left
                ];

            const float hR =
                m_heightMap[
                    i * heightmapWidth +
                        right
                ];

            const float hD =
                m_heightMap[
                    down * heightmapWidth +
                        j
                ];

            const float hU =
                m_heightMap[
                    up * heightmapWidth +
                        j
                ];


            // -------------------------------------------------
            // Recalculate normal
            // -------------------------------------------------

            const glm::vec3 normal =
                glm::normalize(
                    glm::vec3(
                        hL - hR,
                        2.0f,
                        hD - hU
                    )
                );


            // -------------------------------------------------
            // Store normal back into vertex
            // -------------------------------------------------

            m_vertices[base + 3] =
                normal.x;

            m_vertices[base + 4] =
                normal.y;

            m_vertices[base + 5] =
                normal.z;
        }
    }


    // -------------------------------------------------
    // Upload changed vertices to GPU
    // -------------------------------------------------

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
    if (m_grassTexture != 0)
    {
        glDeleteTextures(
            1,
            &m_grassTexture
        );

        m_grassTexture = 0;
    }

    if (m_rockTexture != 0)
    {
        glDeleteTextures(
            1,
            &m_rockTexture
        );

        m_rockTexture = 0;
    }

    if (m_dirtTexture != 0)
    {
        glDeleteTextures(
            1,
            &m_dirtTexture
        );

        m_dirtTexture = 0;
    }

    if (m_terrainTexture != 0)
    {
        glDeleteTextures(
            1,
            &m_terrainTexture
        );

        m_terrainTexture = 0;
    }

    if (m_VBO != 0)
    {
        glDeleteBuffers(
            1,
            &m_VBO
        );

        m_VBO = 0;
    }

    if (m_EBO != 0)
    {
        glDeleteBuffers(
            1,
            &m_EBO
        );

        m_EBO = 0;
    }

    if (m_VAO != 0)
    {
        glDeleteVertexArrays(
            1,
            &m_VAO
        );

        m_VAO = 0;
    }

    if (m_rockNormalTexture != 0)
    {
        glDeleteTextures(
            1,
            &m_rockNormalTexture
        );

        m_rockNormalTexture = 0;
    }

}

// =========================================================
// Save the terrain heightmap to a binary file
// =========================================================

bool Terrain::SaveHeightMap(const std::string& filePath, const TerrainWaterSettings& water) const
{
    // ============================================
    // VALIDATE TERRAIN
    // ============================================

    if (heightmapWidth <= 0 ||
        heightmapHeight <= 0)
    {
        BOX_LOG_ERROR("Terrain Save: invalid dimensions");
        return false;
    }

    const std::size_t expectedCount =
        static_cast<std::size_t>(heightmapWidth) *
        static_cast<std::size_t>(heightmapHeight);

    if (m_heightMap.size() != expectedCount)
    {
        BOX_LOG_ERROR("Terrain Save: invalid heightmap size");
        return false;
    }

    // ============================================
    // CREATE DESTINATION FOLDER
    // ============================================

    const fs::path path(filePath);

    if (path.has_parent_path())
    {
        std::error_code error;

        fs::create_directories(
            path.parent_path(),
            error
        );

        if (error)
        {
            BOX_LOG_ERROR(
                "Terrain Save: cannot create folder "
                << error.message()
            );

            return false;
        }
    }

    // ============================================
    // OPEN BINARY FILE
    // ============================================

    

    std::ofstream file(std::filesystem::path(filePath), std::ios::binary | std::ios::trunc);

    /*std::ofstream file(
        filePath,
        std::ios::binary | std::ios::trunc);*/

    if (!file.is_open())
    {
        BOX_LOG_ERROR(
            "Terrain Save: cannot open " << filePath
        );
        return false;
    }

    // ============================================
    // WRITE FILE HEADER
    // ============================================

    // 8-byte identifier
    const char magic[8] =
    {
        'B', 'T', 'E', 'R',
        'R', 'A', 'I', 'N'
    };

    const std::uint32_t version = 2;

    const std::uint32_t width =
        static_cast<std::uint32_t>(heightmapWidth);

    const std::uint32_t height =
        static_cast<std::uint32_t>(heightmapHeight);

    file.write(magic, sizeof(magic));

    file.write(
        reinterpret_cast<const char*>(&version),
        sizeof(version)
    );

    file.write(
        reinterpret_cast<const char*>(&width),
        sizeof(width)
    );

    file.write(
        reinterpret_cast<const char*>(&height),
        sizeof(height)
    );

    // ============================================
    // WRITE TERRAIN HEIGHT VALUES
    // ============================================

    static_assert(
        sizeof(float) == 4,
        "Terrain format requires 32-bit floats"
        );

    file.write(
        reinterpret_cast<const char*>(m_heightMap.data()),
        static_cast<std::streamsize>(
            m_heightMap.size() * sizeof(float)));


    // ============================================
    // VERSION 2 - SAVE WATER SETTINGS
    // ============================================

    // Store bool as a fixed-size integer.
    const std::uint32_t waterEnabled =
        water.enabled ? 1u : 0u;

    file.write(
        reinterpret_cast<const char*>(&waterEnabled),
        sizeof(waterEnabled)
    );

    file.write(
        reinterpret_cast<const char*>(&water.height),
        sizeof(float)
    );

    file.write(
        reinterpret_cast<const char*>(&water.waveSpeed),
        sizeof(float)
    );

    file.write(
        reinterpret_cast<const char*>(&water.waveStrength),
        sizeof(float)
    );

    file.write(
        reinterpret_cast<const char*>(&water.specularPower),
        sizeof(float));

	// end water settings





    file.flush();

    if (!file)
    {
        BOX_LOG_ERROR(
            "Terrain Save: failed to write " << filePath
        );

        return false;
    }

    BOX_LOG_INFO(
        "Terrain saved: " << filePath
        << " Width=" << width
        << " Height=" << height
        << " Samples=" << m_heightMap.size()
    );

    return true;
}


bool Terrain::LoadHeightMap(const std::string& filePath, TerrainWaterSettings& water)
{
    water = TerrainWaterSettings{};

    std::ifstream file(std::filesystem::path(filePath), std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        BOX_LOG_ERROR(
            "Terrain Load: cannot open " << filePath
        );
        return false;
    }

    const std::streamoff fileSize = file.tellg();

    file.seekg(0, std::ios::beg);

    // ============================================
    // READ AND VALIDATE HEADER
    // ============================================

    char magic[8]{};

    
    std::uint32_t version = 0;
    std::uint32_t width = 0;
    std::uint32_t height = 0;

    file.read(magic, 8);

    file.read(
        reinterpret_cast<char*>(&version),
        sizeof(version));

    std::cout << "Terrain file version: " << version << std::endl;

    file.read(
        reinterpret_cast<char*>(&width),
        sizeof(width));

    file.read(
        reinterpret_cast<char*>(&height),
        sizeof(height));

    const char expectedMagic[8] =
    {
        'B', 'T', 'E', 'R',
        'R', 'A', 'I', 'N'
    };

    if (!file ||
        std::memcmp(magic, expectedMagic, 8) != 0 ||
        (version != 1 && version != 2) ||
        width < 2 ||
        height < 2 ||
        width > 8192 ||
        height > 8192)
    {
        BOX_LOG_ERROR(
            "Terrain Load: invalid file header"
        );
        return false;
    }

    // ============================================
    // VERIFY FILE SIZE
    // ============================================

    const std::uint64_t sampleCount =
        static_cast<std::uint64_t>(width) *
        static_cast<std::uint64_t>(height);

    const std::uint64_t expectedSize =
        20ULL +
        sampleCount * sizeof(float) +
        (version == 2 ? 20ULL : 0ULL);

    if (fileSize < 0 ||
        static_cast<std::uint64_t>(fileSize) != expectedSize)
    {
        BOX_LOG_ERROR(
            "Terrain Load: unexpected file size"
        );
        return false;
    }

    // ============================================
    // READ INTO TEMPORARY BUFFER
    // ============================================

    std::vector<float> loadedHeights(
        static_cast<std::size_t>(sampleCount)
    );

    file.read(
        reinterpret_cast<char*>(loadedHeights.data()),
        static_cast<std::streamsize>(
            sampleCount * sizeof(float)
            )
    );

    if (!file)
    {
        BOX_LOG_ERROR(
            "Terrain Load: failed to read heights"
        );
        return false;
    }

    for (float value : loadedHeights)
    {
        if (!std::isfinite(value))
        {
            BOX_LOG_ERROR(
                "Terrain Load: invalid height value"
            );
            return false;
        }
    }


    // ============================================
    // VERSION 2 - LOAD WATER SETTINGS
    // ============================================

    TerrainWaterSettings loadedWater;

    if (version == 2)
    {
        std::uint32_t waterEnabled = 0;

        file.read(
            reinterpret_cast<char*>(&waterEnabled),
            sizeof(waterEnabled)
        );

        file.read(
            reinterpret_cast<char*>(&loadedWater.height),
            sizeof(float)
        );

        file.read(
            reinterpret_cast<char*>(&loadedWater.waveSpeed),
            sizeof(float)
        );

        file.read(
            reinterpret_cast<char*>(&loadedWater.waveStrength),
            sizeof(float)
        );

        file.read(
            reinterpret_cast<char*>(&loadedWater.specularPower),
            sizeof(float)
        );

        if (!file ||
            waterEnabled > 1 ||
            !std::isfinite(loadedWater.height) ||
            !std::isfinite(loadedWater.waveSpeed) ||
            !std::isfinite(loadedWater.waveStrength) ||
            !std::isfinite(loadedWater.specularPower))
        {
            BOX_LOG_ERROR(
                "Terrain Load: invalid water settings"
            );

            return false;
        }

        loadedWater.enabled = (waterEnabled == 1);
        loadedWater.hasWaterSettings = true;
    }


    // ============================================
    // UPDATE TERRAIN
    // ============================================

    std::cout << "\n========== TERRAIN LOAD ==========\n";

    std::cout << "Saved file width: "
        << width << "\n";

    std::cout << "Saved file height: "
        << height << "\n";
    // Exception thrown: read access violation. this was nullptr.

    std::cout << "Current terrain width: "
        << heightmapWidth << "\n";

    std::cout << "Current terrain height: "
        << heightmapHeight << "\n";

    std::cout << "Heightmap samples: "
        << m_heightMap.size() << "\n";

    std::cout << "Terrain vertices: "
        << m_vertices.size() / 8 << "\n";

    std::cout << "==================================\n";




    // The current terrain renderer uses a fixed
    // grid topology. For our first test, only
    // load files matching the active grid size.
    if (static_cast<int>(width) != heightmapWidth ||
        static_cast<int>(height) != heightmapHeight)
    {
        BOX_LOG_ERROR(
            "Terrain Load: dimensions do not match "
            "the current terrain"
        );
        return false;
    }

    // All validation has succeeded.
    m_heightMap = std::move(loadedHeights);

    // Update vertex positions, normals and GPU data.
    
    UpdateMeshFromHeightMap();

    // Return the loaded water settings to the caller.
    water = loadedWater;

    BOX_LOG_INFO(
        "Terrain loaded: " << filePath
        << " Width=" << width
        << " Height=" << height
    );

    return true;
}


bool Terrain::GetCellAtWorldPosition(
    const glm::vec3& worldPosition,
    int& outCellX,
    int& outCellZ) const
{
    if (heightmapWidth < 2 ||
        heightmapHeight < 2 ||
        m_heightMap.empty())
    {
        return false;
    }

    // Convert world coordinates into local terrain space.
    const glm::vec3 localPosition =
        worldPosition - m_position;

    // Terrain is centred around the origin.
    const float gridX =
        localPosition.x +
        static_cast<float>(heightmapHeight) * 0.5f;

    const float gridZ =
        localPosition.z +
        static_cast<float>(heightmapWidth) * 0.5f;

    // Make sure we're inside the terrain.
    if (gridX < 0.0f ||
        gridZ < 0.0f ||
        gridX >= static_cast<float>(heightmapHeight - 1) ||
        gridZ >= static_cast<float>(heightmapWidth - 1))
    {
        return false;
    }

    outCellX = static_cast<int>(std::floor(gridX));
    outCellZ = static_cast<int>(std::floor(gridZ));

    return true;
}

