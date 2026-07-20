#include "BoxEngine.h"
#include <shader/Shader.h>
#include <entity/Entity.h>
#include <rendering/Grid.h>
#include "camera/Camera.h"

#include <Helpers.h>
#include <miniBoxLog.h>

#include <glm/gtc/matrix_transform.hpp>

#include <string>

BoxEngine::BoxEngine() = default;
BoxEngine::~BoxEngine() = default;

bool BoxEngine::Initialize()
{
    Helpers helpers;

    const std::string cubeVertexShaderPath =
        helpers.GetAssetPath(
            "assets/shader/basicCube.vert"
        );

    const std::string cubeFragmentShaderPath =
        helpers.GetAssetPath(
            "assets/shader/basicCube.frag"
        );

    m_sceneShader = std::make_unique<Shader>(
        cubeVertexShaderPath,
        cubeFragmentShaderPath
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

    const std::string gridVertexShaderPath =
        helpers.GetAssetPath(
            "assets/shader/grid.vert"
        );

    const std::string gridFragmentShaderPath =
        helpers.GetAssetPath(
            "assets/shader/grid.frag"
        );

    m_gridShader = std::make_unique<Shader>(
        gridVertexShaderPath,
        gridFragmentShaderPath
    );

    if (!m_gridShader ||
        m_gridShader->ID() == 0)
    {
        BOX_LOG_ERROR(
            "BoxEngine failed to create grid shader"
        );

        m_gridShader.reset();
        m_sceneShader.reset();
        return false;
    }

    m_camera = std::make_unique<Camera>(glm::vec3(6.0f, 5.0f, 8.0f));

    m_camera->SetPositionYawPitch(glm::vec3(6.0f, 5.0f, 8.0f), -135.0f, -25.0f);

    m_camera->Target = glm::vec3(0.0f);

    m_camera->OrbitDistance = glm::length(m_camera->Position - m_camera->Target);

    
    if (!AddGrid(glm::vec3(0.0f), 20, 1.0f))
    {
        BOX_LOG_ERROR(
            "BoxEngine failed to create grid"
        );

        return false;
    }

    BOX_LOG_INFO("BoxEngine initialized");
    return true;

}

void BoxEngine::Shutdown()
{
    if (m_grid)
    {
        m_grid->Destroy();
        m_grid.reset();
    }

    // Entity destructors delete their OpenGL buffers.
    m_entities.clear();

    m_camera.reset();

    m_gridShader.reset();
    m_sceneShader.reset();

    m_sceneFramebuffer.Destroy();

    BOX_LOG_INFO("BoxEngine shutdown complete");
}

Camera& BoxEngine::GetCamera()
{
    return *m_camera;
}

const Camera& BoxEngine::GetCamera() const
{
    return *m_camera;
}

bool BoxEngine::AddGrid(const glm::vec3& position, int halfSize, float spacing)
{
    m_grid = std::make_unique<Grid>();

    m_grid->SetPosition(position);

    if (!m_grid->Create(halfSize, spacing))
    {
        BOX_LOG_ERROR(
            "Failed to add editor grid"
        );

        m_grid.reset();
        return false;
    }

    BOX_LOG_INFO(
        "Added editor grid"
    );

    return true;
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
// Return a const reference to the vector of unique_ptr<Entity> for the editor panels to access the entities in the scene
const std::vector<std::unique_ptr<Entity>>&
BoxEngine::GetEntities() const
{
    return m_entities;
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

        if (!m_camera)
        {
            Framebuffer::Unbind();
            return;
        }

        const glm::mat4 view =
            m_camera->GetViewMatrix();

        const glm::mat4 projection =
            m_camera->GetProjectionMatrix(aspect);

        /*const glm::mat4 projection =
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
            );*/

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

        // ################################## Grid Rendering ########################################
        if (m_grid && m_gridShader)
        {
            m_gridShader->Use();

            m_gridShader->setVec3(
                "uGridColor",
                glm::vec3(0.35f)
            );

            m_grid->Render(*m_gridShader, view, projection);
        }
                
        // ##########################################################################################

		// Render entities
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



