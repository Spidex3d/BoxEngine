#include <BoxEngine.h>
#include "panels/ObjectExplorerPanel.h"
#include <panels/MaterialEditor.h>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui/imgui.h>
#include <imgui/ImGuiAF.h>

#include <entity/Entity.h>
#include <cstring>
#include <string>
#include <miniBoxLog.h>
//#include "MatirialBrowser.h"

ObjectExplorerPanel::ObjectExplorerPanel() = default;

ObjectExplorerPanel::~ObjectExplorerPanel()
{
    Shutdown();
}

bool ObjectExplorerPanel::Initialize()
{
    BOX_LOG_INFO(
        "ObjectExplorerPanel::Initialize called"
    );


    m_materialEditor = std::make_unique<MaterialEditor>();

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
            DrawObjectTab(engine, *selected);

            ImGui::EndTabItem();
        }

        // ####################################################
        // Textures tab
        // ####################################################
        if (ImGui::BeginTabItem("Textures"))
        {
            DrawTexturesTab(engine,*selected);

            ImGui::EndTabItem();
        }

        // ####################################################
        // Modifiers tab
        // ####################################################
        if (ImGui::BeginTabItem("Modifiers"))
        {
            DrawModifiersTab(engine, *selected);

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
    
}

void ObjectExplorerPanel::DrawObjectTab(BoxEngine& engine, Entity& entity)
{
   // char nameBuffer[128]{};

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
void ObjectExplorerPanel::DrawTexturesTab(BoxEngine& engine,Entity& entity)
{
    

    ImGui::Separator();

    ImGui::TextDisabled(
        "No textures assigned."
    );

    ImGui::Spacing();
    
    // Draw the material editor for the selected entity.
    if (m_materialEditor)
    {
        m_materialEditor->Draw(engine, entity);
    }
    else
    {
        ImGui::TextDisabled(
            "Material editor is not initialized."
        );
    }

}

void ObjectExplorerPanel::DrawModifiersTab(BoxEngine& engine, Entity& entity)
{
	// the name of the entity is displayed at the top of the modifiers tab.
    ImGui::Text("Modifiers for: %s", entity.GetName().c_str());

    Entity* selectedEntity = engine.GetSelectedEntity();

	// Example of handling a specific modifier type (Extrude) for the selected entity.
    // Handle Extrude modifier logic here.
    if (selectedEntity)
    {
		// Example: Check if the selected entity has a modifier applied. then display the modifier controls.
        // This is a placeholder for actual modifier logic.
        bool hasExtrudeModifier = true; // Replace with actual check.
        if (hasExtrudeModifier)
        {
            ImGui::Text("Extrude modifier is applied.");
            // Add controls for the Extrude modifier here.
            float fex = 0.0f;

                if (ImGui::InputFloat("Extrude", &fex)) {
					// this is where you would update the extrude value to the selected entity's mesh data.  
                    
                }

        }
	}

    ImGui::Separator();

    ImGui::TextDisabled(
        "No modifiers applied."
    );

    ImGui::Spacing();

    if (ImGui::Button(
        "Update Modifier", ImVec2(120.0f, 0.0f)))
    {
        
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


