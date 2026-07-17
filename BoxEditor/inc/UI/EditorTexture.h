#pragma once
#include <string>
#include <glad/glad.h>

struct EditorTexture
{
    GLuint id = 0;
    int width = 0;
    int height = 0;

    bool IsValid() const
    {
        return id != 0;
    }
};




    EditorTexture LoadEditorTexture(const std::string& relativePath);
    void DestroyEditorTexture(EditorTexture& texture);  


