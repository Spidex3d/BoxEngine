#include "BoxEngine.h"
#include <shader/Shader.h>
#include <entity/Entity.h>
#include <rendering/Grid.h>
#include "camera/Camera.h"

#include <Helpers.h>
#include <miniBoxLog.h>
#include "tools/MoveGizmo.h"

#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <algorithm>

#include <cmath>
#include <limits>

#include <glm/gtc/matrix_inverse.hpp>

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
	// ############################################### outline shader for selected entity ###############################################
    const std::string outlineVertexPath =
        helpers.GetAssetPath(
            "assets/shader/outline.vert"
        );

    const std::string outlineFragmentPath =
        helpers.GetAssetPath(
            "assets/shader/outline.frag"
        );

    m_outlineShader = std::make_unique<Shader>(
            outlineVertexPath, outlineFragmentPath
        );

    if (!m_outlineShader ||
        m_outlineShader->ID() == 0)
    {
        BOX_LOG_ERROR(
            "Failed to create outline shader."
        );

        return false;
    }

	// ################################################# end shader ########################################################
    
	// ################################################# Tools  ########################################################
   
    const std::string gizmoVertexPath =
        helpers.GetAssetPath(
            "assets/shader/gizmo.vert"
        );

    const std::string gizmoFragmentPath =
        helpers.GetAssetPath(
            "assets/shader/gizmo.frag"
        );

    m_gizmoShader =
        std::make_unique<Shader>(
            gizmoVertexPath,
            gizmoFragmentPath
        );

    if (!m_gizmoShader ||
        m_gizmoShader->ID() == 0)
    {
        BOX_LOG_ERROR(
            "Failed to create gizmo shader."
        );

        return false;
    }

    m_moveGizmo =
        std::make_unique<MoveGizmo>();

    if (!m_moveGizmo->Create())
    {
        BOX_LOG_ERROR(
            "Failed to create move gizmo."
        );

        return false;
    }

	// ################################################# End tools######################################################

    m_camera = std::make_unique<Camera>(glm::vec3(6.0f, 5.0f, 8.0f));

    m_camera->SetPositionYawPitch(glm::vec3(6.0f, 5.0f, 8.0f), -135.0f, -25.0f);

    m_camera->Target = glm::vec3(0.0f);

    m_camera->OrbitDistance = glm::length(m_camera->Position - m_camera->Target);

    
    if (!AddGrid(glm::vec3(0.0f, -0.5f, 0.0f), 20, 1.0f))
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

    m_selectedEntityID = -1;

    // Entity destructors delete their OpenGL buffers.
    m_entities.clear();

    m_camera.reset();

    m_gridShader.reset();
    m_sceneShader.reset();
    m_outlineShader.reset();

    if (m_moveGizmo)
    {
        m_moveGizmo->Destroy();
        m_moveGizmo.reset();
    }
    m_gizmoShader.reset();


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

	m_selectedEntityID = entityID; // set the newly added cube as the selected entity

    BOX_LOG_INFO(
        "Added editable cube. Entity count: "
        << m_entities.size()
    );

    return true;
}

bool BoxEngine::AddEditableSphere(const glm::vec3& position)
{
    const int entityID = m_nextEntityID++;

    const std::string name =
        "Sphere " +
        std::to_string(entityID);

    auto sphere =
        std::make_unique<Entity>(
            entityID,
            name
        );

    sphere->SetPosition(position);

    if (!sphere->CreateSphere())
    {
        return false;
    }

    m_entities.push_back(
        std::move(sphere)
    );

    return true;
}





// Return a const reference to the vector of unique_ptr<Entity> for the editor panels to access the entities in the scene
const std::vector<std::unique_ptr<Entity>>&
BoxEngine::GetEntities() const
{
    return m_entities;
}

void BoxEngine::SetSelectedEntity(int entityID)
{
    {
        for (const auto& entity : m_entities)
        {
            if (entity &&
                entity->GetID() == entityID)
            {
                m_selectedEntityID = entityID;
                return;
            }
        }

        m_selectedEntityID = -1;
    }
}

Entity* BoxEngine::GetSelectedEntity()
{
    for (auto& entity : m_entities)
    {
        if (entity &&
            entity->GetID() ==
            m_selectedEntityID)
        {
            return entity.get();
        }
    }

    return nullptr;
}

const Entity* BoxEngine::GetSelectedEntity() const
{
    for (const auto& entity : m_entities)
    {
        if (entity &&
            entity->GetID() ==
            m_selectedEntityID)
        {
            return entity.get();
        }
    }

    return nullptr;
}

int BoxEngine::GetSelectedEntityID() const
{
    return m_selectedEntityID;
}

void BoxEngine::ClearSelectedEntity()
{
    m_selectedEntityID = -1;
}

