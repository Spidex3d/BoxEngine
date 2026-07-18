#pragma once
#include <memory>
#include <ImGuiLayer.h>
#include "panels\MainMenuBar.h"
#include "panels\SceneViewportPanel.h"
#include "panels\SceneCollectionPanel.h"
#include <UI/EditorIcons.h>

class BoxWindow;
class EditorIcons;
class BoxEngine;
class App
{
public:
	App();
	~App();

	bool Init(); // Initialize the window and ImGui context
	int Run();        // set the main loop
	void Shutdown();

private:
	void HandleMenuAction(MenuAction action);
	void HandleViewportAction(ViewportAction action, BoxEngine& engine);
	bool m_isRunning = false;
	std::unique_ptr<BoxWindow> m_window;
	std::unique_ptr<EditorIcons> m_editorIcons;

	std::unique_ptr<BoxEngine> m_engine;

	// Imgui Initialize panels for the main window
	std::unique_ptr<ImGuiLayer> m_imgui;
	std::unique_ptr<MainMenuBar> m_imgMenu; // menu bar panel for ImGui
	std::unique_ptr<SceneViewportPanel> m_imgScene; // Scene panel for ImGui
	std::unique_ptr<SceneCollectionPanel> m_imgSceneCollection; // Scene Collection for ImGui
	std::unique_ptr<SceneViewportPanel> m_sceneViewport;
};
