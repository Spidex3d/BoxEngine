#pragma once
#include <memory>
#include <ImGuiLayer.h>
#include "panels\MainMenuBar.h"

class BoxWindow;
//class ImGuiLayer;
//class MainMenuBar;

class App
{
public:
	App() = default;
	~App() = default;

	bool Init(); // Initialize the window and ImGui context
	int Run();        // set the main loop
	void Shutdown();

private:
		
	bool m_isRunning = false;
	std::unique_ptr<BoxWindow> m_window;
	std::unique_ptr<ImGuiLayer> m_imgui;
	std::unique_ptr<MainMenuBar> m_imgMenu; // menu bar panel for ImGui
};
