#pragma once
#include <glm/glm.hpp>

class Entity;
class MeshEditing;


struct AngleExtrudeSettings
{
	float distance = 0.25f;

	float angleDegrees = 90.0f;

	int segments = 4;

	glm::vec3 extrusionAxis =
		glm::vec3(1.0f, 0.0f, 0.0f);

	glm::vec3 rotationAxis =
		glm::vec3(0.0f, 0.0f, 1.0f);
};

class AngleExtrude
{
public:

	bool Use(MeshEditing& mesh, std::size_t faceIndex, const AngleExtrudeSettings& settings);
	

};


