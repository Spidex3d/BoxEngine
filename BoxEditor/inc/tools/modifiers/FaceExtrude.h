#pragma once
#include <cstddef>

class Entity;

//  FaceExtrude
//  - duplicate vertices
//  - create new face
//  - create side faces
//  - update topology

class FaceExtrude
{
public:
public:
    //bool Execute(Entity& entity, std::size_t faceIndex, float distance);
    bool Use(Entity& entity, std::size_t faceIndex, float distance);


private:


};
