#include "panels/EcosystemPanel.h"
#include <BoxEngine.h>
#include <imgui/imgui.h>
#include <miniBoxLog.h>
#include <rendering/Sky.h>

EcosystemPanel::~EcosystemPanel() = default;

bool EcosystemPanel::Initialize()
{
   

    BOX_LOG_INFO("EcosystemPanel initialized");

    return true;
}

void EcosystemPanel::Open()
{
    m_isOpen = true;

    BOX_LOG_INFO("EcosystemPanel::Open - m_isOpen = " << m_isOpen);
}

void EcosystemPanel::Close()
{
    m_isOpen = false;
}

bool EcosystemPanel::IsOpen() const
{
    return m_isOpen;
}

// ----------------------------------------------------------------
// My Ecosystem Panel Draw Function
// ----------------------------------------------------------------

EcoSystemAction EcosystemPanel::Draw(BoxEngine& engine)
{

    EcoSystemAction Ecoaction = EcoSystemAction::None;

    if (!m_isOpen)
    {
       return Ecoaction;
    }

    ImGui::SetNextWindowSize(
        ImVec2(700.0f, 550.0f),
        ImGuiCond_FirstUseEver
    );


    if (!ImGui::Begin(
        "Ecosystem",
        &m_isOpen,
        ImGuiWindowFlags_NoCollapse))
    {
        ImGui::End();
        return Ecoaction;
    }


    // =================================================
    // ECOSYSTEM TABS
    // =================================================

    if (ImGui::BeginTabBar("##EcosystemTabs"))
    {
        // -------------------------------------------------
        // FLOOR
        // -------------------------------------------------

        if (ImGui::BeginTabItem("Floor"))
        {
            ImGui::Text("Floor Generator");

            ImGui::Separator();
            ImGui::Spacing();


            // =============================================
            // FLOOR SIZE
            // =============================================

            ImGui::Text("Floor Size");


            ImGui::DragFloat(
                "Width",
                &m_floorWidth,
                0.5f,
                1.0f,
                500.0f,
                "%.1f"
            );


            ImGui::DragFloat(
                "Depth",
                &m_floorDepth,
                0.5f,
                1.0f,
                500.0f,
                "%.1f"
            );


            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();


            // =============================================
            // SUBDIVISIONS
            // =============================================

            ImGui::Text("Mesh Resolution");


            ImGui::DragInt(
                "Subdivisions X",
                &m_floorSubdivisionsX,
                1.0f,
                1,
                200
            );


            ImGui::DragInt(
                "Subdivisions Z",
                &m_floorSubdivisionsZ,
                1.0f,
                1,
                200
            );


            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();


            // =============================================
            // INFORMATION
            // =============================================

            const int vertexCount =
                (m_floorSubdivisionsX + 1) *
                (m_floorSubdivisionsZ + 1);


            const int faceCount =
                m_floorSubdivisionsX *
                m_floorSubdivisionsZ;


            ImGui::Text(
                "Vertices: %d",
                vertexCount
            );


            ImGui::Text(
                "Faces: %d",
                faceCount
            );


            ImGui::Spacing();


            // =============================================
            // GENERATE BUTTON
            // =============================================

            if (ImGui::Button("Generate Floor", ImVec2(140.0f, 32.0f)))
            {
                Ecoaction = EcoSystemAction::AddFloor;
            }


            ImGui::EndTabItem();
        }


        // -------------------------------------------------
        // ROCKS
        // -------------------------------------------------
        if (ImGui::BeginTabItem("Rocks"))
        {
            const EcoSystemAction rockAction =
                RocksTab(engine);

            if (rockAction != EcoSystemAction::None)
            {
                Ecoaction = rockAction;
            }

            ImGui::EndTabItem();
        }
        


        // -------------------------------------------------
        // TERRAIN
        // -------------------------------------------------

        if (ImGui::BeginTabItem("Terrain"))
        {
			const EcoSystemAction terrainAction = TerrainTab(engine);

			if (terrainAction != EcoSystemAction::None)
			{
				Ecoaction = terrainAction;
			}

            ImGui::EndTabItem();
        }


        // -------------------------------------------------
        // WATER
        // -------------------------------------------------

        if (ImGui::BeginTabItem("Water"))
        {
            WaterTab(
                engine
            );

            ImGui::EndTabItem();
        }

        
        // -------------------------------------------------
        // GRASS
        // -------------------------------------------------

        if (ImGui::BeginTabItem("Grass"))
        {
            const EcoSystemAction grassAction = GrassTab(engine);

            if (grassAction != EcoSystemAction::None)
            {
                Ecoaction = grassAction;
            }

            ImGui::EndTabItem();
        }


        // -------------------------------------------------
        // PLANTS
        // -------------------------------------------------

        if (ImGui::BeginTabItem("Plants"))
        {
            PlantsTab(
                engine
            );

            ImGui::EndTabItem();
        }


        // -------------------------------------------------
        // TREES
        // -------------------------------------------------

        if (ImGui::BeginTabItem("Trees"))
        {
            const EcoSystemAction treeAction =
                TreesTab(engine);

            if (treeAction !=
                EcoSystemAction::None)
            {
                Ecoaction =
                    treeAction;
            }

            ImGui::EndTabItem();
        }

        // -------------------------------------------------
        // SKY
        // -------------------------------------------------

        if (ImGui::BeginTabItem("Sky"))
        {
            const EcoSystemAction skyAction = SkyTab(engine);

            if (skyAction != EcoSystemAction::None)
            {
                Ecoaction = skyAction;
            }

            ImGui::EndTabItem();
        }

        // -------------------------------------------------
        // EnvironmentTab
        // -------------------------------------------------

        if (ImGui::BeginTabItem("Environment"))
        {
            EnvironmentTab(
                engine
            );

            ImGui::EndTabItem();
        }


        ImGui::EndTabBar();
    }


    ImGui::End();
    
    return Ecoaction;
}
// ---------------------------------------- End of EcosystemPanel::DrawEco ----------------------------------------

