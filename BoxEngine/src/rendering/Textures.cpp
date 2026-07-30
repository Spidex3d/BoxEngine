#include "rendering\Textures.h"
#include <stb/stb_image.h>

Texture::~Texture()
{
    if (m_id != 0)
    {
        glDeleteTextures(
            1,
            &m_id
        );
    }
}

bool Texture::LoadFromFile(
    const std::string& filePath)
{
    // Delete an existing texture if this
    // object is being reused.
    if (m_id != 0)
    {
        glDeleteTextures(
            1,
            &m_id
        );

        m_id = 0;
    }

    int width = 0;
    int height = 0;
    int channels = 0;

    stbi_set_flip_vertically_on_load(true);

    unsigned char* pixels =
        stbi_load(
            filePath.c_str(),
            &width,
            &height,
            &channels,
            0
        );

    if (!pixels)
    {
        return false;
    }

    GLenum dataFormat = GL_RGB;
    GLenum internalFormat = GL_RGB8;

    if (channels == 1)
    {
        dataFormat = GL_RED;
        internalFormat = GL_R8;
    }
    else if (channels == 3)
    {
        dataFormat = GL_RGB;
        internalFormat = GL_RGB8;
    }
    else if (channels == 4)
    {
        dataFormat = GL_RGBA;
        internalFormat = GL_RGBA8;
    }
    else
    {
        stbi_image_free(pixels);
        return false;
    }

    glGenTextures(
        1,
        &m_id
    );

    glBindTexture(
        GL_TEXTURE_2D,
        m_id
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_REPEAT
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_REPEAT
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR_MIPMAP_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
    );

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        internalFormat,
        width,
        height,
        0,
        dataFormat,
        GL_UNSIGNED_BYTE,
        pixels
    );

    glGenerateMipmap(
        GL_TEXTURE_2D
    );

    stbi_image_free(pixels);

    glBindTexture(
        GL_TEXTURE_2D,
        0
    );

    m_width = width;
    m_height = height;
    m_channels = channels;

    return true;
}

void Texture::Bind(
    unsigned int slot) const
{
    if (m_id == 0 || slot > 31)
    {
        return;
    }

    glActiveTexture(
        GL_TEXTURE0 + slot
    );

    glBindTexture(
        GL_TEXTURE_2D,
        m_id
    );
}

void Texture::Unbind() const
{
    glBindTexture(
        GL_TEXTURE_2D,
        0
    );
}

//bool Texture::LoadFromFile(const std::string& filePath)
//{
//
//    m_id = 0;
//
//    glGenTextures(1, &m_id);
//
//    glBindTexture(
//        GL_TEXTURE_2D,
//        m_id
//    );
//
//   
//    glTexParameteri(
//        GL_TEXTURE_2D,
//        GL_TEXTURE_WRAP_S,
//        GL_REPEAT
//    );
//
//    glTexParameteri(
//        GL_TEXTURE_2D,
//        GL_TEXTURE_WRAP_T,
//        GL_REPEAT
//    );
//
//    glTexParameteri(
//        GL_TEXTURE_2D,
//        GL_TEXTURE_MIN_FILTER,
//        GL_LINEAR_MIPMAP_LINEAR
//    );
//
//    glTexParameteri(
//        GL_TEXTURE_2D,
//        GL_TEXTURE_MAG_FILTER,
//        GL_LINEAR
//    );
//
//    int width = 0;
//    int height = 0;
//    int channels = 0;
//
//    stbi_set_flip_vertically_on_load(true);
//
//    unsigned char* pixels =
//        stbi_load(
//            filePath.c_str(),
//            &width,
//            &height,
//            &channels,
//            0
//        );
//
//    if (!pixels)
//    {
//        glDeleteTextures(
//            1,
//            &m_id
//        );
//
//        return false;
//    }
//
//    GLenum format = GL_RGB;
//
//    if (channels == 1)
//    {
//        format = GL_RED;
//    }
//    else if (channels == 3)
//    {
//		format = GL_RGB; // jpg, bmp, tga
//    }
//    else if (channels == 4)
//    {
//		format = GL_RGBA; // png
//    }
//
//    glTexImage2D(
//        GL_TEXTURE_2D,
//        0,
//        format,
//        width,
//        height,
//        0,
//        format,
//        GL_UNSIGNED_BYTE,
//        pixels
//    );
//
//    glGenerateMipmap(
//        GL_TEXTURE_2D
//    );
//
//    stbi_image_free(pixels);
//
//    glBindTexture(
//        GL_TEXTURE_2D,
//        0
//    );
//
//    return true;
//}
//
//void Texture::Bind(unsigned int slot) const
//{
//    if (m_id == 0)
//        return;
//
//    if (slot > 31)
//        return;
//
//    glActiveTexture(GL_TEXTURE0 + slot);
//
//    glBindTexture(
//        GL_TEXTURE_2D,
//        m_id
//    );
//}

//void Texture::Unbind() const
//{
//    if (m_id != 0)
//    {
//        glBindTexture(GL_TEXTURE_2D, 0);
//	}
//}