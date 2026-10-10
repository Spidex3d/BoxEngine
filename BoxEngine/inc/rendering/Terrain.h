#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

class Shader;

struct TerrainImage
{
	GLuint textureID;
	std::string path;
};


class Terrain
{
public:
		
	bool Initialize();

	// build a terrain vertex array object
	bool GenerateTerrainMesh(); 

	// Render the terrain mesh
	void RenderTerrain(Shader& shader, const glm::mat4& view, const glm::mat4& projection);

	// Load terrain textures from a folder and return a vector of TerrainImage
	std::vector<TerrainImage> LoadTerrainTextureFromFolder(const std::string& folderPath);

	// update the terrain texture with a new texture, and save the new texture to the terrain folder
	unsigned int loadSaveTerrainTexture(const std::string& terrainPath);

	// update the terrain mesh with new width, depth, and scale values
	void ReloadTerrainMeshFromFile(const std::string& heightmapPath);


	void Destroy();

	// Getters for terrain mesh data
	const std::vector<float>& GetVertices() const { return m_vertices; }
	const std::vector<unsigned int>& GetIndices() const { return m_indices; }

private:
	GLuint m_VAO, m_VBO, m_EBO;

	std::vector<float> m_vertices;
	std::vector<unsigned int> m_indices;
	unsigned int m_terrainTexture;

	int heightmapWidth = 0;
	int heightmapHeight = 0;
	int heightmapChannels = 0;
	
	// texturemap
	int textureMapWidth = 0;
	int textureMapHeight = 0;
	int textureMapChannels = 0;

	int rez = 1;
	int numStrips = 0;
	int vertsPerStrip = 0;




};
