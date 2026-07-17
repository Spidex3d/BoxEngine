#pragma once
#include "imgui\imgui.h"
#include <functional>
#include <string>

class BoxEngine;

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
	
	ViewportAction DrawSceneViewport(BoxEngine& engine);

    ImVec2 GetViewportPosition() const
    {
        return m_sceneViewportPos;
    }

    ImVec2 GetViewportSize() const
    {
        return m_sceneViewportSize;
    }	

private:
	
	int m_EditMode = 0;
    		
	ImVec2 m_sceneViewportPos = ImVec2(0, 0);
	ImVec2 m_sceneViewportSize = ImVec2(0, 0);

};
