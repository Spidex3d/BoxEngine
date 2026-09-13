#pragma once
//#include <cstddef>
#include <vector>


class MeshEditing;


class FaceFill
{
public:
    bool Use(MeshEditing& mesh, const std::vector<std::size_t>& vertices);
};