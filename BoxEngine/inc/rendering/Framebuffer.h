#pragma once
#include <glad/glad.h>
class Framebuffer
{
public:
    Framebuffer() = default;
    ~Framebuffer();

    bool Create(int width, int height);
    bool Resize(int width, int height);

    void Bind() const;
    static void Unbind();

    void Destroy();

    GLuint GetColorTexture() const
    {
        return m_colorTexture;
    }

    int GetWidth() const
    {
        return m_width;
    }

    int GetHeight() const
    {
        return m_height;
    }

    bool IsValid() const
    {
        return m_fbo != 0;
    }


private:
    GLuint m_fbo = 0;
    GLuint m_colorTexture = 0;
    GLuint m_depthStencil = 0;

    int m_width = 0;
    int m_height = 0;
};