//void EcosystemPanel::FloorTab(BoxEngine& engine)
//{
//    ImGui::Text("Floor Generator");
//    ImGui::Separator();
//
//    ImGui::TextDisabled(
//        "Editable ecosystem floor coming next..."
//    );
//}

// -----------------------------------------------------
// Rocks Generator
// -----------------------------------------------------

EcoSystemAction EcosystemPanel::RocksTab(
    BoxEngine& engine)
{
    EcoSystemAction action =
        EcoSystemAction::None;

    ImGui::Text("Rock Generator");
    ImGui::Separator();
    ImGui::Spacing();


    // =================================================
    // ROCK SIZE
    // =================================================

    ImGui::Text("Rock Size");

    ImGui::DragFloat(
        "Radius",
        &m_rockRadius,
        0.05f,
        0.1f,
        5.0f,
        "%.2f",
        ImGuiSliderFlags_AlwaysClamp
    );


    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();


    // =================================================
    // ROCK SHAPE
    // =================================================

    

    ImGui::DragInt(
        "Subdivisions",
        &m_rockSubdivisions,
        1.0f,
        0,
        6
    );

	
    ImGui::DragFloat(
        "Roughness",
        &m_rockRoughness,
        0.01f,
        0.0f,
        1.0f,
        "%.2f"
    );


    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();


    // =================================================
    // RANDOM SEED
    // =================================================

    ImGui::Text("Variation");

    ImGui::InputInt(
        "Seed",
        &m_rockSeed
    );


    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

	// ------------------------------------------------
	// ROCK FLATTENING
	// ------------------------------------------------

    ImGui::DragFloat(
        "Flattening",
        &m_rockFlattening,
        0.01f,
        0.0f,
        0.75f,
        "%.2f",
        ImGuiSliderFlags_AlwaysClamp
    );

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // =================================================
    // GENERATE
    // =================================================

    if (ImGui::Button(
        "Generate Rock",
        ImVec2(140.0f, 32.0f)))
    {
        action =
            EcoSystemAction::AddRocks;
    }

    return action;
}

// ---------------------------------------------------------------------------------------------------------------
// Terrain Generator
// ---------------------------------------------------------------------------------------------------------------

EcoSystemAction EcosystemPanel::TerrainTab(BoxEngine& engine)
{
    EcoSystemAction action = EcoSystemAction::None;
    // TODO: Implement terrain generation logic here
    ImGui::Text("Terrain Generator");
    ImGui::Separator();

    ImGui::TextDisabled("Procedural terrain generation coming soon...");
    if (ImGui::Button(
        "Generate Terrain",
        ImVec2(140.0f, 32.0f)))
    {
        action = EcoSystemAction::Addterrain;
    }

    return action;
}

// -----------------------------------------------------------------------------------------------------------------
// Water plane Generator
// -----------------------------------------------------

void EcosystemPanel::WaterTab(BoxEngine& engine)
{
    // TODO: Implement water generation logic here
    ImGui::Text("Water Generator");
    ImGui::Separator();

    ImGui::TextDisabled("Procedural water generation coming soon...");
}

// ----------------------------------------------------------------------------------------------------------------------
// Plants Generator
// -----------------------------------------------------

void EcosystemPanel::PlantsTab(BoxEngine& engine)
{
    // TODO: Implement plants generation logic here
    ImGui::Text("Plants Generator");
    ImGui::Separator();

    ImGui::TextDisabled("Procedural plants generation coming soon...");
}

