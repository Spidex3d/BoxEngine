#define GLM_ENABLE_EXPERIMENTAL

#include "BoxEngine.h"
#include <shader/Shader.h>
#include <entity/Entity.h>
#include <rendering/Grid.h>
#include "camera/Camera.h"
#include <rendering/Textures.h>

#include <Helpers.h>
#include <miniBoxLog.h>


#include <string>
#include <algorithm>

#include <cmath>
#include <limits>
#include <array>

#include <random>
#include <cstdint>

#include <unordered_map>

#include <mesh/MeshCombiner.h>

#include <fileManager/SceneSerializer.h>

// Skybox
#include <rendering/Sky.h>
// Terrain
#include <rendering/Terrain.h>
#include <rendering/TerrainBrush.h>
// water
#include <rendering/Water.h>
// test grass blade
#include <mesh/ParticleSystem/GrassBlade.h>

#include <mesh/ParticleSystem/ParticleScatter.h>
#include <mesh/ParticleSystem/GrassClump.h>
#include <mesh/modifiers/ModifierData.h>
#include <mesh/ParticleSystem/PlantPreview.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
// Rendering
#include <rendering/Render.h>
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

	// ################################################# Skybox shader ######################################################
    const std::string skyVertexPath =
        helpers.GetAssetPath(
            "assets/shader/SkyBox.vert"
        );

    const std::string skyFragmentPath =
        helpers.GetAssetPath(
            "assets/shader/SkyBox.frag"
        );

    m_skyShader =
        std::make_unique<Shader>(
            skyVertexPath,
            skyFragmentPath
        );

    if (!m_skyShader ||
        m_skyShader->ID() == 0)
    {
        BOX_LOG_ERROR(
            "Failed to create skybox shader"
        );

        return false;
    }


    m_sky =
        std::make_unique<Sky>();

    if (!m_sky->Initialize())
    {
        BOX_LOG_ERROR(
            "Failed to initialize skybox"
        );

        m_sky.reset();

        return false;
    }
	// ################################################ Terrain shader ######################################################
    const std::string terrainVertexPath =
        helpers.GetAssetPath(
            "assets/shader/Terrain.vert"
        );

    const std::string terrainFragmentPath =
        helpers.GetAssetPath(
            "assets/shader/Terrain.frag"
        );

    m_terrainShader =
        std::make_unique<Shader>(
            terrainVertexPath,
            terrainFragmentPath
        );

    if (!m_terrainShader || m_terrainShader->ID() == 0)
    {
        BOX_LOG_ERROR("Failed to create terrain shader");

        return false;
    }

    // =====================================================
    // GRASS BLADE SHADER
    // =====================================================

    const std::string grassVertexShaderPath =
        helpers.GetAssetPath(
            "assets/shader/grassBlade.vert"
        );

    const std::string grassFragmentShaderPath =
        helpers.GetAssetPath(
            "assets/shader/grassBlade.frag"
        );

    m_grassBladeShader =
        std::make_unique<Shader>(
            grassVertexShaderPath,
            grassFragmentShaderPath
        );

    if (!m_grassBladeShader ||
        m_grassBladeShader->ID() == 0)
    {
        BOX_LOG_ERROR(
            "Failed to create grass blade shader"
        );

        m_grassBladeShader.reset();

        return false;
    }

    BOX_LOG_INFO("Grass Blade Shader initialized");


    // =====================================================
    // CREATE SHARED GRASS BLADE MESH
    // =====================================================

    m_testGrassBlade = std::make_unique<GrassBlade>();

    if (!m_testGrassBlade->CreateGrassBlade())
    {
        BOX_LOG_ERROR(
            "Failed to create shared grass blade mesh"
        );

        m_testGrassBlade.reset();
        return false;
    }

    // =====================================================
    // CREATE SHARED BROKEN GRASS BLADE
    // =====================================================

    m_brokenGrassBlade = std::make_unique<GrassBlade>();

    m_brokenGrassBlade->SetBreakPosition(0.80f);
    m_brokenGrassBlade->SetBreakAngle(160.0f);

    if (!m_brokenGrassBlade->CreateBrokenGrassBlade())
    {
        BOX_LOG_ERROR(
            "Failed to create broken grass blade mesh"
        );

        m_brokenGrassBlade.reset();

        return false;
    }

    // =====================================================
    // SEPARATE TEST GRASS PREVIEW
    // =====================================================

    m_previewGrassBlade =
        std::make_unique<GrassBlade>();

    if (!m_previewGrassBlade->CreateGrassBlade())
    {
        BOX_LOG_ERROR(
            "Failed to create grass preview mesh"
        );

        m_previewGrassBlade.reset();
        return false;
    }

    // =====================================================
    // CREATE BROKEN GRASS PREVIEW
    // =====================================================

    m_previewBrokenBlade =
        std::make_unique<GrassBlade>();

    // Initial test settings.
	m_previewBrokenBlade->SetBreakPosition(0.80f); // 65% 70% 80% of the way up the blade
	m_previewBrokenBlade->SetBreakAngle(125.0f);    // 125 plus degrees for a more natural bend

    // Generate the broken geometry.
    if (!m_previewBrokenBlade->CreateBrokenGrassBlade())
    {
        BOX_LOG_ERROR(
            "Failed to create broken grass preview mesh"
        );

        m_previewBrokenBlade.reset();
        return false;
    }

    BOX_LOG_INFO("Broken grass preview initialized");

