#pragma once

#include <cstddef>
#include <glm/glm.hpp>
// naming convention for each class
// FaceExtrude, LoopCut, BevelEdge, FaceInset, BridgeFace, Subdivide, WeldVertex, Smooth, Mirror, Solidify, Array
// ExtrudeUse, LoopUse, BevelUse, InsetUse, BridgeUse, SubdivideUse, WeldUse, SmoothUse, MirrorUse, SolidifyUse, ArrayUse 

class Entity;
class MeshEditing;

class FaceInset
{
public:

    bool Use(MeshEditing& mesh, std::size_t faceIndex, float amount);

private:


};
