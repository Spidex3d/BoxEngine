#pragma once

class App
{
public:
	App() = default;
	~App() = default;

	bool Initialize(); // Initialize the window and ImGui context
	bool Run();        // set the main loop
	void Shutdown();

private:

	bool m_isRunning = false;
};
