#pragma once
#include <cstddef>

// naming convention for each class
// FaceExtrude, LoopCut, BevelEdge, InsetFace, BridgeFace, Subdivide, WeldVertex, Smooth, Mirror, Solidify, Array
// ExtrudeUse, LoopUse, BevelUse, InsetUse, BridgeUse, SubdivideUse, WeldUse, SmoothUse, MirrorUse, SolidifyUse, ArrayUse 

class Entity;


class FaceExtrude
{
public:

    bool ExtrudeUse(Entity& entity, std::size_t faceIndex, float distance);

private:


};
