#pragma once

#include <cstddef>
#include <glm/glm.hpp>

class Entity;
class MeshEditing;

class FaceInset
{
public:

    bool Use(MeshEditing& mesh, std::size_t faceIndex, float amount);

private:


};
