#pragma once
#include <rendering/Framebuffer.h>

#include <glm/glm.hpp>

#include <memory>
#include <vector>

class Shader;
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

    bool AddEditableCube(
        const glm::vec3& position =
        glm::vec3(0.0f)
    );

    GLuint GetSceneTexture() const;

private:
    Framebuffer m_sceneFramebuffer;

    std::unique_ptr<Shader> m_sceneShader;
    std::vector<std::unique_ptr<Entity>> m_entities;

    int m_nextEntityID = 0;

    glm::vec3 m_cameraPosition{
        2.5f,
        2.0f,
        3.5f
    };
};



//#include <rendering\Framebuffer.h>
//
//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
//
//
//
//class BoxEngine
//{
//public:
//	BoxEngine() = default;
//	~BoxEngine() = default;
//	
//	void testFunction();
//	
//	bool Initialize();
//
//	void ResizeSceneViewport(int width, int height);
//	void RenderScene();
//
//	GLuint GetSceneTexture() const;
//private:
//	Framebuffer m_sceneFramebuffer;
//	
//
//
//};