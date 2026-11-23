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
#include <tools/VertexEditController.h>
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
    EdgeEditController& edgeEditController, VertexEditController& vertexEditController)
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
        if (ImGui::BeginTabItem("Lighting"))
        {
            DrawLightingTab(engine);

            ImGui::EndTabItem();
        }

        // ####################################################
        // Modifiers tab
        // ####################################################
        if (ImGui::BeginTabItem("Modifiers"))
        {
            DrawModifiersTab(engine, *selected, faceEditController, edgeEditController, vertexEditController);

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

	case EntityPrimitiveType::Cone:
	{
		// Cone properties later.
        ImGui::SeparatorText("Cone Properties");

        bool coneChanged = false;

        float radius = entity.GetConeRadius();

        int sectors = entity.GetConeSectors();

        int stacks = entity.GetConeStacks();

        float height = entity.GetConeHeight();

		// Radius
        if (ImGui::InputFloat("Radius", &radius, 0.05f, 0.25f))
        {
            radius = std::clamp(radius, 0.05f, 6.0f);

            entity.SetConeRadius(radius);

            coneChanged = true;
        }
        // Height
		if (ImGui::InputFloat("Height", &height, 0.05f, 0.25f))
		{
			height = std::clamp(height, 0.05f, 6.0f);
			entity.SetConeHeight(height);
			coneChanged = true;
		}

		// Sectors
		if (ImGui::InputInt("Sectors", &sectors, 1, 4))
		{
			sectors = std::clamp(sectors, 3, 34);
			entity.SetConeSectors(sectors);
			coneChanged = true;
		}
		// Stacks
		if (ImGui::InputInt("Stacks", &stacks, 1, 1))
		{
			stacks = std::clamp(stacks, 1, 4);
			entity.SetConeStacks(stacks);
			coneChanged = true;
		}

		if (coneChanged)
		{
			entity.ClearSelectedVertices();
			entity.ClearSelectedEdges();
			entity.ClearSelectedFace();

			entity.UpdateCone();
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
//void ObjectExplorerPanel::DrawTexturesTab(BoxEngine& engine, Entity& entity, FaceEditController& faceEditController)
void ObjectExplorerPanel::DrawLightingTab(BoxEngine& engine)
{
    Lighting& lighting = engine.GetLighting();

    ImGui::SeparatorText(
        "Scene Lighting"
    );


    // =================================================
    // Enabled
    // =================================================

    bool enabled =
        lighting.IsEnabled();

    if (ImGui::Checkbox(
        "Enabled",
        &enabled))
    {
        lighting.SetEnabled(
            enabled
        );
    }


    // =================================================
    // Direction
    // =================================================

    glm::vec3 direction =
        lighting.GetDirection();

    if (ImGui::DragFloat3(
        "Direction",
        &direction.x,
        0.01f,
        -1.0f,
        1.0f))
    {
        lighting.SetDirection(
            direction
        );
    }


    // =================================================
    // Colour
    // =================================================

    glm::vec3 color =
        lighting.GetColor();

    if (ImGui::ColorEdit3(
        "Light Colour",
        &color.x))
    {
        lighting.SetColor(
            color
        );
    }


    // =================================================
    // Intensity
    // =================================================

    float intensity =
        lighting.GetIntensity();

    if (ImGui::DragFloat(
        "Intensity",
        &intensity,
        0.05f,
        0.0f,
        10.0f))
    {
        lighting.SetIntensity(
            intensity
        );
    }

}

void ObjectExplorerPanel::DrawModifiersTab(BoxEngine& engine, Entity& entity, FaceEditController& faceEditController,
    EdgeEditController& edgeEditController, VertexEditController& vertexEditController)
{
    ImGui::Text("Modifiers for: %s", entity.GetName().c_str());

    // ============================================
    // PERSISTENT MIRROR SELECTION
    // ============================================

    MirrorControls(entity);

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::Separator();

    // =================================================
    // ACTIVE OPERATION ALWAYS HAS PRIORITY
    // =================================================

    // =================================================
    // ACTIVE VERTEX MOVEMENT
    // =================================================

    if (vertexEditController.IsMoving())
    {
        VertexMoveControls(entity, vertexEditController);

        return;
    }

    // ============================================
    // ACTIVE EDGE MOVEMENT
    // ============================================

    if (edgeEditController.IsMultiMoving())
    {
        EdgeMoveControls(entity, edgeEditController);

        return;
    }

    // ============================================
    // ACTIVE FACE MOVEMENT
    // ============================================

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

    // Round Inset
    if (faceEditController.IsRoundingInset())
    {
        RoundInsetControls(entity, faceEditController);

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

    // ============================================
    // VERTEX SELECTION - SHOW MOVE CONTROLS
    // ============================================

    if (!entity.GetSelectedVertices().empty())
    {
        VertexMoveControls(entity, vertexEditController);

        return;
    }

    // ============================================
    // EDGE SELECTION - SHOW MOVE CONTROLS
    // ============================================

    if (!entity.GetSelectedEdges().empty())
    {
        EdgeMoveControls(
            entity,
            edgeEditController
        );

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
        case LastOperationType::RoundInset:
        {
            RoundInsetControls(entity, faceEditController);

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

        // ============================================
        // EXTRUSION MODE
        // ============================================

        ImGui::Spacing();

        bool regionMode =
            faceEditController.IsRegionExtrude();

        if (ImGui::Checkbox(
            "Extrude as Region",
            &regionMode))
        {
            faceEditController.SetRegionExtrude(entity, regionMode);
        }

        ImGui::TextDisabled(
            regionMode
            ? "Connected faces extrude together"
            : "Each face extrudes individually"
        );

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
            
        case ExtrudeAxis::Normal:
            axis = 4;
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
            "Z",
            "Normal"
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

            case 4:
                newAxis =
                    ExtrudeAxis::Normal;
                break;


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

void ObjectExplorerPanel::RoundInsetControls(Entity& entity, FaceEditController& faceEditController)
{

    if (faceEditController.IsRoundingInset())
    {
        ImGui::SeparatorText("Active Operation");

        ImGui::Text("Round Inset");

        ImGui::Spacing();

        float amount = faceEditController.GetRoundInsetAmount();

        if (ImGui::InputFloat(
            "Round Inset Amount",
            &amount,
            0.01f,
            0.1f))
        {
            faceEditController
                .SetRoundInsetAmount(
                    entity, amount);
        }
        
        int segments = faceEditController.GetRoundSegs();

        if (ImGui::InputInt(
            "Round Inset segments",
            &segments,
            1,
            1))
        {
            faceEditController
                .SetRoundSegs(
                    entity, segments);
        }

		float roundness = faceEditController.GetRoundRound();

        if (ImGui::InputFloat(
            "Round Inset roundness",
            &roundness,
            0.01f,
            0.1f))
        {
            faceEditController
                .SetRoundRound(
                    entity, roundness);
        }

        ImGui::Spacing();

        if (ImGui::Button(
            "Confirm",
            ImVec2(90.0f, 0.0f)))
        {
            faceEditController
                .ConfirmRoundInset(
                    entity
                );
        }

        ImGui::SameLine();

        if (ImGui::Button(
            "Cancel",
            ImVec2(90.0f, 0.0f)))
        {
            faceEditController
                .CancelRoundInset(
                    entity
                );
        }

        return;
    }


    // =================================================
    // LAST CONFIRMED INSET
    // =================================================

    if (entity.HasLastRoundInset())
    {
        ImGui::SeparatorText(
            "Last Operation"
        );

        ImGui::Text(
            "Round Inset"
        );

        RoundInsetModifierData lastRoundInset =
            entity.GetLastRoundInset();

        bool changed = false;


        // ---------------------------------------------
        // Amount
        // ---------------------------------------------

        if (ImGui::InputFloat(
            "Round Inset Amount",
            &lastRoundInset.insetAmount,
            0.01f,
            0.1f))
        {
            lastRoundInset.insetAmount =
                std::clamp(
                    lastRoundInset.insetAmount,
                    0.0f,
                    0.95f
                );

            changed = true;
        }


        // ---------------------------------------------
        // Segments
        // ---------------------------------------------

        if (ImGui::InputInt(
            "Round Inset Segments",
            &lastRoundInset.segments,
            1,
            1))
        {
            lastRoundInset.segments =
                std::clamp(
                    lastRoundInset.segments,
                    1,
                    16
                );

            changed = true;
        }


        // ---------------------------------------------
        // Roundness
        // ---------------------------------------------

        if (ImGui::InputFloat(
            "Round Inset Roundness",
            &lastRoundInset.roundness,
            0.01f,
            0.1f))
        {
            lastRoundInset.roundness =
                std::clamp(
                    lastRoundInset.roundness,
                    0.0f,
                    1.0f
                );

            changed = true;
        }


        // ---------------------------------------------
        // Rebuild last operation
        // ---------------------------------------------

        if (changed)
        {
            entity.UpdateLastRoundInset(
                lastRoundInset.insetAmount,
                lastRoundInset.segments,
                lastRoundInset.roundness
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

void ObjectExplorerPanel::MirrorControls(
    Entity& entity)
{
    auto& modifiers = entity.GetModifiers();

    // Find this object's mirror modifier.
    ModifierData* mirrorModifier = nullptr;

    for (ModifierData& modifier : modifiers)
    {
        if (modifier.type == ModifierType::Mirror)
        {
            mirrorModifier = &modifier;
            break;
        }
    }

    ImGui::SeparatorText("Mirror Selection");

    // Offer a way to add Mirror to this object.
    if (!mirrorModifier)
    {
        if (ImGui::Button("Add Mirror Selection"))
        {
            ModifierData modifier;
            modifier.type = ModifierType::Mirror;
            modifier.modEnabled = true;

            modifier.mirror.axis = ModifierAxis::X;
            modifier.mirror.tolerance = 0.001f;

            modifiers.push_back(modifier);

            BOX_LOG_INFO(
                "Added Mirror Selection to "
                << entity.GetName()
            );
        }

        return;
    }

    // ========================================
    // ENABLE / DISABLE
    // ========================================

    ImGui::Checkbox(
        "Enable Symmetry",
        &mirrorModifier->modEnabled
    );

    ImGui::Spacing();

    // ========================================
    // MIRROR AXIS
    // ========================================

    int selectedAxis = 0;

    switch (mirrorModifier->mirror.axis)
    {
    case ModifierAxis::Y:
        selectedAxis = 1;
        break;

    case ModifierAxis::Z:
        selectedAxis = 2;
        break;

    case ModifierAxis::X:
    default:
        selectedAxis = 0;
        break;
    }

    const char* axisItems[] = {
        "X", "Y", "Z"
    };

    ImGui::Text("Mirror Axis");

    if (ImGui::Combo(
        "##MirrorAxis",
        &selectedAxis,
        axisItems,
        IM_ARRAYSIZE(axisItems)))
    {
        switch (selectedAxis)
        {
        case 1:
            mirrorModifier->mirror.axis =
                ModifierAxis::Y;
            break;

        case 2:
            mirrorModifier->mirror.axis =
                ModifierAxis::Z;
            break;

        default:
            mirrorModifier->mirror.axis =
                ModifierAxis::X;
            break;
        }
    }

    // ========================================
    // MATCH TOLERANCE
    // ========================================

    ImGui::Spacing();
    ImGui::Text("Match Tolerance");

    if (ImGui::InputFloat(
        "##MirrorTolerance",
        &mirrorModifier->mirror.tolerance,
        0.0001f,
        0.001f,
        "%.4f"))
    {
        mirrorModifier->mirror.tolerance =
            std::clamp(
                mirrorModifier->mirror.tolerance,
                0.000001f,
                0.1f
            );
    }

    ImGui::TextDisabled(
        "Mirror plane: local axis = 0"
    );

    ImGui::Spacing();

    // ========================================
    // REMOVE MIRROR MODIFIER
    // ========================================

    if (ImGui::Button("Remove Mirror Selection"))
    {
        modifiers.erase(
            std::remove_if(
                modifiers.begin(),
                modifiers.end(),
                [](const ModifierData& modifier)
        {
            return modifier.type ==
                ModifierType::Mirror;
        }
            ),
            modifiers.end()
        );

        BOX_LOG_INFO(
            "Removed Mirror Selection from "
            << entity.GetName()
        );
    }
}


// =================================================
// PRECISE VERTEX MOVE CONTROLS
// =================================================


void ObjectExplorerPanel::VertexMoveControls(
    Entity& entity,
    VertexEditController& vertexEditController)
{
    const bool isMoving =
        vertexEditController.IsMoving();

    ImGui::SeparatorText(
        isMoving
        ? "Active Operation"
        : "Vertex Movement"
    );

    ImGui::Text(
        "Selected Vertices: %d",
        static_cast<int>(
            entity.GetSelectedVertices().size()
            )
    );

    ImGui::Spacing();

    // ============================================
    // AXIS SELECTION
    // ============================================

    const char* axisItems[] =
    {
        "None",
        "X",
        "Y",
        "Z",
        "Normal"
    };

    // During a move, display the active axis.
    if (isMoving)
    {
        switch (vertexEditController.GetMoveAxis())
        {
        case VertexMoveAxis::X:
            m_vertexMoveAxis = 1;
            break;

        case VertexMoveAxis::Y:
            m_vertexMoveAxis = 2;
            break;

        case VertexMoveAxis::Z:
            m_vertexMoveAxis = 3;
            break;

        case VertexMoveAxis::Normal:
            m_vertexMoveAxis = 4;
            break;

        default:
            m_vertexMoveAxis = 0;
            break;
        }
    }

    ImGui::BeginDisabled(isMoving);

    ImGui::Combo(
        "Move Axis",
        &m_vertexMoveAxis,
        axisItems,
        IM_ARRAYSIZE(axisItems)
    );

    ImGui::EndDisabled();

    ImGui::Spacing();

    // ============================================
    // START A NEW MOVE
    // ============================================

    if (!isMoving)
    {
        if (ImGui::Button(
            "Start Move",
            ImVec2(120.0f, 0.0f)))
        {
            VertexMoveAxis axis =
                VertexMoveAxis::None;

            switch (m_vertexMoveAxis)
            {
            case 1:
                axis = VertexMoveAxis::X;
                break;

            case 2:
                axis = VertexMoveAxis::Y;
                break;

            case 3:
                axis = VertexMoveAxis::Z;
                break;

            case 4:
                axis = VertexMoveAxis::Normal;
                break;

            default:
                break;
            }

            if (axis != VertexMoveAxis::None)
            {
                vertexEditController.StartMove(
                    entity,
                    axis
                );
            }
        }

        ImGui::TextDisabled(
            "Choose an axis and start moving."
        );

        return;
    }

    // ============================================
    // PRECISE MOVEMENT DISTANCE
    // ============================================

    float distance =
        vertexEditController.GetMoveDistance();

    if (ImGui::InputFloat(
        "Move Distance",
        &distance,
        0.01f,
        0.10f,
        "%.4f"))
    {
        vertexEditController.SetMoveDistance(
            entity,
            distance
        );
    }

    ImGui::TextDisabled(
        "Negative values move in reverse."
    );

    ImGui::Spacing();

    // ============================================
    // CONFIRM / CANCEL
    // ============================================

    if (ImGui::Button(
        "Confirm",
        ImVec2(90.0f, 0.0f)))
    {
        vertexEditController.ConfirmMove();
    }

    ImGui::SameLine();

    if (ImGui::Button(
        "Cancel",
        ImVec2(90.0f, 0.0f)))
    {
        vertexEditController.CancelMove(entity);
    }
}


void ObjectExplorerPanel::EdgeMoveControls(
    Entity& entity,
    EdgeEditController& edgeEditController)
{
    const bool isMoving =
        edgeEditController.IsMultiMoving();

    ImGui::SeparatorText(
        isMoving
        ? "Active Operation"
        : "Edge Movement"
    );

    ImGui::Text(
        "Selected Edges: %d",
        static_cast<int>(
            entity.GetSelectedEdges().size()
            )
    );

    ImGui::Spacing();

    // ============================================
    // AXIS SELECTION
    // ============================================

    const char* axisItems[] =
    {
        "None",
        "X",
        "Y",
        "Z",
        "Normal"
    };

    if (isMoving)
    {
        switch (edgeEditController.GetMultiMoveAxis())
        {
        case EdgeMoveAxis::X:
            m_edgeMoveAxis = 1;
            break;

        case EdgeMoveAxis::Y:
            m_edgeMoveAxis = 2;
            break;

        case EdgeMoveAxis::Z:
            m_edgeMoveAxis = 3;
            break;

        case EdgeMoveAxis::Normal:
            m_edgeMoveAxis = 4;
            break;

        default:
            m_edgeMoveAxis = 0;
            break;
        }
    }

    ImGui::BeginDisabled(isMoving);

    ImGui::Combo(
        "Move Axis",
        &m_edgeMoveAxis,
        axisItems,
        IM_ARRAYSIZE(axisItems)
    );

    ImGui::EndDisabled();

    ImGui::Spacing();

    // ============================================
    // START EDGE MOVE
    // ============================================

    if (!isMoving)
    {
        if (ImGui::Button(
            "Start Move",
            ImVec2(120.0f, 0.0f)))
        {
            EdgeMoveAxis axis =
                EdgeMoveAxis::None;

            switch (m_edgeMoveAxis)
            {
            case 1:
                axis = EdgeMoveAxis::X;
                break;

            case 2:
                axis = EdgeMoveAxis::Y;
                break;

            case 3:
                axis = EdgeMoveAxis::Z;
                break;

            case 4:
                axis = EdgeMoveAxis::Normal;
                break;

            default:
                break;
            }

            if (axis != EdgeMoveAxis::None)
            {
                edgeEditController.StartMultiMove(
                    entity,
                    axis
                );
            }
        }

        ImGui::TextDisabled(
            "Choose an axis and start moving."
        );

        return;
    }

    // ============================================
    // PRECISE MOVEMENT DISTANCE
    // ============================================

    float distance =
        edgeEditController.GetMultiMoveDistance();

    if (ImGui::InputFloat(
        "Move Distance##Edge",
        &distance,
        0.01f,
        0.10f,
        "%.4f"))
    {
        edgeEditController.SetMultiMoveDistance(
            entity,
            distance
        );
    }

    ImGui::TextDisabled(
        "Negative values move in reverse."
    );

    ImGui::Spacing();

    // ============================================
    // CONFIRM / CANCEL
    // ============================================

    if (ImGui::Button(
        "Confirm##EdgeMove",
        ImVec2(90.0f, 0.0f)))
    {
        edgeEditController.ConfirmMultiMove();
    }

    ImGui::SameLine();

    if (ImGui::Button(
        "Cancel##EdgeMove",
        ImVec2(90.0f, 0.0f)))
    {
        edgeEditController.CancelMultiMove(
            entity
        );
    }
}
