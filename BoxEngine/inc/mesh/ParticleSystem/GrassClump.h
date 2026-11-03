#pragma once
#include <glm\glm.hpp>
#include <vector>

class GrassBlade;
class Shader;

class GrassClump
{
public:
    GrassClump() = default;
    ~GrassClump() = default;

    void Draw(const GrassBlade& grassBlade, Shader& shader,
        const glm::mat4& model) const;

    // NEW: Generate blade transformations for instancing
    void AppendInstanceMatrices(const glm::mat4& clumpModel,
        std::vector<glm::mat4>& matrices) const;

private:
    // Individual blade transformations
};