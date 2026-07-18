#pragma once
#include <rendering/Framebuffer.h>

#include <glm/glm.hpp>

#include <memory>
#include <vector>

class Shader;
class Grid;
class Entity;

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

    bool AddGrid(const glm::vec3& position, int halfSize, float spacing);    

    bool AddEditableCube(const glm::vec3& position = glm::vec3(0.0f));

    GLuint GetSceneTexture() const;

private:
    Framebuffer m_sceneFramebuffer;

    std::unique_ptr<Shader> m_sceneShader;
    std::unique_ptr<Shader> m_gridShader;
	std::unique_ptr<Grid> m_grid; // for the editor grid not an entity
	std::vector<std::unique_ptr<Entity>> m_entities; // for the editable cubes and other entities

    int m_nextEntityID = 0;

    glm::vec3 m_cameraPosition{
        2.5f,
        2.0f,
        3.5f
    };
};


