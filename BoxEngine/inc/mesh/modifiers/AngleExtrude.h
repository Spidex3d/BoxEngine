#pragma once
#include <cstddef>
#include <glm/glm.hpp>

class Entity;
class MeshEditing;


struct AngleExtrudeSettings
{
    // For this first version this acts as the bend radius.
    float distance = 0.25f;

    // Positive / negative controls bend direction.
    float angleDegrees = 90.0f;

    // Number of sections making the bend.
    int segments = 4;

    // Initial direction of travel.
    glm::vec3 extrusionAxis = glm::vec3(1.0f, 0.0f, 0.0f);

    // Axis that the bend rotates around.
    glm::vec3 rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
};

class AngleExtrude
{
public:

	bool Use(MeshEditing& mesh, std::size_t faceIndex, const AngleExtrudeSettings& settings);
	

};


