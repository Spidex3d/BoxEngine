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
class EditorInput;

class App
{
public:
	App();
	~App();

	bool Init(); // Initialize the window and ImGui context
	int Run();        // set the main loop
	void Shutdown();

private:
	float m_deltaTime = 0.0f;
	float m_lastFrameTime = 0.0f;

	

	void HandleMenuAction(MenuAction action);		// Handle actions from the main menu bar
	void HandleViewportAction(ViewportAction action, BoxEngine& engine); // Handle actions from the main menu and viewport panels
	void HandleInput();							// Handle input events
	bool m_isRunning = false;						// Flag to indicate if the application is running
	std::unique_ptr<BoxWindow> m_window;			// BoxWindow instance for managing the application window
	std::unique_ptr<EditorIcons> m_editorIcons;		// EditorIcons instance for managing editor icons
	std::unique_ptr<EditorInput> m_input;		// EditorInput instance for managing input

	std::unique_ptr<BoxEngine> m_engine;			// BoxEngine instance for rendering and scene management

	// Imgui Initialize panels for the main window
	std::unique_ptr<ImGuiLayer> m_imgui;
	std::unique_ptr<MainMenuBar> m_imgMenu;			// menu bar panel for ImGui
	std::unique_ptr<SceneViewportPanel> m_imgScene; // Scene panel for ImGui
	std::unique_ptr<SceneCollectionPanel> m_imgSceneCollection; // Scene Collection for ImGui
	std::unique_ptr<SceneViewportPanel> m_sceneViewport; // Scene Viewport panel for ImGui
};