// ============================================
// PARTICLE SYSTEM - SMALL ROCK
// ============================================

    if (m_particleSystemRender.LoadSmallRock())
    {
        // Create the rock geometry on the GPU.
        if (!m_particleSystemRender.CreateSmallRockBuffers())
        {
            BOX_LOG_ERROR(
                "Failed to create Small Rock GPU buffers"
            );
        }

        // Get the albedo texture path saved in the MBX.
        const std::filesystem::path albedoPath =
            m_particleSystemRender.GetSmallRockAlbedoPath();

        if (!albedoPath.empty())
        {
            m_smallRockAlbedoTexture =
                LoadTexture(albedoPath.string());

            if (m_smallRockAlbedoTexture == 0)
            {
                BOX_LOG_ERROR(
                    "Failed to load rock albedo: "
                    << albedoPath.string()
                );
            }
        }
        else
        {
            BOX_LOG_WARNING(
                "Small Rock has no albedo texture path"
            );
        }
    }
    else
    {
        BOX_LOG_ERROR(
            "Failed to load Small_Rock.mbx"
        );
    }

    // ============================================
    // NEW ROCK LIBRARY
    // ============================================

    // ============================================
    // INITIALIZE ROCK LIBRARY
    // ============================================

    if (m_particleSystemRender.LoadRockLibrary("assets/rocks"))
    {
        BOX_LOG_INFO(
            "Rock Library contains "
            << m_particleSystemRender.GetRockLibraryCount()
            << " models"
        );

        // Create OpenGL buffers for all loaded rocks.
        if (!m_particleSystemRender.CreateRockLibraryBuffers())
        {
            BOX_LOG_ERROR(
                "Failed to initialize Rock Library GPU buffers"
            );
        }
    }
    else
    {
        BOX_LOG_ERROR(
            "Failed to load Rock Library"
        );
    }


    /*if (!m_particleSystemRender.LoadRockLibrary("assets/rocks"))
    {
        BOX_LOG_ERROR("Failed to initialize Rock Library");
    }
    else
    {
        BOX_LOG_INFO(
            "Rock Library contains " << m_particleSystemRender.GetRockLibraryCount() << " models");
    }*/

    // ============================================
    // END PARTICLE SYSTEM - SMALL ROCK
    // ============================================


	// Create the terrain Brush for visualizing the area of effect when editing the terrain
    const std::string terrainBrushVertexPath =
        helpers.GetAssetPath(
            "assets/shader/TerrainBrush.vert"
        );

    const std::string terrainBrushFragmentPath =
        helpers.GetAssetPath(
            "assets/shader/TerrainBrush.frag"
        );

    m_terrainBrushShader =
        std::make_unique<Shader>(
            terrainBrushVertexPath,
            terrainBrushFragmentPath
        );

    if (!m_terrainBrushShader ||
        m_terrainBrushShader->ID() == 0)
    {
        BOX_LOG_ERROR(
            "Failed to create terrain brush shader"
        );

        return false;
    }

    m_terrainBrush =
        std::make_unique<TerrainBrush>();

    if (!m_terrainBrush->Initialize())
    {
        BOX_LOG_ERROR(
            "Failed to initialize terrain brush"
        );

        return false;
    }

	// ---------------------------------------- Water shader ----------------------------------------
    const std::string waterVertexPath =
        helpers.GetAssetPath(
            "assets/shader/Water.vert"
        );

    const std::string waterFragmentPath =
        helpers.GetAssetPath(
            "assets/shader/Water.frag"
        );

    m_waterShader =
        std::make_unique<Shader>(
            waterVertexPath,
            waterFragmentPath
        );

    if (!m_waterShader ||
        m_waterShader->ID() == 0)
    {
        BOX_LOG_ERROR(
            "Failed to create water shader"
        );

        return false;
    }

    m_water = std::make_unique<Water>();

    if (!m_water->Initialize(300.0f))
    {
        BOX_LOG_ERROR("Failed to initialize water");

        m_water.reset();

        return false;
    }

    m_water->SetHeight(0.0f);

   
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

    if (m_terrain)
    {
        m_terrain->Destroy();
        m_terrain.reset();
    }

    m_terrainShader.reset();

    if (m_water)
    {
        m_water->Destroy();
        m_water.reset();
    }

    m_waterShader.reset();

    // ============================================
    // DESTROY BOX RENDERER
    // ============================================

    if (m_boxRenderer)
    {
        m_boxRenderer->Destroy();
        m_boxRenderer.reset();
    }

    m_showFinalRender = false;

    m_previewGrassBlade.reset();
    //m_cachedGrassMatrices.clear();
    m_particleSystemRender.ClearGrassCache();
    m_testGrassBlade.reset();
    m_brokenGrassBlade.reset();
    m_previewGrassBlade.reset();
    m_previewBrokenBlade.reset();

    m_particleSystemRender.ClearRockCache();
    

    m_grassBladeShader.reset();

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
// ------------------------------------------------- Grid functions -------------------------------------------------
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

void BoxEngine::SetGridVisible(bool visible)
{
	if (m_grid)
	{
		m_grid->SetVisible(visible);
	}
}
bool BoxEngine::IsGridVisible() const
{
	return m_grid && m_grid->IsVisible();
}

// ------------------------------------------------- End grid functions -------------------------------------------------

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
    int subdivisions, float roughness, std::uint32_t seed, float flattening, float elongation, float angularity,
    int flatFaceCount, float cutDepth, bool randomFaceDirection)
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
        flattening,
        elongation,
        angularity,
        flatFaceCount,
        cutDepth,
        randomFaceDirection))
    {
        BOX_LOG_ERROR(
            "Failed to add editable rock"
        );

        return false;
    }

    // NEW: Remember that this is a rock.
    rock->SetPrimitiveType(EntityPrimitiveType::Rock);

    m_entities.push_back(std::move(rock));

    m_selectedEntityID = entityID;

    return true;
}

// -------------------------- My Create a terrain primitive with subdivisions, width and depth --------------------------
bool BoxEngine::AddEditableTerrain(const glm::vec3& position, int width, int depth, int subdivisionsX, int subdivisionsZ,
    float heightScale, std::uint32_t seed)
{
    return false;
}

// =========================================================================================================================
// =========================== ============ SELECT PLANT PREVIEW MODE ======================================================
// =========================================================================================================================

void BoxEngine::SetPlantPreviewMode(PlantPreviewMode mode)
{
    m_plantPreview.SetMode(mode);

    // Automatically show the preview
    // when a new mode is selected.
    m_plantPreview.Show();
}

