#include "panels/EcosystemPanel.h"

#include <BoxEngine.h>
#include <entity/Entity.h>
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

    BOX_LOG_INFO(
        "EcosystemPanel::Open - m_isOpen = "
        << m_isOpen
    );
}

void EcosystemPanel::Close()
{
    m_isOpen = false;
}

bool EcosystemPanel::IsOpen() const
{
    return m_isOpen;
}

void EcosystemPanel::Draw(BoxEngine& engine, Entity& entity)
{
    if (!m_isOpen)
    {
        return;
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
        return;
    }


    // =================================================
    // ECOSYSTEM TABS
    // =================================================

    if (ImGui::BeginTabBar(
        "##EcosystemTabs"))
    {
        // -------------------------------------------------
        // FLOOR
        // -------------------------------------------------

        if (ImGui::BeginTabItem(
            "Floor"))
        {
            FloorTab(
                engine,
                entity
            );

            ImGui::EndTabItem();
        }


        // -------------------------------------------------
        // ROCKS
        // -------------------------------------------------

        if (ImGui::BeginTabItem(
            "Rocks"))
        {
            RocksTab(
                engine,
                entity
            );

            ImGui::EndTabItem();
        }


        // -------------------------------------------------
        // TERRAIN
        // -------------------------------------------------

        if (ImGui::BeginTabItem(
            "Terrain"))
        {
            TerrainTab(
                engine,
                entity
            );

            ImGui::EndTabItem();
        }


        // -------------------------------------------------
        // WATER
        // -------------------------------------------------

        if (ImGui::BeginTabItem(
            "Water"))
        {
            WaterTab(
                engine,
                entity
            );

            ImGui::EndTabItem();
        }


        // -------------------------------------------------
        // GRASS
        // -------------------------------------------------

        if (ImGui::BeginTabItem(
            "Grass"))
        {
            GrassTab(
                engine,
                entity
            );

            ImGui::EndTabItem();
        }


        // -------------------------------------------------
        // PLANTS
        // -------------------------------------------------

        if (ImGui::BeginTabItem(
            "Plants"))
        {
            PlantsTab(
                engine,
                entity
            );

            ImGui::EndTabItem();
        }


        // -------------------------------------------------
        // TREES
        // -------------------------------------------------

        if (ImGui::BeginTabItem(
            "Trees"))
        {
            TreesTab(
                engine,
                entity
            );

            ImGui::EndTabItem();
        }


        ImGui::EndTabBar();
    }


    ImGui::End();
}

void EcosystemPanel::FloorTab(BoxEngine& engine, Entity& entity)
{
    ImGui::Text("Floor Generator");
    ImGui::Separator();

    ImGui::TextDisabled(
        "Editable ecosystem floor coming next..."
    );
}

void EcosystemPanel::RocksTab(BoxEngine & engine, Entity & entity)
{

    ImGui::Text("Rock Generator");
    ImGui::Separator();

    ImGui::TextDisabled("Procedural rocks and scatter coming soon...");
}

void EcosystemPanel::TerrainTab(BoxEngine & engine, Entity & entity)
{
	// TODO: Implement terrain generation logic here
    ImGui::Text("Terrain Generator");
    ImGui::Separator();

    ImGui::TextDisabled("Procedural terrain generation coming soon...");
}

void EcosystemPanel::WaterTab(BoxEngine & engine, Entity & entity)
{
	// TODO: Implement water generation logic here
    ImGui::Text("Water Generator");
    ImGui::Separator();

    ImGui::TextDisabled("Procedural water generation coming soon...");
}

void EcosystemPanel::PlantsTab(BoxEngine & engine, Entity & entity)
{
	// TODO: Implement plants generation logic here
    ImGui::Text("Plants Generator");
    ImGui::Separator();

    ImGui::TextDisabled("Procedural plants generation coming soon...");
}

void EcosystemPanel::GrassTab(BoxEngine & engine, Entity & entity)
{
	// TODO: Implement grass generation logic here
    ImGui::Text("Grass Generator");
    ImGui::Separator();

    ImGui::TextDisabled("Procedural grass generation coming soon...");
}

void EcosystemPanel::TreesTab(BoxEngine & engine, Entity & entity)
{
	// TODO: Implement trees generation logic here
    ImGui::Text("Trees Generator");
    ImGui::Separator();

    ImGui::TextDisabled("Procedural trees generation coming soon...");
}
