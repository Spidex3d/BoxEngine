#pragma once
#include <glm\glm.hpp>
#include <vector>
#include <memory>
#include <string>

struct EditorObj { // Any Editor object name change from gameObj

    virtual ~EditorObj() = default; // make polymorphic for safe dynamic_cast

    EditorObj()
        : entId(-1),
        entTypeID(-1),
        entObjectIndex(-1),
        entName(),
        position(0.0f),
        scale(1.0f),
        rotation(0.0f),
        modelMatrix(1.0f),
        isRotateY(false),
        isVisible(true),
        tex_ID(0),
        texPath(""),
        //entType(""),
        assetPath("")
    {
    }

    int entId;          // individual entity ID
    int entTypeID;      // type of entity ie; plane, cube, npc, pickup etc
    int entObjectIndex; // index to the model object in the model manager Ie; how many objects of this type exist
    std::string entName;

    glm::vec3 position;     // Position of the object
    glm::vec3 scale;        // Scale of the object
    glm::vec3 rotation;     // Rotation of the object
    glm::mat4 modelMatrix;  // Model matrix for transformations

    bool isRotateY;          // whether the entity should rotate around the Y-axis (for simple animation)
    bool isVisible;         // Render or not
    // Textures for game objects like cubes and planes,
    unsigned int tex_ID;
    std::string texPath; // path to the texture file, used for loading and debugging, not used at runtime after texture is loaded
    // used for complex models that have their own materials/textures (like gltf/obj models) but can be applied to them as a single texture
    // if desired (e.g. for quick testing or if the model is untextured)

    //std::string entType;    // e.g. "Cube", "Plane", "Floor", "Obj", "Gltf", "Skybox"
    std::string assetPath;  // optional: path to model  "Obj", "Gltf", "Skybox" file used to create this entity

};




//This is whereare put together all the components of an entity, such as mesh, material, transform, etc.
class Entity
{
public:
	Entity() = default;
	~Entity() = default;
	// that can be modified in the editor, but we can expand it later as needed.
	void CreateEditableCube(std::vector<std::unique_ptr<EditorObj>>& entVector, int& currentIndex,
		int& EditableCubeObjIdx, const glm::vec3& position = glm::vec3(0.0f),
		const std::string& texturePath = "");



};