// ----------------------------------------------------------------------------------------------------------------------
// Grass Generator
// -----------------------------------------------------

EcoSystemAction EcosystemPanel::GrassTab(BoxEngine& engine)
{
    EcoSystemAction action = EcoSystemAction::None;
    // TODO: Implement grass generation logic here
    ImGui::Text("Grass Generator");
    ImGui::Separator();

    ImGui::TextDisabled("Procedural grass generation coming soon...");

    // =================================================
    // CLUMP SETTINGS
    // =================================================

    ImGui::Text("Clump");
       
    ImGui::InputFloat("Blade Scale", &m_grassClumpScale, 0.05f, 0.10f, "%.2f");

    if (m_grassClumpScale < 0.10f)
    {
        m_grassClumpScale = 0.10f;
    }

    if (m_grassClumpScale > 5.0f)
    {
        m_grassClumpScale = 5.0f;
    }

    ImGui::InputFloat("Clump Size", &m_grassClumpSize, 0.05f, 0.10f, "%.2f");

	// Blade Count: 5 to 10
    ImGui::InputInt("Blade Count", &m_grassClumpDensity);

    if (m_grassClumpDensity < 5)
    {
        m_grassClumpDensity = 5;
    }
    if (m_grassClumpDensity > 10)
    {
        m_grassClumpDensity = 10;
    }
	// Blade Curve: 0.0f to 0.70f
	ImGui::InputFloat("Blade Curve", &m_grassClumpCurve, 0.03f, 0.07f, "%.2f");
	if (m_grassClumpCurve < 0.0f)
	{
		m_grassClumpCurve = 0.0f;
	}
	if (m_grassClumpCurve > 0.70f)
	{
		m_grassClumpCurve = 0.70f;
	}

    ImGui::Spacing();
    ImGui::SeparatorText("Set Grass Random Direction on");
	ImGui::Checkbox("Random Direction", &m_grassRndDir);



    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();


    // =================================================
    // VARIATION
    // =================================================

    ImGui::Text("Variation");


    ImGui::InputInt("Seed", &m_grassSeed);


    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button(
        "Generate Grass",
        ImVec2(140.0f, 32.0f)))
    {
        action = EcoSystemAction::AddGrass;
    }

    return action;
}

// -----------------------------------------------------------------------------------------------------------------
// Tree Generator
// -----------------------------------------------------

