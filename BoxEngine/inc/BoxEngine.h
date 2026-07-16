#pragma once
#include <rendering\Framebuffer.h>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers



class BoxEngine
{
public:
	BoxEngine() = default;
	~BoxEngine() = default;
	
	void testFunction();
	
	bool Initialize();

	void ResizeSceneViewport(int width, int height);
	void RenderScene();

	GLuint GetSceneTexture() const;
private:
	Framebuffer m_sceneFramebuffer;
	


};