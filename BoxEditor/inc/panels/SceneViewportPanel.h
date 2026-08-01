#pragma once
#include "imgui\imgui.h"
#include <functional>
#include <string>
#include <UI\EditorTexture.h>
#include <glm/glm.hpp>

class BoxEngine;
class EditorIcons;

enum class ViewportAction
{
    None,

    SetObjectMode,
    SetEditMode,

	vertexEditMode,
	edgeEditMode,
	faceEditMode,

    AddObjModel,
    AddGltfModel,

    AddEditableCube,
    AddEditablePlane,
    AddEditableSphere,
    AddEditableICOSphere,
    AddEditableCylinder,
    AddEditableCone,
    AddEditableTorus,

    AddSpotLight,
    AddAreaLight,

    AddSkyBox,
    AddSkySphere,

    ResetCamera
};

enum class TransformAxis
{
    None,
    X,
    Y,
    Z
};

class SceneViewportPanel
{

public:

	ViewportAction DrawSceneViewport(BoxEngine& engine, const EditorIcons& icons);

    ImVec2 GetViewportPosition() const
    {
        return m_sceneViewportPos;
    }

    ImVec2 GetViewportSize() const
    {
        return m_sceneViewportSize;
    }	
	

	void Shutdown();

private:
    // Converts mouse coordinates to a ray in world space for piking
    glm::vec3 CreateMouseRay(float mouseX, float mouseY, float viewportWidth, float viewportHeight, const glm::mat4& view,
        const glm::mat4& projection) const;

	int m_EditMode = 0;
    int m_editType = 0;

    // ################################## tools ###############################
	int m_selectedEditToolType = -1;
    // Interactive transform state.
    bool m_isTransforming = false;

    TransformAxis m_transformAxis = TransformAxis::None;

    glm::vec3 m_transformStartPosition = glm::vec3(0.0f);

    ImVec2 m_transformStartMouse = ImVec2(0.0f, 0.0f);

    // World units moved for each screen pixel. NEEDS PUTING SETTINGS LATER
    float m_moveSensitivity = 0.01f;

    void BeginMoveTransform(BoxEngine& engine, TransformAxis axis);

    void UpdateMoveTransform(BoxEngine& engine);

    void ConfirmTransform();

    void CancelTransform(
        BoxEngine& engine
    );

	// end tools

	int m_AddMeshType = 0;
    		
	ImVec2 m_sceneViewportPos = ImVec2(0, 0);
	ImVec2 m_sceneViewportSize = ImVec2(0, 0);
	// ################################## Transform Tool Bar Colors ###############################
    void TransformToolBarColors()
    {
        const ImVec2 min = ImGui::GetItemRectMin();
        const ImVec2 max = ImGui::GetItemRectMax();
        ImGui::GetWindowDrawList()->AddRect(min, max, IM_COL32(50, 150, 255, 255), 4.0f, 0, 2.0f);
    }

};