EcoSystemAction EcosystemPanel::TreesTab(BoxEngine& engine)
{
    EcoSystemAction action =
        EcoSystemAction::None;


    ImGui::Text("Tree Generator");

    ImGui::Separator();
    ImGui::Spacing();


    // =================================================
    // TRUNK
    // =================================================

    ImGui::Text("Trunk");


    ImGui::DragFloat(
        "Trunk Height",
        &m_treeTrunkHeight,
        0.05f,
        0.5f,
        10.0f,
        "%.2f",
        ImGuiSliderFlags_AlwaysClamp
    );


    ImGui::DragFloat(
        "Trunk Radius",
        &m_treeTrunkRadius,
        0.01f,
        0.05f,
        2.0f,
        "%.2f",
        ImGuiSliderFlags_AlwaysClamp
    );


    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();


    // =================================================
    // BRANCHES
    // =================================================

    ImGui::Text("Branches");


    ImGui::DragInt(
        "Branch Count",
        &m_treeBranchCount,
        1.0f,
        0,
        12
    );


    ImGui::DragFloat(
        "Branch Length",
        &m_treeBranchLength,
        0.05f,
        0.2f,
        5.0f,
        "%.2f",
        ImGuiSliderFlags_AlwaysClamp
    );


    ImGui::DragFloat(
        "Branch Angle",
        &m_treeBranchAngle,
        1.0f,
        0.0f,
        80.0f,
        "%.1f",
        ImGuiSliderFlags_AlwaysClamp
    );


    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();


    // =================================================
    // LEAVES
    // =================================================

    ImGui::Text("Leaves");


    ImGui::DragFloat(
        "Leaf Size",
        &m_treeLeafSize,
        0.05f,
        0.2f,
        4.0f,
        "%.2f",
        ImGuiSliderFlags_AlwaysClamp
    );


    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();


    // =================================================
    // RANDOM SEED
    // =================================================

    ImGui::Text("Variation");


    ImGui::InputInt(
        "Seed",
        &m_treeSeed
    );


    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();


    // =================================================
    // GENERATE
    // =================================================

    if (ImGui::Button(
        "Generate Tree",
        ImVec2(140.0f, 32.0f)))
    {
        action = EcoSystemAction::AddTrees;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Finish Tree");

    ImGui::TextDisabled("Shift-select both in Scene Collection. Select the trunk and leaves, then join them."
    );

    if (ImGui::Button("Join Tree",
        ImVec2(140.0f, 32.0f)))
    {
        action = EcoSystemAction::JoinTree;
    }


    return action;
}

// ----------------------------------------------------------------------------------------------------------------
// Sky Generator
// -----------------------------------------------------

EcoSystemAction EcosystemPanel::SkyTab(BoxEngine& engine)
{

    EcoSystemAction action = EcoSystemAction::None;

    Sky& sky = engine.GetSky();


    if (sky.IsLoading())
    {
        sky.LoadNextSky();
    }


    // =================================================
    // SKY
    // =================================================

    ImGui::Text("Sky Generator");

    ImGui::Separator();

    ImGui::TextDisabled(
        "Select a cubemap sky for the scene."
    );

    ImGui::Spacing();


    // =================================================
    // LOAD SKY LIBRARY
    // =================================================
        
    const bool skiesLoaded = sky.HasLoadedSkies();

    const bool disableLoadButton = sky.HasLoadedSkies() || sky.IsLoading();

	// Load Skies button, disabled if skies are already loaded or loading
    ImGui::BeginDisabled(disableLoadButton);

    if (ImGui::Button("Load Skies",
        ImVec2(140.0f, 32.0f)))
    {
        action = EcoSystemAction::AddSky;
    }

    ImGui::EndDisabled();

	ImGui::SameLine();

	// Remove Sky button, disabled if no sky is active
    ImGui::BeginDisabled(
        !sky.HasActiveSky()
    );

    if (ImGui::Button("Remove Sky",
        ImVec2(140.0f, 32.0f)))
    {
        sky.ClearSky();
    }

    ImGui::EndDisabled();


    if (skiesLoaded)
    {
        ImGui::SameLine();

        ImGui::TextDisabled("Sky library loaded");
    }

    /*if (ImGui::Button(
        "Load Skies",
        ImVec2(140.0f, 32.0f)))
    {
        action = EcoSystemAction::AddSky;
    }*/

	// progress bar for loading sky textures
    if (sky.IsLoading())
    {
        ImGui::Spacing();

        ImGui::Text("Loading sky textures...");

        ImGui::ProgressBar(sky.GetLoadProgress(), ImVec2(-1.0f, 20.0f));
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();


    // =================================================
    // SKY PICKER
    // =================================================

    ImGui::Text("Available Skies");


    const auto& skyTextures =
        engine.GetSky().GetSkyTextures();


    if (skyTextures.empty())
    {
        ImGui::TextDisabled(
            "No sky textures loaded."
        );

        return action;
    }


    const std::size_t selectedIndex =
        engine.GetSky().GetSelectedSkyIndex();


    // =================================================
    // THUMBNAIL GRID
    // =================================================

    const int columns = 4;

    int count = 0;


    ImGui::BeginChild(
        "SkyTextureGrid",
        ImVec2(0.0f, 300.0f),
        true
    );


    for (std::size_t i = 0;
        i < skyTextures.size();
        ++i)
    {
        const SkyTexture& sky =
            skyTextures[i];


        ImGui::PushID(
            static_cast<int>(i)
        );


        // -----------------------------------------
        // Selected sky outline
        // -----------------------------------------

        const ImVec2 cursorPos =
            ImGui::GetCursorScreenPos();


        if (ImGui::ImageButton(
            "##SkyPreview",
            (ImTextureID)(static_cast<intptr_t>(sky.frontFaceTexID)),
            ImVec2(64.0f, 64.0f)))
        {
            engine.GetSky().SetSkyTexture(i);
        }


        // -----------------------------------------
        // Draw selection outline
        // -----------------------------------------

        if (i == selectedIndex)
        {
            ImDrawList* drawList =
                ImGui::GetWindowDrawList();

            drawList->AddRect(
                cursorPos,
                ImVec2(
                    cursorPos.x + 64.0f,
                    cursorPos.y + 64.0f
                ),
                IM_COL32(
                    255,
                    170,
                    40,
                    255
                ),
                2.0f,
                0,
                3.0f
            );
        }


		


        ImGui::PopID();


        ++count;

        if (count % columns != 0)
        {
            ImGui::SameLine();
        }
    }


    ImGui::EndChild();


    return action;

}

// ---------------------------------------------------------------------------------------------------------------------
// Environment Generator
// -----------------------------------------------------

void EcosystemPanel::EnvironmentTab(BoxEngine& engine)
{
    ImGui::Text("Environment Generator");
    ImGui::Separator();

    ImGui::TextDisabled("Procedural environment generation coming soon...");
}
