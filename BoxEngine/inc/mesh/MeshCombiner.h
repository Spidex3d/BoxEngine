#pragma once
#include <mesh/MeshEditing.h>



class Entity;

class MeshCombiner
{
public:

    static bool Combine(
        const Entity& entityA,
        const Entity& entityB,
        MeshEditing& outMesh
    );
};
