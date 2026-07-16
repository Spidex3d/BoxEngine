#include <iostream>

#include "BoxEngine.h"



// TODO: This is an example of a library function
void BoxEngine::testFunction()
{
	std::cout << "This is a test function from BoxEngine lib!" << std::endl;
}

void BoxEngine::ResizeSceneViewport(int width, int height)
{
    m_sceneFramebuffer.Resize(width, height);
}

void BoxEngine::RenderScene()
{
    if (!m_sceneFramebuffer.IsValid())
        return;

    m_sceneFramebuffer.Bind();

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.12f, 0.15f, 0.18f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Renderer draws camera, grid, entities, lights, etc.

    Framebuffer::Unbind();
}

GLuint BoxEngine::GetSceneTexture() const
{
    return m_sceneFramebuffer.GetColorTexture();
}