// ===============================================================
// Grass clump creation
// ===============================================================
Entity* BoxEngine::AddEditableGrassClump(const MeshEditing& grassMesh, const glm::vec3& position)
{
    if (grassMesh.GetVertexCount() == 0 ||
        grassMesh.GetFaceCount() == 0)
    {
        BOX_LOG_ERROR(
            "AddEditableGrassClump: empty mesh"
        );

        return nullptr;
    }

    const int entityID =
        m_nextEntityID++;

    const std::string entityName =
        "Grass Clump " +
        std::to_string(entityID);

    auto grass =
        std::make_unique<Entity>(
            entityID,
            entityName
        );

    // Default material for now.
    grass->GetMaterial().SetBaseColorTexture(
        m_defaultTexture.GetID(),
        m_defaultTexturePath
    );

    Material& material =
        grass->GetMaterial();

    material.SetUseBaseColorTexture(true);

    grass->SetPosition(position);

    grass->GetEditableMesh() =
        grassMesh;

    grass->GetBaseEditableMesh() =
        grassMesh;

    if (!grass->RebuildFromEditableMesh())
    {
        BOX_LOG_ERROR(
            "Failed to build grass clump entity"
        );

        return nullptr;
    }

    Entity* result =
        grass.get();

    m_entities.push_back(
        std::move(grass)
    );

    m_selectedEntityID =
        entityID;

    BOX_LOG_INFO(
        "Added grass clump. Entity count: "
        << m_entities.size()
    );

    return result;
}

// =====================================================
// NEW CREATE PLANT PREVIEW
// =====================================================
// =====================================================
// TOGGLE PLANT PREVIEW
// =====================================================

bool BoxEngine::TogglePlantPreview()
{
    if (!m_previewGrassBlade)
        return false;

    m_plantPreview.Toggle();

    return true;
}

// =====================================================
// APPLY PLANT PREVIEW SETTINGS
// =====================================================

bool BoxEngine::ApplyPlantPreviewSettings()
{
    if (!m_previewGrassBlade || !m_previewBrokenBlade)
        return false;

    PlantPreviewSettings& settings =
        m_plantPreview.GetSettings();

    // Keep geometry measurements sensible.
    settings.bladeWidth =
        glm::max(settings.bladeWidth, 0.0001f);

    settings.bladeHeight =
        glm::max(settings.bladeHeight, 0.001f);

    settings.concavity =
        glm::max(settings.concavity, 0.0f);

    settings.breakPosition =
        glm::clamp(settings.breakPosition, 0.05f, 0.95f);

    settings.breakAngle =
        glm::clamp(settings.breakAngle, 0.0f, 180.0f);

    settings.previewScale =
        glm::max(settings.previewScale, 0.01f);

    // -------------------------------------------------
    // Normal grass blade
    // -------------------------------------------------

    m_previewGrassBlade->SetSize(
        settings.bladeWidth,
        settings.bladeHeight
    );

    m_previewGrassBlade->SetCurvature(
        settings.curvature
    );

    m_previewGrassBlade->SetConcavity(
        settings.concavity
    );

    if (!m_previewGrassBlade->CreateGrassBlade())
    {
        BOX_LOG_ERROR(
            "Failed to rebuild normal grass preview"
        );

        return false;
    }

    // -------------------------------------------------
    // Broken grass blade
    // -------------------------------------------------

    m_previewBrokenBlade->SetSize(
        settings.bladeWidth,
        settings.bladeHeight
    );

    m_previewBrokenBlade->SetCurvature(
        settings.curvature
    );

    m_previewBrokenBlade->SetConcavity(
        settings.concavity
    );

    m_previewBrokenBlade->SetBreakPosition(
        settings.breakPosition
    );

    m_previewBrokenBlade->SetBreakAngle(
        settings.breakAngle
    );

    if (!m_previewBrokenBlade->CreateBrokenGrassBlade())
    {
        BOX_LOG_ERROR(
            "Failed to rebuild broken grass preview"
        );

        return false;
    }

    // -------------------------------------------------
    // Preview size
    // -------------------------------------------------

    m_plantPreview.ApplyPreviewScale();

    return true;
}
// ================================================ END NEW CREATE PLANT PREVIEW ================================================
// end of new grass blade function for testing



// Load a skybox from a folder containing the six images for the skybox faces
bool BoxEngine::LoadSkyBox(
    const std::string& folderPath)
{
    if (!m_sky)
    {
        return false;
    }

    return m_sky->BeginLoadSkyFolder(
        folderPath
    );
}


Sky& BoxEngine::GetSky()
{
    return *m_sky;
}

const Sky& BoxEngine::GetSky() const
{
    return *m_sky;
}
// ------------------------------------ Terrain ------------------------------------
bool BoxEngine::CreateTerrain()
{
    if (!m_terrain)
    {
        m_terrain = std::make_unique<Terrain>();
        m_terrain->SetPosition(glm::vec3(0.0f, 15.5f, 0.0f));
    }

    if (!m_terrain->Initialize())
    {
        BOX_LOG_ERROR("Failed to initialize terrain" );

        m_terrain.reset();

        return false;
    }

    BOX_LOG_INFO("Terrain created");

    return true;
}

// Update the terrain brush position and radius for visualizing the area of effect when editing the terrain
void BoxEngine::UpdateTerrainBrush(
    const glm::vec3& position,
    float radius)
{
    if (!m_terrain ||
        !m_terrainBrush)
    {
        return;
    }

    m_terrainBrush->Update(
        *m_terrain,
        position,
        radius
    );
}
// Raise the terrain at the specified world position with the given radius and strength
void BoxEngine::RaiseTerrain(const glm::vec3& worldPosition, float radius, float strength)
{
    if (!m_terrain)
    {
        return;
    }

    m_terrain->RaiseTerrain(worldPosition, radius, strength);
    m_terrain->UpdateMeshFromHeightMap();
}
// Lower the terrain at the specified world position with the given radius and strength
void BoxEngine::LowerTerrain(const glm::vec3& worldPosition, float radius, float strength)
{
	if (!m_terrain)
	{
		return;
	}
	m_terrain->LowerTerrain(worldPosition, radius, strength);
	m_terrain->UpdateMeshFromHeightMap();
}

