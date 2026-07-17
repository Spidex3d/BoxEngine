
#include <rendering\Framebuffer.h>
#include <miniBoxLog.h>

Framebuffer::~Framebuffer()
{
    Destroy();
}

bool Framebuffer::Create(int width, int height)
{
    if (width <= 0 || height <= 0)
        return false;

    Destroy();

    m_width = width;
    m_height = height;

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glGenTextures(1, &m_colorTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        m_colorTexture,
        0
    );

    glGenRenderbuffers(1, &m_depthStencil);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthStencil);

    glRenderbufferStorage(
        GL_RENDERBUFFER,
        GL_DEPTH24_STENCIL8,
        m_width,
        m_height
    );

    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT,
        GL_RENDERBUFFER,
        m_depthStencil
    );

    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        BOX_LOG_ERROR(
            "Framebuffer incomplete. Status: "
            << static_cast<unsigned int>(status)
        );

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        Destroy();
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    BOX_LOG_INFO(
        "Framebuffer created: "
        << m_width << " x " << m_height
    );

    return true;
}

bool Framebuffer::Resize(int width, int height)
{
    if (width <= 0 || height <= 0)
        return false;

    if (m_fbo != 0 && width == m_width && height == m_height)
        return true;

    return Create(width, height);
}

void Framebuffer::Bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
}

void Framebuffer::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void Framebuffer::Destroy()
{
    if (m_depthStencil != 0)
    {
        glDeleteRenderbuffers(1, &m_depthStencil);
        m_depthStencil = 0;
    }

    if (m_colorTexture != 0)
    {
        glDeleteTextures(1, &m_colorTexture);
        m_colorTexture = 0;
    }

    if (m_fbo != 0)
    {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }

    m_width = 0;
    m_height = 0;
}