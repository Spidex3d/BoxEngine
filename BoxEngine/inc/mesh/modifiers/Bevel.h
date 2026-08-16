#pragma once
#include <cstddef>
#include <glm/glm.hpp>

class Entity;
class MeshEditing;

class Bevel
{

public:

    bool Use(MeshEditing& mesh, std::size_t edgeIndex, float width, int segments, float profile);

};