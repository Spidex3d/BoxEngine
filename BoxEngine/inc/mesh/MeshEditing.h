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

    std::size_t materialIndex = 0;
};

class MeshEditing
{
public:
    MeshEditing() = default;

    void Clear();

    bool CreateCube();
    bool CreatePlane();
	
	bool CreatePyramid();
	bool CreateSphere(int sectors = 32, int stacks = 16);
	bool CreateIcoSphere(int recursionLevel = 2);
    bool CreateCylinder(int sectors = 32, int stacks = 1, float radius = 0.5f, float height = 1.0f);
	bool CreateCone(int sectors = 32, float radius = 0.5f, float height = 1.0f);
	bool CreateTorus(int sides = 16, int rings = 32, float innerRadius = 0.2f, float outerRadius = 0.5f);


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

	
    bool BuildRenderMesh(MeshData& meshData) const;

    const std::vector<EditVertex>& GetVertices() const;
    const std::vector<EditEdge>& GetEdges() const;
    const std::vector<EditFace>& GetFaces() const;

    
	std::vector<std::size_t>FindEdgeRing(std::size_t startEdgeIndex) const; // Finds an edge loop starting from a given edge index.
	std::vector<std::size_t>FindEdgeLoop(std::size_t startEdgeIndex) const; // Finds an edge loop starting from a given edge index.

private:
    float pi = 3.14159265358979323846f;

    std::vector<EditVertex> m_vertices;
    std::vector<EditEdge> m_edges;
    std::vector<EditFace> m_faces;

	std::size_t FindEdgeIndex(std::size_t vertexA, std::size_t vertexB) const; // Finds the index of an edge given its two vertex indices. Returns -1 if not found.

	bool FaceContainsEdge(const EditFace& face, std::size_t vertexA, std::size_t vertexB) const; // Checks if a given face contains an edge defined by two vertex indices.

};
