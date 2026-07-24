#pragma once
#include <memory>
#include <rendering/Framebuffer.h>
#include <shader/Shader.h>

class Material;

class MaterialPreview
{
public:
    bool Initialize(int width, int height);

    void Render(const Material& material);

    void Draw(const Material& material);

    void Shutdown();

private:
    Framebuffer m_framebuffer;

    std::unique_ptr<Shader> m_shader;
   // std::unique_ptr<Mesh> m_sphereMesh;

    int m_width = 200;
    int m_height = 200;
};
