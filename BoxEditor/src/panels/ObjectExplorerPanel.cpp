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
#include <tools/FaceEditController.h>
#include <tools/EdgeEditController.h>
#include <algorithm>

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


void ObjectExplorerPanel::DrawObjectExplorer(BoxEngine& engine, FaceEditController& faceEditController,
    EdgeEditController& edgeEditController)
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
		// Textures tab NOT USED FOR NOW, MATERIAL EDITOR PANEL IS USED INSTEAD
        // ####################################################
        if (ImGui::BeginTabItem("Textures"))
        {
            DrawTexturesTab(engine,*selected, faceEditController);

            ImGui::EndTabItem();
        }

        // ####################################################
        // Modifiers tab
        // ####################################################
        if (ImGui::BeginTabItem("Modifiers"))
        {
            DrawModifiersTab(engine, *selected, faceEditController, edgeEditController);

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
    
}

void ObjectExplorerPanel::DrawObjectTab(BoxEngine& engine, Entity& entity)
{
   

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
	ImGui::Spacing();
    ImGui::Separator();
   
    // =================================================
    // SHADING
    // =================================================

    bool smoothShading =
        entity.IsSmoothShading();

    if (ImGui::Checkbox(
        "Smooth Shading",
        &smoothShading))
    {
        entity.SetSmoothShading(
            smoothShading
        );
    }

    ImGui::Spacing();


	// -------------------------------------------
	//  Objects properties 
	// -------------------------------------------

    switch (entity.GetPrimitiveType())
    {
    case EntityPrimitiveType::Cube:
    {
        // Cube properties later.

        break;
    }
    case EntityPrimitiveType::Plane:
    {
        // Plane properties later.

        break;
    }

    // -----------------------------------------
    //  Cylinder properties 
    // -----------------------------------------

    case EntityPrimitiveType::Cylinder:
    {
        ImGui::SeparatorText("Cylinder Properties");

        bool cylinderChanged = false;

        float radius = entity.GetCylinderRadius();

        int sectors = entity.GetCylinderSectors();

        int stacks = entity.GetCylinderStacks();

        float height = entity.GetCylinderHeight();


        // -----------------------------------------
        // Radius
        // -----------------------------------------

        if (ImGui::InputFloat("Radius", &radius, 0.05f, 0.25f))
        {
            radius = std::clamp(radius, 0.05f, 6.0f);

            entity.SetCylinderRadius(radius);

            cylinderChanged = true;
        }

        // -----------------------------------------
        // Sectors
        // -----------------------------------------

        if (ImGui::InputInt("Sectors", &sectors, 1, 4))
        {
            sectors = std::clamp(sectors, 3, 64);

            entity.SetCylinderSectors(sectors);

            cylinderChanged = true;
        }


        // -----------------------------------------
        // Stacks
        // -----------------------------------------

        if (ImGui::InputInt("Stacks", &stacks, 1, 1))
        {
            stacks = std::clamp(stacks, 1, 8);

            entity.SetCylinderStacks(stacks);

            cylinderChanged = true;
        }
        // -----------------------------------------
		// Height
        // -----------------------------------------
        
        if (ImGui::InputFloat("Height", &height, 0.05f, 0.25f))
        {
            height = std::clamp(height, 0.05f, 6.0f);

            entity.SetCylinderHeight(height);

            cylinderChanged = true;
        }

        // -----------------------------------------
        // Rebuild once if anything changed.
        // -----------------------------------------

        if (cylinderChanged)
        {
            entity.ClearSelectedVertices();
            entity.ClearSelectedEdges();
            entity.ClearSelectedFace();

            entity.UpdateCylinder();
        }

        break;
    }

    // -----------------------------------------
    // Sphere properties
    // -----------------------------------------

    case EntityPrimitiveType::Sphere:
    {
        ImGui::SeparatorText("Sphere Properties");

        bool sphereChanged = false;

               
        int sectors = entity.GetSphereSectors();

        int stacks = entity.GetSphereStacks();

        

        // -----------------------------------------
        // Sectors
        // -----------------------------------------

        if (ImGui::InputInt("Sectors", &sectors, 1, 4))
        {
            sectors = std::clamp(sectors, 8, 32);

            entity.SetSphereSectors(sectors);

            sphereChanged = true;
        }

        // -----------------------------------------
        // Stacks
        // -----------------------------------------

        if (ImGui::InputInt("Stacks", &stacks, 1, 1))
        {
            stacks = std::clamp(stacks, 2, 16);

            entity.SetSphereStacks(stacks);

            sphereChanged = true;
        }

        

        if (sphereChanged)
        {
            entity.ClearSelectedVertices();
            entity.ClearSelectedEdges();
            entity.ClearSelectedFace();

            entity.UpdateSphere();
        }

        break;
    }


    


    case EntityPrimitiveType::Pyramid:
    {
        // Pyramid properties later.

        break;
    }


    case EntityPrimitiveType::None:
    default:
    {
        break;
    }
    }








 
    bool visible = entity.IsVisible();
    if (ImGui::Checkbox("Visible", &visible)) {
        entity.SetVisible(visible);
    }
}