void BoxEngine::SmoothTerrain(const glm::vec3& worldPosition, float radius, float strength)
{
	if (!m_terrain)
	{
		return;
	}
	m_terrain->SmoothTerrain(worldPosition, radius, strength);
	m_terrain->UpdateMeshFromHeightMap();
}

void BoxEngine::FlattenTerrain(const glm::vec3& worldPosition, float radius, float strength, float targetHeight)
{
    if (!m_terrain)
    {
        return;
    }

    m_terrain->FlattenTerrain(
        worldPosition,
        radius,
        strength,
        targetHeight
    );

    m_terrain->UpdateMeshFromHeightMap();
}

void BoxEngine::NoiseTerrain(const glm::vec3& worldPosition, float radius, float strength)
{
	if (!m_terrain)
	{
		return;
	}
	m_terrain->NoiseTerrain(worldPosition, radius, strength);
	m_terrain->UpdateMeshFromHeightMap();
}

Terrain& BoxEngine::GetTerrain()
{
    return *m_terrain;
}

const Terrain& BoxEngine::GetTerrain() const
{
    return *m_terrain;
}

// Water height adjustment
void BoxEngine::SetWaterHeight(float height)
{
    m_waterHeight = height;

    if (m_water)
    {
        m_water->SetHeight(height);
    }
}

void BoxEngine::SetWaterEnabled(bool enabled)
{
    m_waterEnabled = enabled;
}

void BoxEngine::SetWaveSpeed(float speed)
{
	m_waveSpeed = speed;
}

void BoxEngine::SetWaveStrength(float strength)
{
	m_waveStrength = strength;
}

void BoxEngine::SetWaterSpecularPower(float power)
{
    m_waterSpecularPower = power;
}

void BoxEngine::UpdateWater(float deltaTime)
{
    if (!m_waterEnabled ||
        !m_water)
    {
        return;
    }

    m_water->Update(deltaTime);
}
// End Water

