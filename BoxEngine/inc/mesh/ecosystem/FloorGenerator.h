#pragma once
//#include <vector>
//#include <glm/glm.hpp>
//#include <mesh/MeshData.h>
//
//
//struct FloorVertex
//{
//    glm::vec3 position;
//};
//
//
//struct FloorFace
//{
//    unsigned int v0;
//    unsigned int v1;
//    unsigned int v2;
//    unsigned int v3;
//};
//
//
//class FloorGenerator
//{
//public:
//
//    FloorGenerator() = default;
//    ~FloorGenerator();
//
//    bool Initialize();
//
//    bool GenerateFloor(float width, float depth, int subdivisionsX, int subdivisionsZ);
//
//    MeshData BuildMeshData() const;
//
//    const std::vector<FloorVertex>& GetVertices() const
//    {
//        return m_vertices;
//    }
//
//
//    const std::vector<FloorFace>& GetFaces() const
//    {
//        return m_faces;
//    }
//
//
//private:
//
//    std::vector<FloorVertex>m_vertices;
//
//    std::vector<FloorFace>m_faces;
//};