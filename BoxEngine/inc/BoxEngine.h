#pragma once
#include <rendering/Framebuffer.h>
#include <entity\Entity.h>
#include <glm/glm.hpp>
#include <rendering/Textures.h>

#include <memory>
#include <vector>

class Shader;
class Grid;
class Entity;
class Camera;
class Texture;

enum class TransformTool
{
    Select,
    Move,
    Rotate,
    Scale
};

class BoxEngine
{
public:
    BoxEngine();
    ~BoxEngine();

    BoxEngine(const BoxEngine&) = delete;
    BoxEngine& operator=(const BoxEngine&) = delete;

    bool Initialize();
    void Shutdown();

    void ResizeSceneViewport(
        int width,
        int height
    );

    void RenderScene();

    Camera& GetCamera();
    const Camera& GetCamera() const;

    GLuint GetSceneTexture() const;

    bool AddGrid(const glm::vec3& position, int halfSize, float spacing);    

    bool AddEditableCube(const glm::vec3& position = glm::vec3(0.0f));

    bool AddEditablePlane(const glm::vec3& position = glm::vec3(0.0f));

    bool AddEditableSphere(const glm::vec3& position);

	bool AddEditableCylinder(const glm::vec3& position = glm::vec3(0.0f), int sectors = 8, int stacks = 1, float radius = 0.5f, float height = 1.0f);

	bool AddEditablePyramid(const glm::vec3& position = glm::vec3(0.0f));

	const std::vector<std::unique_ptr<Entity>>& GetEntities() const; // used to access the entities in the scene from the editor panels
    
    void SetSelectedEntity(int entityID);

    Entity* GetSelectedEntity();

    const Entity* GetSelectedEntity() const;

    int GetSelectedEntityID() const;

    void ClearSelectedEntity();

    bool RemoveEntity(int entityID);
    
    void PickEntity(const glm::vec3& rayOrigin, const glm::vec3& rayDirection);

    TransformTool m_activeTool = TransformTool::Move;

    GLuint LoadTexture(const std::string& path);

	Entity* AddImportedMesh(const std::string& name, const MeshData& meshData); // Add an imported mesh to the scene as an entity

private:
    void RenderSelectedEntityOutline(const glm::mat4& view, const glm::mat4& projection); // RenderPreview the outline of the selected entity

	Texture m_defaultTexture; // The Default texture for loading and rendering, checkerboard texture. 

	std::vector<std::unique_ptr<Texture>> m_textures; // the main texture storage for the engine.

private:
    Framebuffer m_sceneFramebuffer;

	std::unique_ptr<Shader> m_sceneShader;      // for rendering the scene entities
	std::unique_ptr<Shader> m_gridShader;       // for rendering the editor grid
	std::unique_ptr<Shader> m_outlineShader;    // for rendering the outline of the selected entity


	std::unique_ptr<Grid> m_grid; // for the editor grid not an entity
	std::vector<std::unique_ptr<Entity>> m_entities; // for the editable cubes and other entities
    
    std::unique_ptr<Camera> m_camera;

    int m_nextEntityID = 0;

	int m_selectedEntityID = -1; // used to track the selected entity in the editor panels

    const std::string checkerboardPath = m_defaultTexture.GetSourcePath().string();
	std::string m_defaultTexturePath; // The default texture path for the checkerboard texture for save and load purposes

	// piking helper function to check if a ray intersects an AABB in world space
    bool RayIntersectsAABB(
        const glm::vec3& rayOriginWorld,
        const glm::vec3& rayDirectionWorld,
        const glm::mat4& modelMatrix,
        const glm::vec3& aabbMinLocal,
        const glm::vec3& aabbMaxLocal,
        float& outDistanceWorld
    ) const;

};


