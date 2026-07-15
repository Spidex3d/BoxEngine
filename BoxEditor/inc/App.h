#pragma once

#include <memory>

class BoxWindow;


class App
{
public:
	App() = default;
	~App() = default;

	bool Init(); // Initialize the window and ImGui context
	bool Run();        // set the main loop
	void Shutdown();

private:
		
	bool m_isRunning = false;
	std::unique_ptr<BoxWindow> m_window;
};
