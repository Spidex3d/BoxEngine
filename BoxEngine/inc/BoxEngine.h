#pragma once
#include <rendering/Framebuffer.h>
#include <entity\Entity.h>
#include <glm/glm.hpp>

#include <memory>
#include <vector>

class Shader;
class Grid;
class Entity;
class Camera;

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


	const std::vector<std::unique_ptr<Entity>>& GetEntities() const; // used to access the entities in the scene from the editor panels
    
    void SetSelectedEntity(int entityID);

    Entity* GetSelectedEntity();

    const Entity* GetSelectedEntity() const;

    int GetSelectedEntityID() const;

    void ClearSelectedEntity();

    bool RemoveEntity(int entityID);
    
    void PickEntity(const glm::vec3& rayOrigin, const glm::vec3& rayDirection);

private:
    void RenderSelectedEntityOutline(const glm::mat4& view, const glm::mat4& projection); // Render the outline of the selected entity

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


