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
    AddEditableUVSphere,
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

	int m_EditMode = 0;
    int m_editType = 0;
    		
	ImVec2 m_sceneViewportPos = ImVec2(0, 0);
	ImVec2 m_sceneViewportSize = ImVec2(0, 0);

};
