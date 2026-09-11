#pragma once


#include <cstddef>

class MeshEditing;

class FaceRemove
{
public:
    bool Use(MeshEditing& mesh, std::size_t faceIndex);

};