// Textures and Material tab.
void ObjectExplorerPanel::DrawTexturesTab(BoxEngine& engine, Entity& entity,
    FaceEditController& faceEditController)
{
    

    //ImGui::Separator();

    //ImGui::TextDisabled(
    //    "No textures assigned."
    //);

    //ImGui::Spacing();
    //
    //// Draw the material editor for the selected entity.
    //if (m_materialEditor)
    //{
    //    m_materialEditor->Draw(engine, entity, faceEditController);
    //}
    //else
    //{
    //    ImGui::TextDisabled(
    //        "Material editor is not initialized."
    //    );
    //}

}

void ObjectExplorerPanel::DrawModifiersTab(BoxEngine& engine, Entity& entity, FaceEditController& faceEditController,
    EdgeEditController& edgeEditController)
{
    ImGui::Text("Modifiers for: %s", entity.GetName().c_str());

    ImGui::Separator();

    // =================================================
    // ACTIVE OPERATION ALWAYS HAS PRIORITY
    // =================================================

    // Extrude
    if (faceEditController.IsExtruding())
    {
        ExtrudeControls(entity, faceEditController);

        return;
    }

	// Inset
    if (faceEditController.IsInsetting())
    {
        InsetControls(entity, faceEditController);

        return;
    }
	// Loop Cut
    if (edgeEditController.IsLoopCutting())
    {
        LoopCutControls(entity, edgeEditController);
        return;
	}
	// Bevel
    if (edgeEditController.IsBeveling())
    {
        BevelControls(entity, edgeEditController);
		return;
	}


    // =================================================
    // NO ACTIVE OPERATION
    // Show the last confirmed operation.
    // =================================================

    switch (entity.GetLastOperationType())
    {
        case LastOperationType::Extrude:
        {
            ExtrudeControls(entity, faceEditController);
            
            break;
        }

        case LastOperationType::Inset:
        {
            InsetControls(entity, faceEditController);
            
            break;
        }
        case LastOperationType::LoopCut:
        {
            LoopCutControls(entity, edgeEditController);
            break;
		}
        case LastOperationType::Bevel:
        {
            BevelControls(entity, edgeEditController);
            break;
		}

        case LastOperationType::None:
        default:
        {
            ImGui::TextDisabled("No modelling operations.");

            break;
        }
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

// =================================================
// Extrude Controls
// =================================================
void ObjectExplorerPanel::ExtrudeControls(Entity& entity, FaceEditController& faceEditController)
{

    if (faceEditController.IsExtruding())
    {
        ImGui::SeparatorText("Active Operation");

        ImGui::Text("Face Extrude");

        ImGui::Spacing();


        // ---------------------------------------------
        // Axis
        // ---------------------------------------------

        int axis = 0;

        switch (
            faceEditController.GetExtrudeAxis())
        {
        case ExtrudeAxis::X:
            axis = 1;
            break;

        case ExtrudeAxis::Y:
            axis = 2;
            break;

        case ExtrudeAxis::Z:
            axis = 3;
            break;

        case ExtrudeAxis::None:
        default:
            axis = 0;
            break;
        }

        const char* axisItems[] =
        {
            "None",
            "X",
            "Y",
            "Z"
        };

        if (ImGui::Combo(
            "Axis",
            &axis,
            axisItems,
            IM_ARRAYSIZE(axisItems)))
        {
            ExtrudeAxis newAxis =
                ExtrudeAxis::None;

            switch (axis)
            {
            case 1:
                newAxis =
                    ExtrudeAxis::X;
                break;

            case 2:
                newAxis =
                    ExtrudeAxis::Y;
                break;

            case 3:
                newAxis =
                    ExtrudeAxis::Z;
                break;

            case 0:
            default:
                newAxis =
                    ExtrudeAxis::None;
                break;
            }

            faceEditController
                .SetExtrudeAxis(entity, newAxis);
        }


        // ---------------------------------------------
        // Amount
        // ---------------------------------------------

        float amount = faceEditController.GetExtrudeAmount();

        if (ImGui::InputFloat(
            "Amount",
            &amount,
            0.01f,
            0.1f))
        {
            faceEditController
                .SetExtrudeAmount(
                    entity,
                    amount
                );
        }

        ImGui::Spacing();

        // ---------------------------------------------
        // Confirm / Cancel
        // ---------------------------------------------

        if (ImGui::Button(
            "Confirm",
            ImVec2(90.0f, 0.0f)))
        {
            faceEditController.ConfirmExtrude(entity);
        }

        ImGui::SameLine();

        if (ImGui::Button(
            "Cancel",
            ImVec2(90.0f, 0.0f)))
        {
            faceEditController.CancelExtrude(entity);
        }

        return;
    }

    // =================================================
    // Nothing currently active
    // =================================================



    if (entity.HasLastExtrude())
    {
        ImGui::SeparatorText("Last Operation");

        ImGui::Text("Face Extrude");

        ExtrudeModifierData lastExtrude = entity.GetLastExtrude();


        // ---------------------------------
        // Axis
        // ---------------------------------

        int axis = 0;

        switch (lastExtrude.axis)
        {
        case ModifierAxis::X:
            axis = 0;
            break;

        case ModifierAxis::Y:
            axis = 1;
            break;

        case ModifierAxis::Z:
            axis = 2;
            break;
        }

        const char* axisItems[] =
        {
            "X",
            "Y",
            "Z"
        };

        bool changed = false;

        if (ImGui::Combo(
            "Extrude Axis",
            &axis,
            axisItems,
            IM_ARRAYSIZE(axisItems)))
        {
            switch (axis)
            {
            case 0:
                lastExtrude.axis =
                    ModifierAxis::X;
                break;

            case 1:
                lastExtrude.axis =
                    ModifierAxis::Y;
                break;

            case 2:
                lastExtrude.axis =
                    ModifierAxis::Z;
                break;
            }

            changed = true;
        }


        // ---------------------------------
        // Amount
        // ---------------------------------

        if (ImGui::InputFloat(
            "Amount",
            &lastExtrude.extrudeAmount,
            0.01f,
            0.1f))
        {
            changed = true;
        }

        // ---------------------------------
        // Update geometry
        // ---------------------------------

        if (changed)
        {
            entity.UpdateLastExtrude(
                lastExtrude.axis,
                lastExtrude.extrudeAmount
            );
        }

        ImGui::TextDisabled("No active modelling operation.");

        ImGui::Spacing();

        ImGui::TextDisabled("Select a face and choose " "Modifiers > Extrude.");

        return;
    }

}
// =================================================
// Inset Controls
// =================================================
void ObjectExplorerPanel::InsetControls(Entity& entity, FaceEditController& faceEditController)
{
    // =================================================
   // ACTIVE INSET
   // =================================================

    if (faceEditController.IsInsetting())
    {
        ImGui::SeparatorText(
            "Active Operation"
        );

        ImGui::Text(
            "Face Inset"
        );

        ImGui::Spacing();

        float amount = faceEditController.GetInsetAmount();

        if (ImGui::InputFloat(
            "Inset Amount",
            &amount,
            0.01f,
            0.1f))
        {
            faceEditController
                .SetInsetAmount(
                    entity,
                    amount
                );
        }

        ImGui::Spacing();

        if (ImGui::Button(
            "Confirm",
            ImVec2(90.0f, 0.0f)))
        {
            faceEditController
                .ConfirmInset(
                    entity
                );
        }

        ImGui::SameLine();

        if (ImGui::Button(
            "Cancel",
            ImVec2(90.0f, 0.0f)))
        {
            faceEditController
                .CancelInset(
                    entity
                );
        }

        return;
    }


    // =================================================
    // LAST CONFIRMED INSET
    // =================================================

    if (entity.HasLastInset())
    {
        ImGui::SeparatorText(
            "Last Operation"
        );

        ImGui::Text(
            "Face Inset"
        );

        InsetModifierData lastInset = entity.GetLastInset();

        if (ImGui::InputFloat(
            "Inset Amount",
            &lastInset.insetAmount,
            0.01f,
            0.1f))
        {
            entity.UpdateLastInset(
                lastInset.insetAmount
                );
        }


        return;
    }

}
// =================================================
// Loop Cut Controls
// =================================================

void ObjectExplorerPanel::LoopCutControls(Entity& entity, EdgeEditController& edgeEditController)
{
    // =================================================
   // ACTIVE Loop CUT
   // =================================================

    if (edgeEditController.IsLoopCutting())
    {
        ImGui::SeparatorText(
            "Active Operation"
        );

        ImGui::Text(
            "Loop Cut"
        );

        ImGui::Spacing();

        float amount =
            edgeEditController
            .GetLoopCutAmount();
        if (ImGui::DragFloat(
            "Loop Cut Amount",
            &amount,
            0.01f,
            0.0f,
            0.95f))
        {
            edgeEditController
                .SetLoopCutAmount(
                    entity,
                    amount
                );
        }

        ImGui::Spacing();

        if (ImGui::Button(
            "Confirm",
            ImVec2(90.0f, 0.0f)))
        {
            edgeEditController.ConfirmLoopCut(entity);
        }

        ImGui::SameLine();

        if (ImGui::Button(
            "Cancel",
            ImVec2(90.0f, 0.0f)))
        {
            edgeEditController
                .CancelLoopCut(
                    entity
                );
        }

        return;
    }


    // =================================================
    // LAST CONFIRMED LOOP CUT
    // =================================================

    if (entity.HasLastLoopCut())
    {
        ImGui::SeparatorText(
            "Last Operation"
        );

        ImGui::Text(
            "Loop Cut"
        );

        LoopCutModifierData lastLoopCut = entity.GetLastLoopCut();

        if (ImGui::DragFloat(
            "Loop Cut Amount",
            &lastLoopCut.cutAmount,
            0.01f,
            0.01f,
            0.99f))
        {
            entity.UpdateLastLoopCut(
                lastLoopCut.cutAmount
            );
        }

        return;
    }

}
// =================================================
// Bevel Controls
// =================================================
void ObjectExplorerPanel::BevelControls(Entity& entity, EdgeEditController& edgeEditController)
{
    // =================================================
   // ACTIVE BEVEL
   // =================================================

    if (edgeEditController.IsBeveling())
    {
        ImGui::SeparatorText(
            "Active Operation"
        );

        ImGui::Text(
            "Edge Bevel"
        );

        ImGui::Spacing();


        float width =
            edgeEditController.GetBevelWidth();

        int segments =
            edgeEditController.GetBevelSegments();

        float profile =
            edgeEditController.GetBevelProfile();

        bool changed = false;

        // ---------------------------------------------
        // Width
        // ---------------------------------------------

        if (ImGui::InputFloat(
            "Bevel Width",
            &width,
            0.01f,
            0.1f))
        {
            width =
                std::clamp(
                    width,
                    0.035f,
                    0.400f
                );

            changed = true;
        }


        // ---------------------------------------------
        // Segments
        // ---------------------------------------------

        if (ImGui::InputInt(
            "Bevel Segments",
            &segments,
            1,
            1))
        {
            segments =
                std::clamp(
                    segments,
                    1,
                    10
                );

            changed = true;
        }


        // ---------------------------------------------
        // Profile
        // ---------------------------------------------

        if (ImGui::InputFloat(
            "Bevel Profile",
            &profile,
            0.01f,
            0.1f))
        {
            profile =
                std::clamp(
                    profile,
                    0.0f,
                    1.0f
                );

            changed = true;
        }


        if (changed)
        {
            edgeEditController.SetBevelValues(
                entity,
                width,
                segments,
                profile
            );
        }

        ImGui::Spacing();
        // ---------------------------------------------
        // Confirm / Cancel
        // ---------------------------------------------

        if (ImGui::Button(
            "Confirm",
            ImVec2(90.0f, 0.0f)))
        {
            edgeEditController
                .ConfirmBevel(entity);
        }


        ImGui::SameLine();


        if (ImGui::Button(
            "Cancel",
            ImVec2(90.0f, 0.0f)))
        {
            edgeEditController
                .CancelBevel(entity);
        }


        return;
    }


    // =================================================
    // LAST CONFIRMED BEVEL
    // =================================================

    if (entity.HasLastBevel())
    {
        ImGui::SeparatorText(
            "Last Operation"
        );

        ImGui::Text(
            "Edge Bevel"
        );


        BevelModifierData lastBevel =
            entity.GetLastBevel();


        bool changed = false;

        if (ImGui::InputFloat(
            "Bevel Width",
            &lastBevel.width,
            0.01f,
            0.1f))
        {
            lastBevel.width =
                std::clamp(
                    lastBevel.width,
                    0.035f,
                    0.400f
                );

            changed = true;
        }
         

        if (ImGui::InputInt(
            "Bevel Segments",
            &lastBevel.segments,
            1,      // +/- changes by 1
            1       // Ctrl +/- also changes by 1
        ))
        {
            // Keep it within our allowed range.
            lastBevel.segments =
                std::clamp(
                    lastBevel.segments,
                    1,
                    10
                );

            changed = true;
        }

        if (ImGui::InputFloat("Bevel Profile", &lastBevel.profile, 0.01f, 0.1f))
        {
            // Keep it within our allowed range.
            lastBevel.profile =
                std::clamp(
                    lastBevel.profile,
                    0.0f,
                    1.0f);
            changed = true;
		}

        if (changed)
        {
            entity.UpdateLastBevel(
                lastBevel.width,
                lastBevel.segments,
                lastBevel.profile
            );
        }


        return;
    }


    ImGui::TextDisabled(
        "No bevel operation."
    );

}
