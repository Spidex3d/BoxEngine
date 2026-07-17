#include <UI/EditorTexture.h>
#include <miniBoxLog.h>
#include <Helpers.h>
#include <stb/stb_image.h>


EditorTexture LoadEditorTexture(const std::string& relativePath)
{
    EditorTexture result;

    Helpers helpers;
    const std::string fullPath =
        helpers.GetAssetPath(relativePath);

    stbi_set_flip_vertically_on_load(false);

    int channels = 0;

    unsigned char* pixels = stbi_load(
        fullPath.c_str(),
        &result.width,
        &result.height,
        &channels,
        4
    );

    if (!pixels)
    {
        BOX_LOG_ERROR(
            "Failed to load editor texture: "
            << fullPath
        );

        return result;
    }

    glGenTextures(1, &result.id);
    glBindTexture(GL_TEXTURE_2D, result.id);

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_CLAMP_TO_EDGE
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_CLAMP_TO_EDGE
    );

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        result.width,
        result.height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixels
    );

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(pixels);

    BOX_LOG_INFO(
        "Loaded editor texture: "
        << fullPath
        << " ID=" << result.id
    );

    return result;
}

void DestroyEditorTexture(EditorTexture& texture)
{
    if (texture.id != 0)
    {
        glDeleteTextures(1, &texture.id);
    }

    texture.id = 0;
    texture.width = 0;
    texture.height = 0;
}
