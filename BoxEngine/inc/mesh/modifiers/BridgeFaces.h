#pragma once
#include <cstddef>
#include <glm/glm.hpp>

class Entity;
class MeshEditing;

class BridgeFaces {


	bool Use(MeshEditing& mesh, 
		std::size_t faceIndexA,
		std::size_t faceIndexB
	);

};
