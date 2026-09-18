#include "panels/EcosystemPanel.h"
//#include <mesh/MeshData.h>
#include <BoxEngine.h>
#include <imgui/imgui.h>
#include <miniBoxLog.h>

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
//EcoSystemAction EcosystemPanel::Draw(BoxEngine& engine, Entity& entity)
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
            // GENERATE
            // =============================================

            if (ImGui::Button(
                "Generate Floor",
                ImVec2(140.0f, 32.0f)))
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
            RocksTab(
                engine
            );

            ImGui::EndTabItem();
        }


        // -------------------------------------------------
        // TERRAIN
        // -------------------------------------------------

        if (ImGui::BeginTabItem("Terrain"))
        {
            TerrainTab(
                engine
            );

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
            GrassTab(
                engine
            );

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
            TreesTab(
                engine
            );

            ImGui::EndTabItem();
        }

        // -------------------------------------------------
        // SKY
        // -------------------------------------------------

        if (ImGui::BeginTabItem("Sky"))
        {
            SkyTab(
                engine
            );

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

void EcosystemPanel::FloorTab(BoxEngine& engine)
{
    ImGui::Text("Floor Generator");
    ImGui::Separator();

    ImGui::TextDisabled(
        "Editable ecosystem floor coming next..."
    );
}

void EcosystemPanel::RocksTab(BoxEngine& engine)
{

    ImGui::Text("Rock Generator");
    ImGui::Separator();

    ImGui::TextDisabled("Procedural rocks and scatter coming soon...");
}

void EcosystemPanel::TerrainTab(BoxEngine& engine)
{
    // TODO: Implement terrain generation logic here
    ImGui::Text("Terrain Generator");
    ImGui::Separator();

    ImGui::TextDisabled("Procedural terrain generation coming soon...");
}

void EcosystemPanel::WaterTab(BoxEngine& engine)
{
    // TODO: Implement water generation logic here
    ImGui::Text("Water Generator");
    ImGui::Separator();

    ImGui::TextDisabled("Procedural water generation coming soon...");
}

void EcosystemPanel::PlantsTab(BoxEngine& engine)
{
    // TODO: Implement plants generation logic here
    ImGui::Text("Plants Generator");
    ImGui::Separator();

    ImGui::TextDisabled("Procedural plants generation coming soon...");
}

void EcosystemPanel::GrassTab(BoxEngine& engine)
{
    // TODO: Implement grass generation logic here
    ImGui::Text("Grass Generator");
    ImGui::Separator();

    ImGui::TextDisabled("Procedural grass generation coming soon...");
}

void EcosystemPanel::TreesTab(BoxEngine& engine)
{
    // TODO: Implement trees generation logic here
    ImGui::Text("Trees Generator");
    ImGui::Separator();

    ImGui::TextDisabled("Procedural trees generation coming soon...");
}

void EcosystemPanel::SkyTab(BoxEngine& engine)
{
    ImGui::Text("Sky Generator");
    ImGui::Separator();

    ImGui::TextDisabled("Procedural sky generation coming soon...");
}

void EcosystemPanel::EnvironmentTab(BoxEngine& engine)
{
    ImGui::Text("Environment Generator");
    ImGui::Separator();

    ImGui::TextDisabled("Procedural environment generation coming soon...");
}
