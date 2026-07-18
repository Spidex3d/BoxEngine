#include "BoxEngine.h"
#include <shader/Shader.h>
#include <entity/Entity.h>

#include <Helpers.h>
#include <miniBoxLog.h>

#include <glm/gtc/matrix_transform.hpp>

#include <string>

BoxEngine::BoxEngine() = default;
BoxEngine::~BoxEngine() = default;

bool BoxEngine::Initialize()
{
    Helpers helpers;

    const std::string vertexShaderPath =
        helpers.GetAssetPath(
            "assets/shader/basicCube.vert"
        );

    const std::string fragmentShaderPath =
        helpers.GetAssetPath(
            "assets/shader/basicCube.frag"
        );

    m_sceneShader = std::make_unique<Shader>(
        vertexShaderPath,
        fragmentShaderPath
    );

    if (!m_sceneShader ||
        m_sceneShader->ID() == 0)
    {
        BOX_LOG_ERROR(
            "BoxEngine failed to create scene shader"
        );

        m_sceneShader.reset();
        return false;
    }

    BOX_LOG_INFO("BoxEngine initialized");
    return true;
}

void BoxEngine::Shutdown()
{
    // Destroy entities first because they own VAOs/VBOs.
    m_entities.clear();

    // Shader owns an OpenGL program.
    m_sceneShader.reset();

    // Framebuffer owns OpenGL textures/renderbuffers.
    m_sceneFramebuffer.Destroy();

    BOX_LOG_INFO("BoxEngine shutdown complete");
}

bool BoxEngine::AddEditableCube(
    const glm::vec3& position)
{
    const int entityID = m_nextEntityID++;

    const std::string entityName =
        "Cube " + std::to_string(entityID);

    auto cube = std::make_unique<Entity>(
        entityID,
        entityName
    );

    cube->SetPosition(position);

    if (!cube->CreateCube())
    {
        BOX_LOG_ERROR(
            "Failed to add editable cube"
        );

        return false;
    }

    m_entities.push_back(
        std::move(cube)
    );

    BOX_LOG_INFO(
        "Added editable cube. Entity count: "
        << m_entities.size()
    );

    return true;
}

void BoxEngine::ResizeSceneViewport(
    int width,
    int height)
{
    m_sceneFramebuffer.Resize(
        width,
        height
    );
}

void BoxEngine::RenderScene()
{
    if (!m_sceneFramebuffer.IsValid())
        return;

    m_sceneFramebuffer.Bind();

    glEnable(GL_DEPTH_TEST);

    glClearColor(
        0.12f,
        0.15f,
        0.18f,
        1.0f
    );

    glClear(
        GL_COLOR_BUFFER_BIT |
        GL_DEPTH_BUFFER_BIT
    );

    if (m_sceneShader &&
        m_sceneShader->ID() != 0)
    {
        const float width =
            static_cast<float>(
                m_sceneFramebuffer.GetWidth()
                );

        const float height =
            static_cast<float>(
                m_sceneFramebuffer.GetHeight()
                );

        const float aspect =
            height > 0.0f
            ? width / height
            : 1.0f;

        const glm::mat4 projection =
            glm::perspective(
                glm::radians(45.0f),
                aspect,
                0.1f,
                100.0f
            );

        const glm::mat4 view =
            glm::lookAt(
                m_cameraPosition,
                glm::vec3(0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f)
            );

        m_sceneShader->Use();

        m_sceneShader->setVec3(
            "uLightDirection",
            glm::normalize(
                glm::vec3(
                    -1.0f,
                    -1.0f,
                    -0.5f
                )
            )
        );

        m_sceneShader->setVec3(
            "uObjectColor",
            glm::vec3(
                0.2f,
                0.55f,
                0.9f
            )
        );

        for (const auto& entity : m_entities)
        {
            if (entity)
            {
                entity->Render(
                    *m_sceneShader,
                    view,
                    projection
                );
            }
        }
    }

    Framebuffer::Unbind();
}

GLuint BoxEngine::GetSceneTexture() const
{
    return m_sceneFramebuffer.GetColorTexture();
}






//#include <iostream>
//
//#include "BoxEngine.h"
//
//
//
// TODO: This is an example of a library function
//void BoxEngine::testFunction()
//{
//	std::cout << "This is a test function from BoxEngine lib!" << std::endl;
//}
//
//void BoxEngine::ResizeSceneViewport(int width, int height)
//{
//    m_sceneFramebuffer.Resize(width, height);
//}
//
//void BoxEngine::RenderScene()
//{
//    if (!m_sceneFramebuffer.IsValid())
//        return;
//
//    m_sceneFramebuffer.Bind();
//
//    glEnable(GL_DEPTH_TEST);
//    glClearColor(0.12f, 0.15f, 0.18f, 1.0f);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//     Renderer draws camera, grid, entities, lights, etc.
//
//    Framebuffer::Unbind();
//}
//
//GLuint BoxEngine::GetSceneTexture() const
//{
//    return m_sceneFramebuffer.GetColorTexture();
//}