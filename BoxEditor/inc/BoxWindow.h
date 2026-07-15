#pragma once
#include <glad\glad.h>
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h> 

struct WindowConfig {
	
	int width = 1280;
	int height = 720;
	const char* title = "Box Editor";
	bool resizable = true;
	bool vsync = true;
};

class BoxWindow
{
public:

	explicit BoxWindow(const WindowConfig& config);
	~BoxWindow();

	void SetIcon(GLFWwindow* window); // set window icon from image file
	

	void Update();
	void Shutdown();

	GLFWwindow* GetWindow() const { return window; }
	bool ShouldClose() const;

	void SetVSync(bool enabled);


private:

	GLFWwindow* window = nullptr;
	WindowConfig m_config;

	// Simple refcount so multiple SpxWindow instances don't re-init/terminate GLFW
	static int s_glfwRefCount; // static member declaration
};