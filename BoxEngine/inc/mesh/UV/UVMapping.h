#pragma once

class MeshEditing;

class UVMapping
{
public:

    // Generate UV coordinates using planar projection.
    static bool Planar(MeshEditing& mesh);

    // Generate UV coordinates using box projection.
    static bool Box(MeshEditing& mesh);

    // Generate UV coordinates using spherical projection.
    static bool Spherical(MeshEditing& mesh);
};
