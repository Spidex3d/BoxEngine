#include <BoxEngine.h>
#include "panels/ObjectExplorerPanel.h"
#include <panels/MaterialEditor.h>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui/imgui.h>
#include <imgui/ImGuiAF.h>

#include <entity/Entity.h>
#include <cstring>
#include <string>

ObjectExplorerPanel::ObjectExplorerPanel() = default;

ObjectExplorerPanel::~ObjectExplorerPanel()
{
    Shutdown();
}

bool ObjectExplorerPanel::Initialize()
{
    m_materialEditor =
        std::make_unique<MaterialEditor>();

    if (!m_materialEditor->Initialize())
    {
        m_materialEditor.reset();
        return false;
    }

    return true;
}


void ObjectExplorerPanel::DrawObjectExplorer(
    BoxEngine& engine)
{

    Entity* selected =
        engine.GetSelectedEntity();

    const int selectedID =
        selected
        ? selected->GetID()
        : -1;

    // Open the panel when a new entity is selected.
    if (selectedID != -1 &&
        selectedID != m_lastSelectedEntityID)
    {
        m_isOpen = true;
    }

    m_lastSelectedEntityID =
        selectedID;

    if (!m_isOpen)
    {
        return;
    }

    if (!ImGui::Begin(
        "Object Explorer",
        &m_isOpen))
    {
        ImGui::End();
        return;
    }

    if (!selected)
    {
        ImGui::TextDisabled(
            "No object selected."
        );

        ImGui::End();
        return;
    }

    if (ImGui::BeginTabBar(
        "ObjectExplorerTabs"))
    {
        // ####################################################
        // Object tab
        // ####################################################
        if (ImGui::BeginTabItem("Object"))
        {
            DrawObjectTab(*selected);

            ImGui::EndTabItem();
        }

        // ####################################################
        // Textures tab
        // ####################################################
        if (ImGui::BeginTabItem("Textures"))
        {
            DrawTexturesTab(*selected);

            ImGui::EndTabItem();
        }

        // ####################################################
        // Modifiers tab
        // ####################################################
        if (ImGui::BeginTabItem("Modifiers"))
        {
            DrawModifiersTab(*selected);

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
    
}

void ObjectExplorerPanel::DrawObjectTab(Entity& entity)
{
    char nameBuffer[128]{};

    strncpy_s(nameBuffer, sizeof(nameBuffer), entity.GetName().c_str(), _TRUNCATE);

    if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
    {
        entity.SetName(std::string(nameBuffer));
    }

    ImGui::SeparatorText("Object Properties");

    float pos[3] = { entity.GetPosition().x, entity.GetPosition().y, entity.GetPosition().z };
    if (ImGui::InputFloat3("Position", pos)) {
        entity.SetPosition(glm::vec3(pos[0], pos[1], pos[2]));
    }

	// Rotation
    float rotation[3] = { entity.GetRotation().x, entity.GetRotation().y, entity.GetRotation().z };
    if (ImGui::InputFloat3("Rotation", rotation))
    {
        entity.SetRotation(glm::vec3(rotation[0], rotation[1], rotation[2]));
    }

    // Scale
    float sc[3] = { entity.GetScale().x, entity.GetScale().y, entity.GetScale().z };
    if (ImGui::InputFloat3("Scale", sc)) {
        entity.SetScale(glm::vec3(sc[0], sc[1], sc[2]));
    }

    bool visible = entity.IsVisible();
    if (ImGui::Checkbox("Visible", &visible)) {
        entity.SetVisible(visible);
    }
}

// Textures and Material tab.
void ObjectExplorerPanel::DrawTexturesTab(Entity& entity)
{
    

    ImGui::Separator();

    ImGui::TextDisabled(
        "No textures assigned."
    );

    ImGui::Spacing();

	// Draw the material editor for the selected entity.
    m_materialEditor->Draw(entity);

    
	// We can do Textures after we have material working properly, including Save and Load.

    ImGui::Spacing();
    ImGui::SeparatorText("Texture settings");
    ImGui::Spacing();

    if (ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("Textures for: %s", entity.GetName().c_str());

        ImGui::TextDisabled(
            "Add a texture to the object."
        );
        if (ImGui::Button(
            "Add Texture",
            ImVec2(120.0f, 0.0f))) // change this to an image button with a texture icon later.
        {
            // Texture browser or file dialog later.
        }
    }
}

void ObjectExplorerPanel::DrawModifiersTab(Entity& entity)
{
    ImGui::Text(
        "Modifiers for: %s",
        entity.GetName().c_str()
    );

    ImGui::Separator();

    ImGui::TextDisabled(
        "No modifiers applied."
    );

    ImGui::Spacing();

    if (ImGui::Button(
        "Add Modifier",
        ImVec2(120.0f, 0.0f)))
    {
        // Modifier popup later.
    }
}

void ObjectExplorerPanel::Shutdown()
{
    if (m_materialEditor)
    {
        m_materialEditor->Shutdown();
        m_materialEditor.reset();
    }
}


