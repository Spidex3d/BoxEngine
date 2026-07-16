#pragma once
#include <BoxWindow.h>
#include "imgui\imgui.h"
#include <functional>
#include <string>




class SceneViewportPanel
{
public:
	using ActionCallback = std::function<void(const std::string&)>; // called when UI requests an action
	using RenderCallback = std::function<void()>; // called while FBO is bound so Engine can render into it


	void DrawSceneViewport(); // dedicated panel classes once they grow.

private:
	// toolbar height in UI units
	int m_EditMode = 0;

	ActionCallback m_actionCallback = nullptr;
	RenderCallback m_renderCallback = nullptr;

	GLuint m_fbo = 0;
	GLuint m_fboColor = 0; // color texture
	GLuint m_fboDepth = 0;
	int m_fbWidth = 0;
	int m_fbHeight = 0;

	ImVec2 m_sceneViewportPos = ImVec2(0, 0);
	ImVec2 m_sceneViewportSize = ImVec2(0, 0);

};
