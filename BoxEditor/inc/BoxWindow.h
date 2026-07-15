#pragma once
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
	~BoxWindow() = default;

	bool Initialize(int width, int height, const char* title);
	void Update();
	void Shutdown();
	// Getters
	GLFWwindow* GetWindow() const { return window; }



private:

	GLFWwindow* window = nullptr;


};