void BoxEngine::HideTerrainBrush()
{
    if (m_terrainBrush)
    {
        m_terrainBrush->Hide();
    }
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

bool BoxEngine::AddEditableCone(const glm::vec3& position, int sectors, float radius, float height, int stacks)
{
    const int entityID = m_nextEntityID++;

    const std::string entityName = "Cone " + std::to_string(entityID);

    auto cone = std::make_unique<Entity>(entityID, entityName);

    cone->GetMaterial().SetBaseColorTexture(m_defaultTexture.GetID(), m_defaultTexturePath);

    Material& material = cone->GetMaterial();

    material.SetUseBaseColorTexture(true);

    cone->SetPosition(position);

    if (!cone->CreateCone(sectors, radius, height, stacks))
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
    // ============================================
    // UPDATE BOX RENDERER
    // ============================================

    if (m_boxRenderer &&
        m_boxRenderer->IsRendering())
    {
        m_boxRenderer->Update();
    }


    // ============================================
    // UPDATE SKY LIBRARY
    // ============================================

    if (m_sky)
    {
        // Load one sky image each frame.
        if (m_sky->IsLoading())
        {
            m_sky->LoadNextSky();
        }

        // ========================================
        // RESTORE SAVED SKY SELECTION
        // ========================================

        if (m_restoreSceneSky)
        {
            const auto& skies =
                m_sky->GetSkyTextures();

            bool found = false;

            for (std::size_t i = 0;
                i < skies.size(); ++i)
            {
                const std::filesystem::path loadedPath =
                    skies[i].path;

                if (loadedPath.filename() ==
                    m_pendingSceneSkyPath.filename())
                {
                    m_sky->SetSkyTexture(i);

                    BOX_LOG_INFO(
                        "Restored scene sky: "
                        << loadedPath.string()
                    );

                    m_restoreSceneSky = false;
                    m_pendingSceneSkyPath.clear();

                    found = true;
                    break;
                }
            }

            // Avoid selecting another sky when the
            // requested image could not be found.
            if (!found)
            {
                m_sky->ClearSky();
            }

            // All loading has finished, but the
            // requested sky was not found.
            if (!found && !m_sky->IsLoading())
            {
                BOX_LOG_ERROR(
                    "Scene sky not found in loaded library: "
                    << m_pendingSceneSkyPath.string()
                );

                m_restoreSceneSky = false;
                m_pendingSceneSkyPath.clear();
            }
        }
    }


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

       

		// Skybox rendering
        if (m_sky && m_skyShader)
        {
            m_sky->RenderSkyBox(*m_skyShader, view, projection);
        }
		// End Skybox rendering

		// Terrain rendering
        if (m_terrain && m_terrainShader)
        {
            m_terrain->RenderTerrain(*m_terrainShader, view, projection);
        }

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

        // Ordinary scene objects do not use instancing.
        m_sceneShader->Use();
        m_sceneShader->SetUniformInt(
            "uUseInstancing", 0
        );

        for (const auto& entity : m_entities)
        {
            if (entity)
            {
                entity->RenderScene(*m_sceneShader, *m_camera, aspectRatio);
            }
        }
        // ----------------------------------------------------------------------------------------------------
        // ---------------------------------- End Terrain rendering -------------------------------------------
		// ----------------------------------------------------------------------------------------------------


        // =======================================================================================================
		// ================================= PARTICLE SCATTER ====================================================
		// =======================================================================================================
        
        // =====================================================
        // PARTICLE SYSTEM - ROCKS
        // =====================================================

        if (m_sceneShader && m_sceneShader->ID() != 0)
        {
            m_particleSystemRender.RenderRocks(
                m_entities,
                *m_sceneShader,
                view,
                projection,
                m_camera->Position,
                m_smallRockAlbedoTexture);
        }
         
        
        // =====================================================
        // PARTICLE SYSTEM - GRASS
        // =====================================================

        if (m_testGrassBlade &&
            m_brokenGrassBlade &&
            m_grassBladeShader &&
            m_grassBladeShader->ID() != 0)

        
        {
            m_particleSystemRender.RenderGrass(
                m_entities,
                *m_testGrassBlade,
                *m_brokenGrassBlade,
                m_testGrassClump,
                *m_grassBladeShader,
                view,
                projection
            );

        }

        

        // ===================================================================================================================
		// ======================================= END PARTICLE SCATTER ======================================================
		// ===================================================================================================================

        // =====================================================
        // PLANT PREVIEW RENDERING
        // =====================================================

        if (m_grassBladeShader &&
            m_grassBladeShader->ID() != 0)
        {
            switch (m_plantPreview.GetMode())
            {
                // =============================================
                // NORMAL GRASS BLADE
                // =============================================

            case PlantPreviewMode::NormalBlade:
            {
                if (m_previewGrassBlade)
                {
                    m_plantPreview.RenderSingleBlade(
                        *m_previewGrassBlade,
                        *m_grassBladeShader,
                        view,
                        projection
                    );
                }

                break;
            }

            // =============================================
            // BROKEN GRASS BLADE
            // =============================================

            case PlantPreviewMode::BrokenBlade:
            {
                if (m_previewBrokenBlade)
                {
                    m_plantPreview.RenderSingleBlade(
                        *m_previewBrokenBlade,
                        *m_grassBladeShader,
                        view,
                        projection
                    );
                }

                break;
            }

            // =============================================
            // GRASS CLUMP
            // =============================================

            case PlantPreviewMode::GrassClump:
            {
                if (m_previewGrassBlade)
                {
                    m_plantPreview.Render(
                        *m_previewGrassBlade,
                        m_testGrassClump,
                        *m_grassBladeShader,
                        view,
                        projection
                    );
                }

                break;
            }
            }
        }

        // ============================================================================================================
        // ==============================================  END PLANT PREVIEW RENDERING
        // =============================================================================================================


        // =====================================================
        // PARTICLE SYSTEM - ROCKS moved up
        // =====================================================

        /*if (m_sceneShader && m_sceneShader->ID() != 0)
        {
            m_particleSystemRender.RenderRocks(
                m_entities,
                *m_sceneShader,
                view,
                projection,
                m_camera->Position,
                m_smallRockAlbedoTexture);
        }*/

        // ===========================================================================================
        // ============================= END PARTICLE SYSTEM - ROCKS =================================
        // ===========================================================================================
       
        // -------------------------------------------------
        // Save scene depth before drawing transparent water.
        // -------------------------------------------------
        m_sceneFramebuffer.CopyDepthTexture();
		
        
        // -------------------------------------------------
        // Water
        // -------------------------------------------------

        if (m_waterEnabled &&
            m_water &&
            m_waterShader)
        {
            m_waterShader->Use();

            m_waterShader->SetUniformFloat(
                "uTime",
                m_water->GetTime()
            );

            m_waterShader->SetUniformFloat(
                "uWaveSpeed",
                m_waveSpeed
            );

            m_waterShader->SetUniformFloat(
                "uWaveStrength",
                m_waveStrength
            );


            const glm::vec3 waterLightDirection =
                glm::normalize(
                    glm::vec3(
                        -1.0f,
                        -1.0f,
                        -0.5f
                    )
                );


            m_waterShader->setVec3(
                "uLightDirection",
                waterLightDirection
            );

            m_waterShader->setVec3(
                "uCameraPos",
                m_camera->Position
            );

            m_waterShader->SetUniformFloat(
                "uSpecularPower",
                m_waterSpecularPower
            );

            m_waterShader->SetUniformFloat(
                "uNearPlane",
                m_camera->NearPlane
            );

            m_waterShader->SetUniformFloat(
                "uFarPlane",
                m_camera->FarPlane
            );

			// water depth texture is bound to texture unit 1
            // -------------------------------------------------
            // Scene depth texture
            // -------------------------------------------------

            glActiveTexture(
                GL_TEXTURE1
            );

            glBindTexture(
                GL_TEXTURE_2D,
                m_sceneFramebuffer.GetDepthCopyTexture()
            );

            m_waterShader->SetUniformInt(
                "uSceneDepth",
                1
            );
			// ------------------------------------------------

            m_water->Render(
                *m_waterShader,
                view,
                projection
            );

            glActiveTexture(GL_TEXTURE0);
        }

        // -------------------------------------------------
        // End Water
        // -------------------------------------------------

        // terrain brush rendering for visualizing the area of effect when editing the terrain
        if (m_terrainBrush && m_terrainBrushShader)
        {
            m_terrainBrush->Render(*m_terrainBrushShader, view, projection);
        }
		// end terrain brush rendering

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
		
       
    }

    Framebuffer::Unbind();
}



GLuint BoxEngine::GetSceneTexture() const
{
    // Show BoxRenderer image when requested.
    if (m_showFinalRender && m_boxRenderer)
    {
        const GLuint renderTexture =
            m_boxRenderer->GetRenderTexture();

        if (renderTexture != 0)
        {
            return renderTexture;
        }
    }

    // Otherwise show normal editor viewport.
    return m_sceneFramebuffer.GetColorTexture();
}


//GLuint BoxEngine::GetSceneTexture() const
//{
//    return m_sceneFramebuffer.GetColorTexture();
//}

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