bool BoxEngine::RemoveEntity(int entityID)
{
    const auto iterator =
        std::find_if(
            m_entities.begin(),
            m_entities.end(),
            [entityID](const std::unique_ptr<Entity>& entity)
    {
        return entity &&
            entity->GetID() == entityID;
    }
        );

    if (iterator == m_entities.end())
    {
        return false;
    }

    if (m_selectedEntityID == entityID)
    {
        m_selectedEntityID = -1;
    }

    m_entities.erase(iterator);

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

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

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

       

          /* m_sceneShader->setVec3(
                "uBaseColor",
                glm::vec3(
                    0.2f,
                    0.55f,
                    0.9f
                )
            );*/


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
        
        RenderSelectedEntityOutline(view, projection);

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

       


        // ############################################ Tools rendring ##############################################
        Entity* selectedEntity =
            GetSelectedEntity();

        if (selectedEntity &&
            selectedEntity->IsVisible() &&
            m_moveGizmo &&
            m_gizmoShader)
        {
            const float gizmoScale = 1.5f;

            const glm::vec3 entityScale =
                selectedEntity->GetScale();

            const float cubeTop =
                entityScale.y * 0.5f;

            const float gizmoHalfSize =
                0.08f * gizmoScale;

            const glm::vec3 gizmoPosition =
                selectedEntity->GetPosition() +
                glm::vec3(
                    0.0f,
                    cubeTop + gizmoHalfSize,
                    0.0f
                );

            m_moveGizmo->Render(
                *m_gizmoShader,
                gizmoPosition,
                gizmoScale,
                view,
                projection
            );
        }
        
        // ############################################ End tools rendring ##############################################

       
    }

    Framebuffer::Unbind();
}



GLuint BoxEngine::GetSceneTexture() const
{
    return m_sceneFramebuffer.GetColorTexture();
}

bool BoxEngine::RayIntersectsAABB(
    const glm::vec3& rayOriginWorld,
    const glm::vec3& rayDirectionWorld,
    const glm::mat4& modelMatrix,
    const glm::vec3& aabbMinLocal,
    const glm::vec3& aabbMaxLocal,
    float& outDistanceWorld) const
{
    const glm::mat4 inverseModel =
        glm::inverse(modelMatrix);

    const glm::vec3 rayOriginLocal =
        glm::vec3(
            inverseModel *
            glm::vec4(
                rayOriginWorld,
                1.0f
            )
        );

    const glm::vec3 unnormalisedDirectionLocal =
        glm::vec3(
            inverseModel *
            glm::vec4(
                rayDirectionWorld,
                0.0f
            )
        );

    const float directionLength =
        glm::length(
            unnormalisedDirectionLocal
        );

    if (directionLength <= 0.000001f)
    {
        return false;
    }

    const glm::vec3 rayDirectionLocal =
        unnormalisedDirectionLocal /
        directionLength;

    float tMin = 0.0f;

    float tMax =
        std::numeric_limits<float>::max();

    constexpr float epsilon =
        0.000001f;

    for (int axis = 0; axis < 3; ++axis)
    {
        if (std::abs(
            rayDirectionLocal[axis]) <
            epsilon)
        {
            if (rayOriginLocal[axis] <
                aabbMinLocal[axis] ||
                rayOriginLocal[axis] >
                aabbMaxLocal[axis])
            {
                return false;
            }

            continue;
        }

        const float inverseDirection =
            1.0f /
            rayDirectionLocal[axis];

        float t1 =
            (aabbMinLocal[axis] -
                rayOriginLocal[axis]) *
            inverseDirection;

        float t2 =
            (aabbMaxLocal[axis] -
                rayOriginLocal[axis]) *
            inverseDirection;

        if (t1 > t2)
        {
            std::swap(t1, t2);
        }

        tMin =
            std::max(tMin, t1);

        tMax =
            std::min(tMax, t2);

        if (tMin > tMax)
        {
            return false;
        }
    }

    const glm::vec3 localHitPoint =
        rayOriginLocal +
        rayDirectionLocal *
        tMin;

    const glm::vec3 worldHitPoint =
        glm::vec3(
            modelMatrix *
            glm::vec4(
                localHitPoint,
                1.0f
            )
        );

    outDistanceWorld =
        glm::length(
            worldHitPoint -
            rayOriginWorld
        );

    return true;
}

void BoxEngine::PickEntity(
    const glm::vec3& rayOrigin,
    const glm::vec3& rayDirection)
{
    Entity* closestEntity =
        nullptr;

    float closestDistance =
        std::numeric_limits<float>::max();

    for (const auto& entity : m_entities)
    {
        if (!entity ||
            !entity->IsVisible())
        {
            continue;
        }

        float hitDistance = 0.0f;

        const bool hit =
            RayIntersectsAABB(
                rayOrigin,
                rayDirection,
                entity->GetModelMatrix(),
                entity->GetAABBMin(),
                entity->GetAABBMax(),
                hitDistance
            );

        if (hit &&
            hitDistance < closestDistance)
        {
            closestDistance =
                hitDistance;

            closestEntity =
                entity.get();
        }
    }

    if (closestEntity)
    {
        SetSelectedEntity(
            closestEntity->GetID()
        );
    }
    else
    {
        ClearSelectedEntity();
    }
}

void BoxEngine::RenderSelectedEntityOutline(
    const glm::mat4& view,
    const glm::mat4& projection)
{
    if (!m_outlineShader)
    {
        return;
    }

    Entity* selectedEntity =
        GetSelectedEntity();

    if (!selectedEntity ||
        !selectedEntity->IsVisible())
    {
        return;
    }

    m_outlineShader->Use();

    m_outlineShader->setMat4(
        "model",
        selectedEntity->GetModelMatrix()
    );

    m_outlineShader->setMat4(
        "view",
        view
    );

    m_outlineShader->setMat4(
        "projection",
        projection
    );
	// Set the outline scale factor to slightly enlarge the back faces of the selected entity
    m_outlineShader->SetUniformFloat(
        "outlineScale",
        1.02f
    );

    m_outlineShader->setVec3(
        "outlineColor",
        glm::vec3(
            1.0f,
            0.55f,
            0.05f
        )
    );

    /*
     * Draw only the object's back faces.
     * The slightly enlarged back faces appear
     * around the normally rendered cube.
     */
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    selectedEntity->DrawMesh();

    glCullFace(GL_BACK);
}