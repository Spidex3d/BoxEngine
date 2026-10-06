#include "BoxEngine.h"
#include <shader/Shader.h>
#include <entity/Entity.h>
#include <rendering/Grid.h>
#include "camera/Camera.h"
#include <rendering/Textures.h>

#include <Helpers.h>
#include <miniBoxLog.h>

#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <algorithm>

#include <cmath>
#include <limits>

#include <glm/gtc/matrix_inverse.hpp>

#include <mesh/MeshCombiner.h>

#include <fileManager/SceneSerializer.h>

// ----------------------------------------
// Game Engine
// ----------------------------------------
#include <runtime/Collision.h>

// ----------------------------------------

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
        cubeVertexShaderPath, cubeFragmentShaderPath);

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
    

	// ################################################# Default texture ######################################################
    
    m_defaultTexturePath = helpers.GetAssetPath("assets/textures/texture/checkerboard.jpg");

    if (!m_defaultTexture.LoadFromFile(m_defaultTexturePath))
    {
        BOX_LOG_ERROR("Failed to load default texture: " << m_defaultTexturePath);

        return false;
    }

   
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

    m_textures.clear();

    m_camera.reset();

    m_gridShader.reset();
    m_sceneShader.reset();
    m_outlineShader.reset();

    
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

    cube->GetMaterial().SetBaseColorTexture(m_defaultTexture.GetID(), m_defaultTexturePath);


	// Set the base color texture of the cube's material to the m_defaultTexture checkerboard texture
    Material& material = cube->GetMaterial();
   
    material.SetUseBaseColorTexture(true);

    cube->SetPosition(position);

    if (!cube->CreateCube())
    {
        BOX_LOG_ERROR("Failed to add editable cube");

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

bool BoxEngine::AddEditablePlane(const glm::vec3& position)
{
    const int entityID = m_nextEntityID++;

    const std::string entityName = "Plane " + std::to_string(entityID);

    auto plane = std::make_unique<Entity>(entityID, entityName);

    plane->GetMaterial().SetBaseColorTexture(m_defaultTexture.GetID(), m_defaultTexturePath);


    // Set the base color texture of the cube's material to the m_defaultTexture checkerboard texture
    Material& material = plane->GetMaterial();

    material.SetUseBaseColorTexture(true);

    plane->SetPosition(position);

    if (!plane->CreatePlane())
    {
        BOX_LOG_ERROR("Failed to add editable plane");

        return false;
    }

    m_entities.push_back(std::move(plane));

    m_selectedEntityID = entityID; // set the newly added cube as the selected entity

    BOX_LOG_INFO(
        "Added editable plane. Entity count: " << m_entities.size());

    return true;
       
}

// -------------------------- My Create a floor primitive with subdivisions, width and depth --------------------------
bool BoxEngine::AddEditableFloor(const glm::vec3& position, float width, float depth, int subdivisionsX, int subdivisionsZ)
{
    const int entityID = m_nextEntityID++;

    const std::string entityName = "Floor " + std::to_string(entityID);

    auto floor = std::make_unique<Entity>(entityID, entityName);

    floor->GetMaterial().SetBaseColorTexture(m_defaultTexture.GetID(), m_defaultTexturePath);

    Material& material = floor->GetMaterial();

    material.SetUseBaseColorTexture(true);

    floor->SetPosition(position);

    if (!floor->CreateFloor(width, depth, subdivisionsX, subdivisionsZ))
    {
		BOX_LOG_ERROR("Failed to add editable floor");
		return false;
    }

    m_entities.push_back(std::move(floor));

    m_selectedEntityID = entityID; // set the newly added floor as the selected entity

    BOX_LOG_INFO(
        "Added editable floor. Entity count: " << m_entities.size());

	return true;

}
//bool BoxEngine::AddEditableRock(const glm::vec3& position)
bool BoxEngine::AddEditableRock(const glm::vec3& position, float radius,
    int subdivisions, float roughness, std::uint32_t seed, float flattening)
{
    BOX_LOG_INFO(
        "AddEditableRock - Seed = "
        << seed
    );

    const int entityID = m_nextEntityID++;

    const std::string name =
        "Rock " + std::to_string(entityID);

    auto rock =
        std::make_unique<Entity>(entityID, name);

    // -------------------------------------------------
    // Default material
    // -------------------------------------------------

    rock->GetMaterial().SetBaseColorTexture(
        m_defaultTexture.GetID(),
        m_defaultTexturePath
    );

    Material& material =
        rock->GetMaterial();

    material.SetUseBaseColorTexture(true);

    // -------------------------------------------------
    // Position
    // -------------------------------------------------

    rock->SetPosition(position);

    // -------------------------------------------------
    // Create IcoSphere based rock
    // -------------------------------------------------

    if (!rock->CreateRock(
        subdivisions,
        radius,
        roughness,
        seed,
        flattening))
    {
        BOX_LOG_ERROR(
            "Failed to add editable rock"
        );

        return false;
    }

    m_entities.push_back(std::move(rock));

    m_selectedEntityID = entityID;

    return true;
}

// ------------------------------ End Ecosystem Meshes ------------------------------

bool BoxEngine::AddEditableIcoSphere(const glm::vec3& position, int recursionLevel)
{

	const int entityID = m_nextEntityID++;
    
    const std::string name = "IcoSphere " + std::to_string(entityID);

    auto icoSphere = std::make_unique<Entity>(entityID, name);

    icoSphere->GetMaterial().SetBaseColorTexture(m_defaultTexture.GetID(), m_defaultTexturePath);

    Material& material = icoSphere->GetMaterial();

    material.SetUseBaseColorTexture(true);

    icoSphere->SetPosition(position);

    if (!icoSphere->CreateIcoSphere(recursionLevel))
    {
        BOX_LOG_ERROR("Failed to add editable icosphere");
        return false;
    }

    m_entities.push_back(std::move(icoSphere));

    m_selectedEntityID = entityID; // set the newly added icosphere as the selected entity

    BOX_LOG_INFO("Added editable icosphere. Entity count: " << m_entities.size());

    return true;
}

bool BoxEngine::AddEditableCapsule(const glm::vec3& position, int sectors, int stacks, float radius, float height)
{
	const int entityID = m_nextEntityID++;
	const std::string name = "Capsule " + std::to_string(entityID);
	auto capsule = std::make_unique<Entity>(entityID, name);
	capsule->GetMaterial().SetBaseColorTexture(m_defaultTexture.GetID(), m_defaultTexturePath);
	Material& material = capsule->GetMaterial();
	material.SetUseBaseColorTexture(true);
	capsule->SetPosition(position);
	if (!capsule->CreateCapsule(sectors, stacks, radius, height))
	{
		BOX_LOG_ERROR("Failed to add editable capsule");
		return false;
	}
	m_entities.push_back(std::move(capsule));
	m_selectedEntityID = entityID; // set the newly added capsule as the selected entity
	BOX_LOG_INFO("Added editable capsule. Entity count: " << m_entities.size());
	return true;
}


bool BoxEngine::AddEditableSphere(const glm::vec3& position)
{
    const int entityID = m_nextEntityID++;

    const std::string name = "Sphere " + std::to_string(entityID);

    auto sphere = std::make_unique<Entity>(entityID, name);


    sphere->GetMaterial().SetBaseColorTexture(m_defaultTexture.GetID(), m_defaultTexturePath);
  
	// Set the base color texture of the sphere's material to the m_defaultTexture checkerboard texture
    Material& material = sphere->GetMaterial();
   
    material.SetUseBaseColorTexture(true);

    sphere->SetPosition(position);

    if (!sphere->CreateSphere())
    {
        return false;
    }

    m_entities.push_back(std::move(sphere));

    return true;
}

bool BoxEngine::AddEditableCylinder(const glm::vec3& position, int sectors, int stacks, float radius, float height)
{
    const int entityID = m_nextEntityID++;

    const std::string entityName = "Cylinder " + std::to_string(entityID);

    auto cylinder = std::make_unique<Entity>(entityID, entityName);

    cylinder->GetMaterial().SetBaseColorTexture(m_defaultTexture.GetID(), m_defaultTexturePath);

    Material& material = cylinder->GetMaterial();

    material.SetUseBaseColorTexture(true);

    cylinder->SetPosition(position);

    if (!cylinder->CreateCylinder(sectors, stacks, radius, height))
    {
        BOX_LOG_ERROR("Failed to add editable cylinder");
        return false;
    }

    m_entities.push_back(std::move(cylinder));

    m_selectedEntityID = entityID; // set the newly added cylinder as the selected entity

    BOX_LOG_INFO(
        "Added editable cylinder. Entity count: " << m_entities.size());

    return true;
}

bool BoxEngine::AddEditablePyramid(const glm::vec3& position)
{
    const int entityID = m_nextEntityID++;

    const std::string entityName = "Pyramid " + std::to_string(entityID);

    auto pyramid = std::make_unique<Entity>(entityID, entityName);

    pyramid->GetMaterial().SetBaseColorTexture(m_defaultTexture.GetID(), m_defaultTexturePath);


    // Set the base color texture of the cube's material to the m_defaultTexture checkerboard texture
    Material& material = pyramid->GetMaterial();

    material.SetUseBaseColorTexture(true);

    pyramid->SetPosition(position);

    if (!pyramid->CreatePyramid())
    {
        BOX_LOG_ERROR("Failed to add editable pyramid");

        return false;
    }

    m_entities.push_back(std::move(pyramid));

    m_selectedEntityID = entityID; // set the newly added cube as the selected entity

    BOX_LOG_INFO(
        "Added editable pyramid. Entity count: " << m_entities.size());

    return true;
}

bool BoxEngine::AddEditableCone(const glm::vec3& position, int sectors, float radius, float height)
{
    const int entityID = m_nextEntityID++;

    const std::string entityName = "Cone " + std::to_string(entityID);

    auto cone = std::make_unique<Entity>(entityID, entityName);

    cone->GetMaterial().SetBaseColorTexture(m_defaultTexture.GetID(), m_defaultTexturePath);

    Material& material = cone->GetMaterial();

    material.SetUseBaseColorTexture(true);

    cone->SetPosition(position);

    if (!cone->CreateCone(sectors, radius, height))
    {
        BOX_LOG_ERROR("Failed to add editable cone");
        return false;
    }

    m_entities.push_back(std::move(cone));

    m_selectedEntityID = entityID; // set the newly added cone as the selected entity

    BOX_LOG_INFO(
        "Added editable cone. Entity count: " << m_entities.size());

    return true;
}

bool BoxEngine::AddEditableTorus(const glm::vec3& position, int sides, int rings, float innerRadius, float outerRadius)
{
    
	const int entityID = m_nextEntityID++;
	const std::string entityName = "Torus " + std::to_string(entityID);
	auto torus = std::make_unique<Entity>(entityID, entityName);
	torus->GetMaterial().SetBaseColorTexture(m_defaultTexture.GetID(), m_defaultTexturePath);
	Material& material = torus->GetMaterial();
	material.SetUseBaseColorTexture(true);
	torus->SetPosition(position);
	if (!torus->CreateTorus(sides, rings, innerRadius, outerRadius))
	{
		BOX_LOG_ERROR("Failed to add editable torus");
		return false;
	}
	m_entities.push_back(std::move(torus));
	m_selectedEntityID = entityID; // set the newly added torus as the selected entity
	BOX_LOG_INFO(
		"Added editable torus. Entity count: " << m_entities.size());
	return true;
}

    // -----------------------------------------------------------------
    // ------------------------- JoinEntities --------------------------
    // -----------------------------------------------------------------

Entity* BoxEngine::JoinEntities(Entity* first, Entity* second)
{
    if (!first || !second)
    {
        return nullptr;
    }

    if (first == second)
    {
        return nullptr;
    }

    //MeshData combinedMesh;
    MeshEditing combinedMesh;


    if (!MeshCombiner::Combine(
        *first,
        *second,
        combinedMesh))
    {
        BOX_LOG_ERROR(
            "Failed to combine entities"
        );

        return nullptr;
    }

    const int entityID =
        m_nextEntityID++;

    auto joinedEntity =
        std::make_unique<Entity>(
            entityID,
            "Joined Object"
        );
    // color slot stuff
    // -------------------------------------------------
// Copy material slots from both source entities.
// -------------------------------------------------

    joinedEntity->ClearMaterialSlots();


    // Entity A materials
    for (std::size_t i = 0;
        i < first->GetMaterialSlotCount();
        ++i)
    {
        joinedEntity->AddMaterialSlot(
            first->GetMaterialSlot(i)
        );
    }


    // Entity B materials
    for (std::size_t i = 0;
        i < second->GetMaterialSlotCount();
        ++i)
    {
        joinedEntity->AddMaterialSlot(
            second->GetMaterialSlot(i)
        );
    }

    // ############


    // Because the vertices have already had the
    // original transforms baked into them.
    joinedEntity->SetPosition(
        glm::vec3(0.0f)
    );

    joinedEntity->SetRotation(
        glm::vec3(0.0f)
    );

    joinedEntity->SetScale(
        glm::vec3(1.0f)
    );

    joinedEntity->GetEditableMesh() =
        combinedMesh;

    joinedEntity->GetBaseEditableMesh() =
        combinedMesh;

    if (!joinedEntity->RebuildFromEditableMesh())
    {
        BOX_LOG_ERROR(
            "Failed to build joined editable entity"
        );

        return nullptr;
    }


    Entity* result =
        joinedEntity.get();

    m_entities.push_back(
        std::move(joinedEntity)
    );

    m_selectedEntityID =
        entityID;

    BOX_LOG_INFO(
        "Joined two entities into new object"
    );

    return result;
}

void BoxEngine::AddSelectedEntity(int entityID)
{
    for (int id : m_selectedEntityIDs)
    {
        if (id == entityID)
        {
            return;
        }
    }

    m_selectedEntityIDs.push_back(entityID);

    m_selectedEntityID = entityID;
}

void BoxEngine::ClearSelectedEntities()
{
    m_selectedEntityIDs.clear();

    m_selectedEntityID = -1;
}

const std::vector<int>&
BoxEngine::GetSelectedEntityIDs() const
{
    return m_selectedEntityIDs;
}

Entity* BoxEngine::GetEntityByID(
    int entityID)
{
    for (auto& entity : m_entities)
    {
        if (entity &&
            entity->GetID() == entityID)
        {
            return entity.get();
        }
    }

    return nullptr;
}

// --------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------- End JoinEntities -------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------------------

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
GLuint BoxEngine::LoadTexture(const std::string& path)
{
    if (path.empty())
    {
        BOX_LOG_ERROR(
            "BoxEngine::LoadTexture received an empty path"
        );

        return 0;
    }

    auto texture =
        std::make_unique<Texture>();

    if (!texture->LoadFromFile(path))
    {
        BOX_LOG_ERROR(
            "Failed to load texture: "
            << path
        );

        return 0;
    }

    const GLuint textureID =
        texture->GetID();

    BOX_LOG_INFO(
        "Texture loaded: "
        << path
        << " ID="
        << textureID
        << " Size="
        << texture->GetWidth()
        << "x"
        << texture->GetHeight()
    );

    m_textures.push_back(
        std::move(texture)
    );

    return textureID;
}

Entity* BoxEngine::AddImportedMesh(
    const std::string& name,
    const MeshData& meshData)
{
    const int entityID = m_nextEntityID++;

    auto entity =
        std::make_unique<Entity>(
            entityID,
            name
        );

    if (!entity->CreateFromMeshData(
        meshData))
    {
        BOX_LOG_ERROR("Failed to create imported entity");

        return nullptr;
    }

    Entity* result = entity.get();

    m_entities.push_back(std::move(entity));

    m_selectedEntityID = entityID;

    return result;
}



void BoxEngine::ResizeSceneViewport(
    int width,
    int height)
{
    m_sceneFramebuffer.Resize(width, height);
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

    glClearColor(0.12f, 0.15f, 0.18f, 1.0f);

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


        // ################################## Grid Rendering ########################################
        if (m_grid && m_gridShader)
        {
            m_gridShader->Use();

            m_gridShader->setVec3(
                "uGridColor",
                glm::vec3(0.35f)
            );

            m_grid->RenderPreview(*m_gridShader, view, projection);
        }
        
        RenderSelectedEntityOutline(view, projection);
		// ############################### new RenderScene ########################################
        const int viewportWidth =
            m_sceneFramebuffer.GetWidth();

        const int viewportHeight =
            m_sceneFramebuffer.GetHeight();

        float aspectRatio = 1.0f;

        if (viewportHeight > 0)
        {
            aspectRatio =
                static_cast<float>(viewportWidth) /
                static_cast<float>(viewportHeight);
        }

        for (const auto& entity : m_entities)
        {
            if (entity)
            {
                entity->RenderScene(*m_sceneShader, *m_camera, aspectRatio);
            }
        }

      
       
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
    // --------------------------------------
    glm::vec3 safeMin = aabbMinLocal;
    glm::vec3 safeMax = aabbMaxLocal;

    constexpr float minimumThickness = 0.02f;

    for (int axis = 0; axis < 3; ++axis)
    {
        if ((safeMax[axis] - safeMin[axis]) < minimumThickness)
        {
            const float centre =
                (safeMin[axis] + safeMax[axis]) * 0.5f;

            safeMin[axis] =
                centre - minimumThickness * 0.5f;

            safeMax[axis] =
                centre + minimumThickness * 0.5f;
        }
    }

    // --------------------------------------

    for (int axis = 0; axis < 3; ++axis)
    {
        if (std::abs(rayDirectionLocal[axis]) < epsilon)
        {
            if (rayOriginLocal[axis] < safeMin[axis] ||
                rayOriginLocal[axis] > safeMax[axis])
            {
                return false;
            }

            continue;
        }

        const float inverseDirection =
            1.0f / rayDirectionLocal[axis];

        float t1 =
            (safeMin[axis] - rayOriginLocal[axis]) *
            inverseDirection;

        float t2 =
            (safeMax[axis] - rayOriginLocal[axis]) *
            inverseDirection;

        if (t1 > t2)
        {
            std::swap(t1, t2);
        }

        tMin = std::max(tMin, t1);
        tMax = std::min(tMax, t2);

        if (tMin > tMax)
        {
            return false;
        }
    }

    /*for (int axis = 0; axis < 3; ++axis)
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
    }*/

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
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    selectedEntity->DrawMesh();

    glCullFace(GL_BACK);
    glDepthMask(GL_TRUE);
    
}

bool BoxEngine::SaveScene(const std::filesystem::path& filePath)
{
    SceneSerializer serializer;

    return serializer.SerializeScene(
        filePath,
        m_entities
    );
}
bool BoxEngine::LoadScene(
    const std::filesystem::path& filePath)
{
    SceneSerializer serializer;

    std::vector<std::unique_ptr<Entity>>
        loadedEntities;

    if (!serializer.DeserializeScene(
        filePath,
        loadedEntities))
    {
        BOX_LOG_ERROR(
            "Failed to load scene: "
            << filePath.string()
        );

        return false;
    }

    // =================================================
    // RESTORE MATERIAL TEXTURES
    // =================================================

    for (auto& entity : loadedEntities)
    {
        if (!entity)
            continue;

        for (std::size_t i = 0;
            i < entity->GetMaterialSlotCount();
            ++i)
        {
            Material& material = entity->GetMaterialSlot(i);

            BOX_LOG_INFO(
                "Restoring material "
                << i
                << " Name=["
                << material.GetName()
                << "]"
                << " UseBase="
                << material.UsesBaseColorTexture()
                << " BasePath=["
                << material.GetBaseColorTexturePath()
                << "]"
                << " UseNormal="
                << material.UsesNormalTexture()
                << " NormalPath=["
                << material.GetNormalTexturePath()
                << "]"
            );

            // -----------------------------------------
            // Base colour texture
            // -----------------------------------------
           
            const std::string basePath =
                material.GetBaseColorTexturePath();

            if (!basePath.empty())
            {
                const GLuint textureID =
                    LoadTexture(basePath);

                if (textureID != 0)
                {
                    material.SetBaseColorTexture(
                        textureID,
                        basePath
                    );

                    BOX_LOG_INFO(
                        "Restored base color texture: "
                        << basePath
                    );
                }
            }
            
            
            

            // -----------------------------------------
            // Normal map
            // -----------------------------------------
            
            const std::string normalPath =
                material.GetNormalTexturePath();

            if (!normalPath.empty())
            {
                const GLuint textureID =
                    LoadTexture(normalPath);

                if (textureID != 0)
                {
                    material.SetNormalTexture(
                        textureID,
                        normalPath
                    );

                    BOX_LOG_INFO(
                        "Restored normal map: "
                        << normalPath
                    );
                }
            }

        }
    }


    // -----------------------------------------
    // Find the next available entity ID.
    // -----------------------------------------
    int nextEntityID = 0;

    for (const auto& entity : loadedEntities)
    {
        if (!entity)
            continue;

        nextEntityID =
            std::max(
                nextEntityID,
                entity->GetID() + 1
            );
    }

    // =================================================
// RESTORE MATERIAL TEXTURES
// =================================================

    for (auto& entity : loadedEntities)
    {
        if (!entity)
            continue;

        for (std::size_t i = 0;
            i < entity->GetMaterialSlotCount();
            ++i)
        {
            Material& material =
                entity->GetMaterialSlot(i);

            // -----------------------------------------
            // Base colour texture
            // -----------------------------------------
            const std::string baseTexturePath =
                material.GetBaseColorTexturePath();

            if (material.UsesBaseColorTexture() &&
                !baseTexturePath.empty())
            {
                const GLuint textureID =
                    LoadTexture(baseTexturePath);

                if (textureID != 0)
                {
                    material.SetBaseColorTexture(
                        textureID,
                        baseTexturePath
                    );

                    BOX_LOG_INFO(
                        "Restored base texture: "
                        << baseTexturePath
                    );
                }
                else
                {
                    BOX_LOG_ERROR(
                        "Failed to restore base texture: "
                        << baseTexturePath
                    );
                }
            }

            // -----------------------------------------
            // Normal texture
            // -----------------------------------------
            const std::string normalTexturePath =
                material.GetNormalTexturePath();

            if (material.UsesNormalTexture() &&
                !normalTexturePath.empty())
            {
                const GLuint textureID =
                    LoadTexture(normalTexturePath);

                if (textureID != 0)
                {
                    material.SetNormalTexture(
                        textureID,
                        normalTexturePath
                    );

                    BOX_LOG_INFO(
                        "Restored normal texture: "
                        << normalTexturePath
                    );
                }
                else
                {
                    BOX_LOG_ERROR(
                        "Failed to restore normal texture: "
                        << normalTexturePath
                    );
                }
            }
        }
    }

    // -----------------------------------------
    // Scene loaded successfully.
    // Replace the current scene.
    // -----------------------------------------
    ClearSelectedEntity();

    m_entities =
        std::move(loadedEntities);

    m_nextEntityID =
        nextEntityID;

    BOX_LOG_INFO(
        "Scene loaded successfully. Entities="
        << m_entities.size()
    );

    return true;
}

// ----------------------------------------------------------------------------------------------------
// ------------------------------------ Game Engine Mode Management -----------------------------------
// ----------------------------------------------------------------------------------------------------

void BoxEngine::StartPlayMode()
{
    if (m_engineMode == EngineMode::Playing)
    {
        return;
    }

    m_engineMode = EngineMode::Playing;

    m_camera->Mode =
        Camera::CameraMode::Play;

    m_camera->SetPerspectiveFromDefaults();

    // -------------------------------------------------
    // Start the runtime Game
    // This creates the Player through Player::Initialize()
    // -------------------------------------------------

    if (!m_game.Initialize(*this))
    {
        BOX_LOG_ERROR(
            "Failed to initialize Game"
        );

        m_engineMode = EngineMode::Editor;
        return;
    }

    BOX_LOG_INFO(
        "Play mode started"
    );
}

void BoxEngine::PausePlayMode()
{
    if (m_engineMode == EngineMode::Playing)
    {
        m_engineMode = EngineMode::Paused;

        BOX_LOG_INFO("Play mode paused");
    }
}

void BoxEngine::StopPlayMode()
{
    if (m_engineMode == EngineMode::Editor)
    {
        return;
    }

    // -------------------------------------------------
    // Remove runtime Player
    // -------------------------------------------------

    m_game.Shutdown(*this);

    // -------------------------------------------------
    // Back to editor
    // -------------------------------------------------

    m_engineMode =
        EngineMode::Editor;

    m_camera->Mode =
        Camera::CameraMode::EditorOrbit;

    BOX_LOG_INFO(
        "Play mode stopped"
    );
}


// The New bit
Entity* BoxEngine::CreateRuntimePlayer(const glm::vec3& position)
{
    const int entityID =
        m_nextEntityID++;

    auto player =
        std::make_unique<Entity>(
            entityID,
            "Player"
        );

    player->SetPosition(
        position
    );

    if (!player->CreateCapsule(
        16,
        8,
        0.4f,
        1.8f
    ))
    {
        BOX_LOG_ERROR(
            "Failed to create runtime Player"
        );

        return nullptr;
    }

    Entity* result =
        player.get();

    m_entities.push_back(
        std::move(player)
    );

    return result;
}

bool BoxEngine::PlayerCollidesAt(const glm::vec3& position, const Entity* ignoreEntity)
{
    Entity* playerEntity =
        m_game.GetPlayer().GetEntity();

    if (!playerEntity)
    {
        return false;
    }

    constexpr float playerRadius = 0.4f;
    constexpr float playerHalfHeight = 0.9f;
    constexpr float groundTolerance = 0.05f;

    const float playerFeetY =
        position.y -
        playerHalfHeight;

    for (const auto& entity : m_entities)
    {
        if (!entity)
        {
            continue;
        }

        // Don't collide with Player.
        if (entity.get() == playerEntity)
        {
            continue;
        }

        // Don't treat the surface we're walking on
        // as a wall.
        if (entity.get() == ignoreEntity)
        {
            continue;
        }

        const glm::vec3 entityPosition =
            entity->GetPosition();

       /* const glm::vec3 worldMin =
            entityPosition +
            entity->GetAABBMin();

        const glm::vec3 worldMax =
            entityPosition +
            entity->GetAABBMax();*/

       /* if (worldMax.y <=
            playerFeetY + groundTolerance)
        {
            continue;
        }*/

        /*if (Collision::CapsuleVsAABB(
            position,
            playerRadius,
            playerHalfHeight,
            worldMin,
            worldMax))
        {
            return true;
        }*/
    }

    return false;
}

bool BoxEngine::GetGroundHeightAt(const glm::vec3& position, float& outGroundY)
{
    bool foundGround = false;

    float highestGround =
        -std::numeric_limits<float>::max();

    for (const auto& entity : m_entities)
    {
        if (!entity)
        {
            continue;
        }

        // Skip runtime Player
        Entity* playerEntity =
            m_game.GetPlayer().GetEntity();

        if (entity.get() == playerEntity)
        {
            continue;
        }

        const glm::vec3 entityPosition =
            entity->GetPosition();

        const glm::vec3 worldMin =
            entityPosition +
            entity->GetAABBMin();

        const glm::vec3 worldMax =
            entityPosition +
            entity->GetAABBMax();

        // Is Player horizontally over this AABB?
        const bool insideX =
            position.x >= worldMin.x &&
            position.x <= worldMax.x;

        const bool insideZ =
            position.z >= worldMin.z &&
            position.z <= worldMax.z;

        if (!insideX || !insideZ)
        {
            continue;
        }

        // Only accept surfaces below or near Player
        if (worldMax.y <= position.y + 0.5f)
        {
            if (worldMax.y > highestGround)
            {
                highestGround =
                    worldMax.y;

                foundGround = true;
            }
        }
    }

    if (foundGround)
    {
        outGroundY =
            highestGround;

        return true;
    }

    return false;
}

bool BoxEngine::FindGround(const glm::vec3& position, float maxDistance, GroundHit& outHit)
{
    outHit = GroundHit{};

    const glm::vec3 rayOrigin =
        position;

    const glm::vec3 rayDirection =
        glm::vec3(0.0f, -1.0f, 0.0f);

    float closestDistance =
        maxDistance;

    bool foundGround =
        false;

    Entity* playerEntity =
        m_game.GetPlayer().GetEntity();

    // -------------------------------------------------
    // Check all scene entities
    // -------------------------------------------------

    for (const auto& entity : m_entities)
    {
        if (!entity)
        {
            continue;
        }

        // Never test Player against itself.
        if (entity.get() == playerEntity)
        {
            continue;
        }

        if (!entity->IsCollisionEnabled())
        {
            continue;
        }

        const MeshData& mesh =
            entity->GetMeshData();

        if (mesh.vertices.empty() ||
            mesh.indices.size() < 3)
        {
            continue;
        }

        const glm::mat4 model =
            entity->GetModelMatrix();

        // -------------------------------------------------
        // Test each triangle
        // -------------------------------------------------

        for (std::size_t i = 0;
            i + 2 < mesh.indices.size();
            i += 3)
        {
            const unsigned int index0 =
                mesh.indices[i];

            const unsigned int index1 =
                mesh.indices[i + 1];

            const unsigned int index2 =
                mesh.indices[i + 2];

            // -------------------------------------------------
            // Get local-space triangle vertices
            // -------------------------------------------------

            const glm::vec3 localV0 =
                mesh.vertices[index0].position;

            const glm::vec3 localV1 =
                mesh.vertices[index1].position;

            const glm::vec3 localV2 =
                mesh.vertices[index2].position;

            // -------------------------------------------------
            // Transform triangle into world space
            // -------------------------------------------------

            const glm::vec3 worldV0 =
                glm::vec3(
                    model *
                    glm::vec4(
                        localV0,
                        1.0f
                    )
                );

            const glm::vec3 worldV1 =
                glm::vec3(
                    model *
                    glm::vec4(
                        localV1,
                        1.0f
                    )
                );

            const glm::vec3 worldV2 =
                glm::vec3(
                    model *
                    glm::vec4(
                        localV2,
                        1.0f
                    )
                );

            float hitDistance = 0.0f;

            glm::vec3 hitPoint =
                glm::vec3(0.0f);

            glm::vec3 hitNormal =
                glm::vec3(0.0f, 1.0f, 0.0f);

            if (!Collision::RayVsTriangle(
                rayOrigin,
                rayDirection,
                worldV0,
                worldV1,
                worldV2,
                hitDistance,
                hitPoint,
                hitNormal))
            {
                continue;
            }

            if (hitDistance >
                closestDistance)
            {
                continue;
            }

            closestDistance =
                hitDistance;

            outHit.hit =
                true;

            outHit.point =
                hitPoint;

            outHit.normal =
                hitNormal;

            outHit.distance =
                hitDistance;

            outHit.entity =
                entity.get();

            foundGround =
                true;
        }
    }

    return foundGround;
}

bool BoxEngine::CheckCharacterCollision(const glm::vec3& position, float radius, float halfHeight, CollisionHit& outHit)
{
    outHit = CollisionHit{};

    constexpr float collisionEpsilon = 0.0001f;

    bool foundCollision = false;

    float deepestPenetration = 0.0f;

    Entity* playerEntity =
        m_game.GetPlayer().GetEntity();

    for (const auto& entity : m_entities)
    {
        if (!entity)
        {
            continue;
        }

        if (entity.get() == playerEntity)
        {
            continue;
        }

        if (!entity->IsCollisionEnabled())
        {
            continue;
        }

        const MeshData& mesh =
            entity->GetMeshData();

        if (mesh.vertices.empty() ||
            mesh.indices.size() < 3)
        {
            continue;
        }

        const glm::mat4 model =
            entity->GetModelMatrix();

        for (std::size_t i = 0;
            i + 2 < mesh.indices.size();
            i += 3)
        {
            const unsigned int index0 =
                mesh.indices[i];

            const unsigned int index1 =
                mesh.indices[i + 1];

            const unsigned int index2 =
                mesh.indices[i + 2];

            const glm::vec3 localV0 =
                mesh.vertices[index0].position;

            const glm::vec3 localV1 =
                mesh.vertices[index1].position;

            const glm::vec3 localV2 =
                mesh.vertices[index2].position;

            const glm::vec3 worldV0 =
                glm::vec3(
                    model *
                    glm::vec4(localV0, 1.0f)
                );

            const glm::vec3 worldV1 =
                glm::vec3(
                    model *
                    glm::vec4(localV1, 1.0f)
                );

            const glm::vec3 worldV2 =
                glm::vec3(
                    model *
                    glm::vec4(localV2, 1.0f)
                );

            CollisionHit hit;

            if (!Collision::CapsuleVsTriangle(
                position,
                radius,
                halfHeight,
                worldV0,
                worldV1,
                worldV2,
                hit))
            {
                continue;
            }
            if (hit.penetration <=
                collisionEpsilon)
            {
                continue;
            }

            if (!hit.hit)
            {
                continue;
            }

            if (hit.penetration >
                deepestPenetration)
            {
                deepestPenetration =
                    hit.penetration;

                outHit =
                    hit;

                outHit.entity =
                    entity.get();

                foundCollision =
                    true;
            }
        }
    }

    return foundCollision;
}

// ----------------------------------------------------------------------------------------------------
// Collectibles and Runtime Entities
// ----------------------------------------------------------------------------------------------------
Entity* BoxEngine::CreateRuntimeCollectible(const glm::vec3& position)
{
    const int entityID = m_nextEntityID++;

    auto collectible = std::make_unique<Entity>(entityID, "Collectible");

    collectible->SetPosition(position);
	collectible->SetScale(glm::vec3(0.3f));

    collectible->SetCollisionEnabled(false);

    if (!collectible->CreateIcoSphere(1))
    {
        BOX_LOG_ERROR("Failed to create runtime collectible");

        return nullptr;
    }

    Entity* result = collectible.get();

    m_entities.push_back(std::move(collectible));

    return result;
}




void BoxEngine::DestroyRuntimeEntity(Entity* entity)
{
    if (!entity)
    {
        return;
    }

    m_entities.erase(
        std::remove_if(
            m_entities.begin(),
            m_entities.end(),
            [entity](
                const std::unique_ptr<Entity>& e)
    {
        return e.get() == entity;
    }
        ),
        m_entities.end()
    );
}