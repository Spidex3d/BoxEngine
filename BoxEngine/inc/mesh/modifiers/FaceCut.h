#pragma once
#include <cstddef>
#include <glm/glm.hpp>

class Entity;
class MeshEditing;

class FaceCut
{

public:

    bool Use(MeshEditing& mesh, std::size_t edgeIndex, float amount);



};