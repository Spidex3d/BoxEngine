#pragma once
#include <cstddef>
#include <glm/glm.hpp>
#include <vector>
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


    bool UseMultiple(MeshEditing& mesh, const std::vector<std::size_t>& faceIndices,
        const std::vector<glm::vec3>& directions, float distance);

    bool UseRegion(MeshEditing& mesh, const std::vector<std::size_t>& faceIndices,
        const std::vector<glm::vec3>& directions, float distance);


private:


};
