#pragma once
#include "imgui\imgui.h"
#include <functional>
#include <string>
#include <UI\EditorTexture.h>
#include <glm/glm.hpp>

#include "tools/TransformToolController.h"
#include <tools/VertexEditController.h> // Add this at the top if not already included

class BoxEngine;
class EditorIcons;


enum class ViewportAction
{
    None,

    SetObjectMode,
    SetEditMode,
	SetSelectMode,

	vertexEditMode,
	edgeEditMode,
	faceEditMode,

	AddMbxModel,
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

	int m_EditMode = 0; // 1for object mode ans 2 for edit mode
    int m_editType = 0;

    // ################################## tools ###############################	    
    TransformTools m_transformTools;

	// Add this member to fix the error ######### Edit mode mesh editing controller ##############
    

    VertexEditController m_vertexEditController;

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
