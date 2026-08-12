#pragma once
#include <cstddef>
#include <glm/glm.hpp>
// naming convention for each class
// FaceExtrude, LoopCut, BevelEdge, InsetFace, BridgeFace, Subdivide, WeldVertex, Smooth, Mirror, Solidify, Array
// ExtrudeUse, LoopUse, BevelUse, InsetUse, BridgeUse, SubdivideUse, WeldUse, SmoothUse, MirrorUse, SolidifyUse, ArrayUse 

class Entity;
class MeshEditing;

class FaceExtrude
{
public:

   // bool Use(Entity& entity, std::size_t faceIndex, float distance);

    bool Use(MeshEditing& mesh, std::size_t faceIndex, const glm::vec3& direction, float distance);

private:


};
