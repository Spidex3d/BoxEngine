#pragma once
#include <glm/glm.hpp>
#include <cstddef>
#include <vector>   

// this will be a dedicated to editing of the mesh data, such as adding vertices, edges, faces, and other mesh editing operations.


struct EditVertex
{
    glm::vec3 position{ 0.0f };
};

struct EditEdge
{
    std::size_t vertexA = 0;
    std::size_t vertexB = 0;
};

struct EditFace
{
    std::vector<std::size_t> vertices;
};


class MeshEditing
{
public:
    std::vector<EditVertex> vertices;
    std::vector<EditEdge> edges;
    std::vector<EditFace> faces;


private:

};