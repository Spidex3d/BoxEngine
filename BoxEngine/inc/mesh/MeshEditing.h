#pragma once
#include <glm/glm.hpp>
#include <cstddef>
#include <vector>   
#include "MeshData.h"

// this will be dedicated to editing of the mesh data, such as adding vertices, edges, faces, and other mesh editing operations.
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
    MeshEditing() = default;

    void Clear();

    bool CreateCube();
    bool CreatePlane();
	bool CreatePyramid();

    std::size_t GetVertexCount() const;
    std::size_t GetEdgeCount() const;
    std::size_t GetFaceCount() const;

    EditVertex& GetVertex(std::size_t index);
    const EditVertex& GetVertex(std::size_t index) const;

    EditEdge& GetEdge(std::size_t index);
    const EditEdge& GetEdge(std::size_t index) const;

    EditFace& GetFace(std::size_t index);
    const EditFace& GetFace(std::size_t index) const;
	// ########################################## Mesh Editing add vertex, face,  #########################################
    std::size_t AddVertex(const glm::vec3& position);

    std::size_t AddFace(const std::vector<std::size_t>& vertices);

    bool SetFace(std::size_t faceIndex, const std::vector<std::size_t>& vertices);

    void RebuildEdges();

	// Add an edge between two vertices, if it doesn't already exist. Returns the index of the edge.
	// loop cut will use this to add edges between vertices.
    std::size_t AddEdge(std::size_t vertexA, std::size_t vertexB);

    bool BuildRenderMesh(MeshData& meshData) const;

    const std::vector<EditVertex>& GetVertices() const;
    const std::vector<EditEdge>& GetEdges() const;
    const std::vector<EditFace>& GetFaces() const;

private:
    std::vector<EditVertex> m_vertices;
    std::vector<EditEdge> m_edges;
    std::vector<EditFace> m_faces;

};