bool BoxEngine::RaycastTerrain(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, glm::vec3& outHitPosition)
{
	if (!m_terrain)
	{
		return false;
	}

    return m_terrain->Raycast(rayOrigin, rayDirection, outHitPosition);
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


bool BoxEngine::SaveScene(
    const std::filesystem::path& filePath)
{
    SceneSerializer serializer;

    // ============================================
    // COLLECT SKYBOX INFORMATION
    // ============================================

    SceneSkyData skyData;

    if (m_sky && m_sky->HasActiveSky())
    {
        const auto& skyTextures =
            m_sky->GetSkyTextures();

        const std::size_t selectedIndex =
            m_sky->GetSelectedSkyIndex();

        if (selectedIndex < skyTextures.size())
        {
            skyData.enabled = true;

            skyData.texturePath =
                skyTextures[selectedIndex].path;
        }
    }

    // ============================================
    // SAVE THE SCENE
    // ============================================

    return serializer.SerializeScene(
        filePath,
        m_entities,
        skyData
    );
}

bool BoxEngine::LoadScene(const std::filesystem::path& filePath)
{

    SceneSerializer serializer;

    std::vector<std::unique_ptr<Entity>>
        loadedEntities;

    SceneSkyData skyData;

    // ============================================
    // LOAD SCENE FILE
    // ============================================

    if (!serializer.DeserializeScene(
        filePath,
        loadedEntities,
        skyData))
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

    // -----------------------------------------
    // Scene loaded successfully.
    // Replace the current scene.
    // -----------------------------------------
    ClearSelectedEntity();

    m_entities =
        std::move(loadedEntities);

    m_nextEntityID = nextEntityID;


    // ============================================
    // RESTORE SKYBOX FROM SCENE
    // ============================================

    m_restoreSceneSky = false;
    m_pendingSceneSkyPath.clear();

    if (m_sky)
    {
        // The newly loaded scene replaces the old sky.
        m_sky->ClearSky();

        if (skyData.enabled && !skyData.texturePath.empty())
        {
            namespace fs = std::filesystem;

            fs::path skyPath(skyData.texturePath);

            // Support older absolute paths if the
            // BoxEditor project has been moved.
            if (!fs::exists(skyPath))
            {
                skyPath =
                    fs::path("assets/textures/skybox/NewSky") /
                    skyPath.filename();
            }

            if (!fs::exists(skyPath))
            {
                BOX_LOG_ERROR(
                    "Saved sky image not found: "
                    << skyData.texturePath
                );
            }
            else
            {
                m_pendingSceneSkyPath = skyPath;
                m_restoreSceneSky = true;

                // Load the library only when needed.
                // If already loaded, we'll search it.
                if (!m_sky->HasLoadedSkies() &&
                    !m_sky->IsLoading())
                {
                    if (!m_sky->BeginLoadSkyFolder(
                        skyPath.parent_path().string()))
                    {
                        BOX_LOG_ERROR(
                            "Failed to start sky library: "
                            << skyPath.parent_path().string()
                        );

                        m_restoreSceneSky = false;
                        m_pendingSceneSkyPath.clear();
                    }
                }
            }
        }
    }



    BOX_LOG_INFO(
        "Scene loaded successfully. Entities="
        << m_entities.size()
    );

    return true;
}

// =====================================================================================================
// ========================================== New Scene =================================================
// =====================================================================================================

void BoxEngine::NewScene()
{
    BOX_LOG_INFO("Creating new empty scene");

	CancelFinalRender(); // Stop any ongoing final render before clearing the scene.

    // ============================================
    // 1. CLEAR SELECTION
    // ============================================

    ClearSelectedEntity();
    ClearSelectedEntities();

    // ============================================
    // 2. CLEAR PARTICLE SCATTER CACHES
    // ============================================

    m_particleSystemRender.ClearGrassCache();
    m_particleSystemRender.ClearRockCache();

    m_cachedGrassMatrices.clear();

    // ============================================
    // 3. REMOVE SCENE OBJECTS
    // ============================================

    // Entity destructors release their GPU buffers.
    m_entities.clear();

    // Start entity numbering from zero again.
    m_nextEntityID = 0;

    // ============================================
    // 4. CLEAR SKYBOX
    // ============================================

    m_restoreSceneSky = false;
    m_pendingSceneSkyPath.clear();

    if (m_sky)
    {
        m_sky->CancelPendingLoad();
        m_sky->ClearSky();
    }

    // ============================================
    // 5. RESET WATER
    // ============================================

    SetWaterEnabled(false);

    // ============================================
    // 6. CLEAR TERRAIN BRUSH
    // ============================================

    HideTerrainBrush();

    // ============================================
    // FINISHED
    // ============================================

    BOX_LOG_INFO("New scene created successfully");
}


// =============================================================================================================
// =========================================== START FINAL RENDER ==============================================
// =============================================================================================================

bool BoxEngine::StartFinalRender()
{
    const int width =
        m_sceneFramebuffer.GetWidth();

    const int height =
        m_sceneFramebuffer.GetHeight();

    if (width <= 0 || height <= 0)
    {
        BOX_LOG_ERROR("Invalid render dimensions");
        return false;
    }

    if (!m_boxRenderer)
    {
        m_boxRenderer = std::make_unique<Render>();
    }

    // Stop any previous render before resizing.
    m_boxRenderer->CancelRender();

    if (!m_boxRenderer->Initialize(width, height))
    {
        BOX_LOG_ERROR("Failed to initialize BoxRenderer");
        m_showFinalRender = false;
        return false;
    }


    // ============================================
    // CREATE SCENE SNAPSHOT
    // ============================================

    ::RenderScene renderScene;

    // Match the existing editor camera.
    const float aspect = static_cast<float>(width) / static_cast<float>(height);

    renderScene.view = m_camera->GetViewMatrix();

    renderScene.projection = m_camera->GetProjectionMatrix(aspect);

    // ============================================
    // COPY DIRECTIONAL LIGHT
    // ============================================

    renderScene.lightDirection = glm::normalize(m_renderLightDirection);
   
    renderScene.lightIntensity = m_renderLightIntensity;


    // ============================================
    // RENDER TEXTURE SNAPSHOT CACHE
    // ============================================

    // Maps an OpenGL texture ID to its index
    // inside renderScene.textures.
    std::unordered_map<GLuint, int> textureLookup;

    // Copies a loaded OpenGL texture into CPU memory.
    // This runs only when the render is started.
    auto captureAlbedoTexture =
        [&](GLuint textureID) -> int
    {
        if (textureID == 0)
            return -1;

        // Already captured this texture.
        auto found = textureLookup.find(textureID);

        if (found != textureLookup.end())
            return found->second;

        GLint previousTexture = 0;
        GLint previousPackAlignment = 4;
        GLint previousPackBuffer = 0;

        glGetIntegerv(
            GL_TEXTURE_BINDING_2D,
            &previousTexture
        );

        glGetIntegerv(
            GL_PACK_ALIGNMENT,
            &previousPackAlignment
        );

        glGetIntegerv(
            GL_PIXEL_PACK_BUFFER_BINDING,
            &previousPackBuffer
        );

        glBindTexture(GL_TEXTURE_2D, textureID);

        RenderTexture texture;

        glGetTexLevelParameteriv(
            GL_TEXTURE_2D, 0,
            GL_TEXTURE_WIDTH,
            &texture.width
        );

        glGetTexLevelParameteriv(
            GL_TEXTURE_2D, 0,
            GL_TEXTURE_HEIGHT,
            &texture.height
        );

        if (texture.width > 0 && texture.height > 0)
        {
            texture.pixels.resize(
                static_cast<std::size_t>(texture.width) *
                static_cast<std::size_t>(texture.height) * 4
            );

            // Read into ordinary CPU memory.
            glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
            glPixelStorei(GL_PACK_ALIGNMENT, 1);

            glGetTexImage(
                GL_TEXTURE_2D,
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                texture.pixels.data()
            );
        }

        // Restore the OpenGL state we changed.
        glPixelStorei(
            GL_PACK_ALIGNMENT,
            previousPackAlignment
        );

        glBindBuffer(
            GL_PIXEL_PACK_BUFFER,
            static_cast<GLuint>(previousPackBuffer)
        );

        glBindTexture(
            GL_TEXTURE_2D,
            static_cast<GLuint>(previousTexture)
        );

        if (!texture.IsValid())
            return -1;

        const int index =
            static_cast<int>(renderScene.textures.size());

        renderScene.textures.push_back(
            std::move(texture)
        );

        textureLookup[textureID] = index;

        return index;
    };




    // ============================================
    // COPY EDITABLE MESH GEOMETRY
    // ============================================

    for (const auto& entity : m_entities)
    {
        if (!entity || !entity->IsVisible())
            continue;

        const MeshEditing& mesh =
            entity->GetEditableMesh();

        const auto& vertices =
            mesh.GetVertices();

        const auto& faces =
            mesh.GetFaces();

        // ============================================
        // DEBUG MATERIAL TEXTURES
        // ============================================

        //BOX_LOG_INFO(
        //    "Entity: " << entity->GetName()
        //    << " Material slots: "
        //    << entity->GetMaterialSlotCount()
        //);

        //// Check the entity's primary material.
        //const Material& mainMaterial = entity->GetMaterial();

        //BOX_LOG_INFO(
        //    "Primary Material: "
        //    << mainMaterial.GetName()
        //    << " Texture ID: "
        //    << mainMaterial.GetBaseColorTexture()
        //    << " Uses Texture: "
        //    << mainMaterial.UsesBaseColorTexture()
        //);

        //// Check each material slot.
        //for (std::size_t slot = 0;
        //    slot < entity->GetMaterialSlotCount();
        //    ++slot)
        //{
        //    const Material& material =
        //        entity->GetMaterialSlot(slot);

        //    BOX_LOG_INFO(
        //        "Material Slot " << slot
        //        << " Name: " << material.GetName()
        //        << " Texture ID: "
        //        << material.GetBaseColorTexture()
        //        << " Uses Texture: "
        //        << material.UsesBaseColorTexture()
        //    );
        //}

        //// Check which slot the first face uses.
        //if (!faces.empty())
        //{
        //    BOX_LOG_INFO(
        //        "First face material index: "
        //        << faces[0].materialIndex
        //        << " UV count: "
        //        << faces[0].uvs.size()
        //    );
        //}



        // ============================================= END TEST =====================================================

        // Transform local mesh coordinates to world.
        const glm::mat4 model =
            entity->GetModelMatrix();

        // Every face can have multiple vertices.
        for (const EditFace& face : faces)
        {
            if (face.vertices.size() < 3)
                continue;


            // ============================================
            // GET FACE MATERIAL COLOUR
            // ============================================

            // Default colour if the face has no valid
            // material assigned.
            glm::vec3 faceColor(0.75f, 0.72f, 0.60f);

            const std::size_t materialIndex =
                face.materialIndex;

            if (materialIndex < entity->GetMaterialSlotCount())
            {
                const Material& material =
                    entity->GetMaterialSlot(materialIndex);

                // BoxEditor stores base colours as RGBA.
                // For now we only need RGB.
                const glm::vec4 baseColor =
                    material.GetBaseColor();

                faceColor = glm::vec3(baseColor);
            }
            // ================================================================

            // ============================================
            // FIND FACE ALBEDO TEXTURE
            // ============================================

            int faceTextureIndex = -1;

            if (face.materialIndex <
                entity->GetMaterialSlotCount())
            {
                const Material& material =
                    entity->GetMaterialSlot(face.materialIndex);

                if (material.UsesBaseColorTexture())
                {
                    faceTextureIndex = captureAlbedoTexture(
                        material.GetBaseColorTexture()
                    );
                }
            }

            // Only use texture mapping if this face
            // has a complete set of UV coordinates.
            const bool hasValidUVs = face.uvs.size() == face.vertices.size();


            // ============================================
            // AUTOMATIC FACE UV BOUNDS
            // ============================================

            glm::vec3 faceMin(
                std::numeric_limits<float>::max()
            );

            glm::vec3 faceMax(
                std::numeric_limits<float>::lowest()
            );

            for (std::size_t vertexIndex : face.vertices)
            {
                if (vertexIndex >= vertices.size())
                    continue;

                const glm::vec3& position =
                    vertices[vertexIndex].position;

                faceMin = glm::min(faceMin, position);
                faceMax = glm::max(faceMax, position);
            }

            const glm::vec3 faceSize = faceMax - faceMin;



			// =================================================================                                                                                 

            // Simple triangle fan for our first test.
            // This is suitable for convex faces.
            for (std::size_t i = 1;
                i + 1 < face.vertices.size();
                ++i)
            {
                const std::size_t ia = face.vertices[0];
                const std::size_t ib = face.vertices[i];
                const std::size_t ic = face.vertices[i + 1];

                if (ia >= vertices.size() ||
                    ib >= vertices.size() ||
                    ic >= vertices.size())
                {
                    continue;
                }

                RenderTriangle triangle;

                triangle.a = glm::vec3(
                    model * glm::vec4(
                        vertices[ia].position, 1.0f)
                );

                triangle.b = glm::vec3(
                    model * glm::vec4(
                        vertices[ib].position, 1.0f)
                );

                triangle.c = glm::vec3(
                    model * glm::vec4(
                        vertices[ic].position, 1.0f)
                );

                // ============================================
                // ASSIGN MATERIAL COLOUR
                // ============================================

                triangle.color = faceColor;

				// ============================================
                // ============================================
                // COPY FACE-CORNER UV COORDINATES
                // ============================================


                // ============================================
                // ASSIGN TEXTURE UV COORDINATES
                // ============================================

                if (faceTextureIndex >= 0)
                {
                    if (hasValidUVs)
                    {
                        // Use the mesh's existing UV mapping.
                        triangle.uvA = face.uvs[0];
                        triangle.uvB = face.uvs[i];
                        triangle.uvC = face.uvs[i + 1];
                    }

                    // ====================================
                    // NORMALIZED PLANAR UV MAPPING
                    // ====================================

                    // All triangles on this face use the
                    // same coordinate bounds.

                    const glm::vec3& a = vertices[ia].position;
                    const glm::vec3& b = vertices[ib].position;
                    const glm::vec3& c = vertices[ic].position;

                    const glm::vec3 normal = glm::abs(
                        glm::cross(b - a, c - a)
                    );

                    constexpr float epsilon = 0.000001f;

                    // Map a local vertex position into the
                    // face's 0-1 UV coordinate range.
                    auto projectUV =
                        [&](const glm::vec3& p) -> glm::vec2
                    {
                        if (normal.x >= normal.y &&
                            normal.x >= normal.z)
                        {
                            // X-facing face: YZ plane.
                            return glm::vec2(
                                (p.z - faceMin.z) /
                                glm::max(faceSize.z, epsilon),
                                (p.y - faceMin.y) /
                                glm::max(faceSize.y, epsilon)
                            );
                        }
                        else if (normal.y >= normal.z)
                        {
                            // Y-facing face: XZ plane.
                            return glm::vec2(
                                (p.x - faceMin.x) /
                                glm::max(faceSize.x, epsilon),
                                (p.z - faceMin.z) /
                                glm::max(faceSize.z, epsilon)
                            );
                        }
                        else
                        {
                            // Z-facing face: XY plane.
                            return glm::vec2(
                                (p.x - faceMin.x) /
                                glm::max(faceSize.x, epsilon),
                                (p.y - faceMin.y) /
                                glm::max(faceSize.y, epsilon)
                            );
                        }
                    };

                    // Assign UVs to all three corners.
                    triangle.uvA = projectUV(a);
                    triangle.uvB = projectUV(b);
                    triangle.uvC = projectUV(c);


                    // Assign the captured CPU texture.
                    triangle.textureIndex = faceTextureIndex;
                }



				// ============================================


                renderScene.triangles.push_back(triangle);
            }
        }
    }

    BOX_LOG_INFO(
        "BoxRenderer snapshot contains "
        << renderScene.triangles.size()
        << " triangles"
    );

    // Give the renderer its independent snapshot.
    // 
    //m_boxRenderer->SetScene(std::move(renderScene));

// ============================================
// REPORT TEXTURE SNAPSHOT
// ============================================

    BOX_LOG_INFO(
        "BoxRenderer captured "
        << renderScene.textures.size()
        << " unique albedo textures"
    );

    // ============================================
    // PASS SCENE TO RENDERER
    // ============================================

    m_boxRenderer->SetScene(std::move(renderScene));

    
    // ################################

    BOX_LOG_INFO("BoxRenderer BVH nodes: " << m_boxRenderer->GetBVHNodeCount());


    m_boxRenderer->StartRender();

    m_showFinalRender = true;

    BOX_LOG_INFO(
        "BoxRenderer started: "
        << width << " x " << height
    );

    return true;
}

// ============================================
// CANCEL FINAL RENDER
// ============================================

void BoxEngine::CancelFinalRender()
{
    if (m_boxRenderer)
    {
        m_boxRenderer->CancelRender();
    }

    // Display normal OpenGL viewport again.
    m_showFinalRender = false;

    BOX_LOG_INFO("BoxRenderer cancelled");
}

// ============================================
// IS RENDERING?
// ============================================

bool BoxEngine::IsFinalRendering() const
{
    return m_boxRenderer &&
        m_boxRenderer->IsRendering();
}

// ============================================
// RENDER PROGRESS BAR
// ============================================

float BoxEngine::GetFinalRenderProgress() const
{
    if (!m_boxRenderer)
        return 0.0f;

    return m_boxRenderer->GetProgress();
}


// ============================================
// GET FINAL RENDER TIME
// ============================================

double BoxEngine::GetFinalRenderTime() const
{
    if (!m_boxRenderer)
        return 0.0;

    return m_boxRenderer->GetRenderTime();
}


// ============================================
// SET RENDER LIGHT DIRECTION
// ============================================

void BoxEngine::SetRenderLightDirection(
    const glm::vec3& direction)
{
    // A zero-length direction is invalid.
    // Ignore it until the user enters a
    // valid direction again.

    if (glm::dot(direction, direction) < 0.000001f)
        return;

    m_renderLightDirection = direction;
}

// ============================================
// GET RENDER LIGHT DIRECTION
// ============================================

glm::vec3 BoxEngine::GetRenderLightDirection() const
{
    return m_renderLightDirection;
}

void BoxEngine::SetRenderLightIntensity(float intensity)
{
    m_renderLightIntensity = glm::max(0.0f, intensity);
}

float BoxEngine::GetRenderLightIntensity() const
{
    return m_renderLightIntensity;
}

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------- END RENDER -----------------------------------
// ----------------------------------------------------------------------------------------------------




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