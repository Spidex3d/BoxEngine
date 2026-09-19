#pragma once
#include <filesystem>
#include <memory>
#include <vector>
#include <ostream>

class Entity;
// version 0.1 19/09/2026
constexpr int MBSENE_VERSION_MAJOR = 0;
constexpr int MBSENE_VERSION_MINOR = 1;

struct MBSeneData
{
	int versionMajor = 0;
	int versionMinor = 0;
	// list all entities in the scene with their properties
	
};


class SceneSerializer
{
public:
	SceneSerializer() = default;
	~SceneSerializer();

	bool SerializeScene(const std::filesystem::path& filePath, const std::vector<std::unique_ptr<Entity>>& entities); // save scene to file



private:
	bool SerializeEntity(std::ostream& output, const Entity& entity);

};

/*
# BoxEditor Scene
version 0 1

entities 3

entity
id 1
name Cube_1
primitive Cube

position 0 0 0
rotation 0 0 0
scale 1 1 1

...editable vertices...
...editable faces...
...material slots...

endentity

entity
...
endentity
*/