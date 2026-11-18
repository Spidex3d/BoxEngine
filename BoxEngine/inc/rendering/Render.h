#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <filesystem>
#include <string>
#include <vector>

// BoxRenderer v0.1. 18/11/2026

class Render
{
public:

    bool Initialize(int width, int height);

    void StartRender();
    void Update();
    void CancelRender();

    bool IsRendering() const;
    float GetProgress() const;

    GLuint GetRenderTexture() const;

    void Destroy();
};