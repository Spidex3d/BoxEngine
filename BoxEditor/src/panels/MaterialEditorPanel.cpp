#include "panels/MaterialEditorPanel.h"

#include <BoxEngine.h>
#include <entity/Entity.h>
#include <imgui/imgui.h>
#include <miniBoxLog.h>

#include <tools/FaceEditController.h>
//#include "panels/MaterialEditor.h"


// this is the new implementation of the MaterialEditorPanel class,
// which is a panel that allows the user to edit materials in the BoxEditor application.
// It provides methods to make new Material & save, open Materials 

MaterialEditorPanel::~MaterialEditorPanel() = default;

bool MaterialEditorPanel::Initialize()
{
    BOX_LOG_INFO("MaterialEditorPanel::Initialize called");


    m_materialEditor = std::make_unique<MaterialEditor>();

    if (!m_materialEditor->Initialize())
    {
        m_materialEditor.reset();
        return false;
    }

    return true;
}


void MaterialEditorPanel::Open()
{
    m_isOpen = true;
}

void MaterialEditorPanel::Close()
{
    m_isOpen = false;
}

bool MaterialEditorPanel::IsOpen() const
{
    return m_isOpen;
}

void MaterialEditorPanel::Draw(BoxEngine& engine, Entity& entity,
    FaceEditController& faceEditController)
{

    if (!m_isOpen)
    {
        return;
    }


    if (ImGui::Begin(
        "Material Editor",
        &m_isOpen))
    {
		ImGui::SeparatorText("Material Editor Panel");

        // Draw the material editor for the selected entity.
        if (m_materialEditor)
        {
            m_materialEditor->Draw(engine, entity, faceEditController);
        }
        else
        {
            ImGui::TextDisabled("Material editor is not initialized.");
        }

    }


    ImGui::End();
   
}
	