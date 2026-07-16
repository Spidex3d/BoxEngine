#pragma once
#include "imgui\imgui.h"
#include <functional>
#include <string>

class SceneViewportPanel
{
public:
	using ActionCallback = std::function<void(const std::string&)>; // called when UI requests an action


	void DrawSceneViewport(); // dedicated panel classes once they grow.

private:
	// toolbar height in UI units
	int m_EditMode = 0;

	ActionCallback m_actionCallback = nullptr;
		
	ImVec2 m_sceneViewportPos = ImVec2(0, 0);
	ImVec2 m_sceneViewportSize = ImVec2(0, 0);

};
