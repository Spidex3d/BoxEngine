#pragma once
#include <cstddef>
#include <glm/glm.hpp>


class Entity;
class MeshEditing;

class RoundInset
{
public:

   
	   bool Use(MeshEditing& mesh, std::size_t faceIndex, float amount, int segments, float roundness);

private:


};

