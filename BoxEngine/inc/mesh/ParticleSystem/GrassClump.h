#pragma once
#include <glm\glm.hpp>

class GrassBlade;
class Shader;

class GrassClump
{
public:
    GrassClump() = default;
    ~GrassClump() = default;

    void Draw(const GrassBlade& grassBlade, Shader& shader,
        const glm::mat4& model) const;

private:
    // Individual blade transformations
};