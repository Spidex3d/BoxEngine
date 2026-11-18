#pragma once
#include <filesystem>
#include <string>
#include <memory>
#include <vector>
#include <ostream>

class Entity;
// version 0.1 19/09/2026
// version 0.2 18/11/2026
// version 0.3 18/11/2026
constexpr int MBSENE_VERSION_MAJOR = 0;
constexpr int MBSENE_VERSION_MINOR = 3;

struct MBSeneData
{
	int versionMajor = 0;
	int versionMinor = 0;
	// list all entities in the scene with their properties
	
};

// ============================================
// SKYBOX SCENE DATA
// ============================================

struct SceneSkyData
{
	bool enabled = false;
	std::string texturePath;
};

class SceneSerializer
{
public:
	SceneSerializer() = default;
	~SceneSerializer();

	bool SerializeScene(const std::filesystem::path& filePath, const std::vector<std::unique_ptr<Entity>>& entities); // save scene to file
	bool SerializeScene(const std::filesystem::path& filePath, const std::vector<std::unique_ptr<Entity>>& entities,
		const SceneSkyData& skyData);
	
	//bool DeserializeScene(const std::filesystem::path& filePath);
	bool DeserializeScene(const std::filesystem::path& filePath, std::vector<std::unique_ptr<Entity>>& loadedEntities, SceneSkyData& skyData);
private:
	bool SerializeEntity(std::ostream& output, const Entity& entity);

};

