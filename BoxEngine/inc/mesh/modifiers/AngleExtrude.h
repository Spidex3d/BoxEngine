#pragma once
#include <cstddef>
#include <glm/glm.hpp>
#include <imgui\imgui.h>

class Entity;
class MeshEditing;

enum class AngleExtrudeDirection
{
    Positive,
    Negative
};

enum class AngleExtrudeRotationAxis
{
    Tangent,
    Bitangent
};



struct AngleExtrudeSettings
{
    // For this first version this acts as the bend radius.
    float distance = 0.25f;

    // Positive / negative controls bend direction.
    float angleDegrees = 90.0f;

    // Number of sections making the bend.
    int segments = 4;

    
	AngleExtrudeDirection direction = AngleExtrudeDirection::Positive;
    AngleExtrudeRotationAxis rotationAxis = AngleExtrudeRotationAxis::Bitangent;
};


class AngleExtrude
{
public:

	bool Use(MeshEditing& mesh, std::size_t faceIndex, const AngleExtrudeSettings& settings);
	
    


private:

   

};


