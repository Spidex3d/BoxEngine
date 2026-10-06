#include "mesh/MeshCombiner.h"
#include <entity/Entity.h>

#include <glm/glm.hpp>

bool MeshCombiner::Combine(
    const Entity& entityA,
    const Entity& entityB,
    MeshEditing& outMesh)
{
    outMesh.Clear();


    // =================================================
    // Helper:
    // Obtain editable topology from an Entity.
    //
    // Native BoxEditor objects already have an
    // editable mesh.
    //
    // Imported MBX objects currently only contain
    // render mesh data, so create temporary editable
    // triangle topology from that data.
    // =================================================

    auto GetEditableMesh =
        [](const Entity& entity,
            MeshEditing& temporaryMesh)
        -> const MeshEditing*
    {
        const MeshEditing& editable =
            entity.GetEditableMesh();


        // -------------------------------------------------
        // Native editable BoxEditor object.
        // -------------------------------------------------

        if (editable.GetVertexCount() > 0 &&
            editable.GetFaceCount() > 0)
        {
            return &editable;
        }


        // -------------------------------------------------
        // Imported / render-only mesh fallback.
        // -------------------------------------------------

        const MeshData& renderMesh =
            entity.GetMeshData();


        if (renderMesh.vertices.empty())
        {
            return nullptr;
        }


        temporaryMesh.Clear();


        // -------------------------------------------------
        // MBX importer currently expands geometry into
        // groups of three non-indexed triangle vertices.
        //
        // Convert each triangle into one editable face.
        // -------------------------------------------------

        // -------------------------------------------------
// Find an existing editable vertex at the same
// position, or create a new one.
//
// This restores shared topology from the expanded
// MBX render triangles and allows smooth shading
// to average normals across neighbouring faces.
// -------------------------------------------------

        auto FindOrAddVertex =
            [&temporaryMesh](
                const glm::vec3& position)
            -> std::size_t
        {
            constexpr float epsilon =
                0.00001f;

            for (std::size_t vertexIndex = 0;
                vertexIndex <
                temporaryMesh.GetVertexCount();
                ++vertexIndex)
            {
                const glm::vec3& existingPosition =
                    temporaryMesh
                    .GetVertex(vertexIndex)
                    .position;

                const glm::vec3 difference =
                    existingPosition -
                    position;

                if (glm::dot(
                    difference,
                    difference) <=
                    epsilon * epsilon)
                {
                    return vertexIndex;
                }
            }

            return temporaryMesh.AddVertex(
                position
            );
        };


        // -------------------------------------------------
        // Reconstruct MBX triangles using shared logical
        // vertices.
        // -------------------------------------------------

        for (std::size_t i = 0;
            i + 2 < renderMesh.vertices.size();
            i += 3)
        {
            const std::size_t a =
                FindOrAddVertex(
                    renderMesh.vertices[i].position
                );

            const std::size_t b =
                FindOrAddVertex(
                    renderMesh.vertices[i + 1].position
                );

            const std::size_t c =
                FindOrAddVertex(
                    renderMesh.vertices[i + 2].position
                );


            const std::size_t faceIndex =
                temporaryMesh.AddFace(
                    { a, b, c }
                );


            EditFace& face =
                temporaryMesh.GetFace(
                    faceIndex
                );


            // Preserve material.
            face.materialIndex =
                renderMesh.vertices[i]
                .materialIndex;


            // UVs remain face-corner data, so welding
            // vertices does not destroy UV seams.
            face.uvs =
            {
                renderMesh.vertices[i].uv,
                renderMesh.vertices[i + 1].uv,
                renderMesh.vertices[i + 2].uv
            };
        }


        //for (std::size_t i = 0;
        //    i + 2 < renderMesh.vertices.size();
        //    i += 3)
        //{
        //    // ---------------------------------------------
        //    // Create editable vertices.
        //    // ---------------------------------------------

        //    const std::size_t a =
        //        temporaryMesh.AddVertex(
        //            renderMesh.vertices[i].position
        //        );


        //    const std::size_t b =
        //        temporaryMesh.AddVertex(
        //            renderMesh.vertices[i + 1].position
        //        );


        //    const std::size_t c =
        //        temporaryMesh.AddVertex(
        //            renderMesh.vertices[i + 2].position
        //        );


        //    // ---------------------------------------------
        //    // Create editable triangle face.
        //    // ---------------------------------------------

        //    const std::size_t faceIndex =
        //        temporaryMesh.AddFace(
        //            { a, b, c }
        //        );


        //    EditFace& face =
        //        temporaryMesh.GetFace(
        //            faceIndex
        //        );


        //    // ---------------------------------------------
        //    // Preserve material index from imported mesh.
        //    // ---------------------------------------------

        //    face.materialIndex =
        //        renderMesh.vertices[i].materialIndex;


        //    // ---------------------------------------------
        //    // Preserve UV coordinates.
        //    // ---------------------------------------------

        //    face.uvs =
        //    {
        //        renderMesh.vertices[i].uv,
        //        renderMesh.vertices[i + 1].uv,
        //        renderMesh.vertices[i + 2].uv
        //    };
        //}


        // -------------------------------------------------
        // Rebuild editable edges from the new faces.
        // -------------------------------------------------

        temporaryMesh.RebuildEdges();


        if (temporaryMesh.GetVertexCount() == 0 ||
            temporaryMesh.GetFaceCount() == 0)
        {
            return nullptr;
        }


        return &temporaryMesh;
    };


    // =================================================
    // Obtain topology for both entities.
    // =================================================

    MeshEditing temporaryA;
    MeshEditing temporaryB;


    const MeshEditing* meshAPtr =
        GetEditableMesh(
            entityA,
            temporaryA
        );


    const MeshEditing* meshBPtr =
        GetEditableMesh(
            entityB,
            temporaryB
        );


    if (!meshAPtr ||
        !meshBPtr)
    {
        return false;
    }


    const MeshEditing& meshA =
        *meshAPtr;


    const MeshEditing& meshB =
        *meshBPtr;


    // =================================================
    // MATERIAL OFFSET
    //
    // Entity A material slots will appear first in the
    // joined Entity.
    //
    // Therefore Entity B's material indices must be
    // moved forward by the number of A's material slots.
    //
    // Example:
    //
    // A:
    //   0 Default
    //   1 Bark
    //
    // B:
    //   0 Default
    //   1 Leaves
    //
    // Joined:
    //   0 Default
    //   1 Bark
    //   2 Default
    //   3 Leaves
    // =================================================

    const std::size_t materialOffsetB =
        entityA.GetMaterialSlotCount();


    // =================================================
    // Validate editable meshes.
    // =================================================

    if (meshA.GetVertexCount() == 0 ||
        meshA.GetFaceCount() == 0)
    {
        return false;
    }


    if (meshB.GetVertexCount() == 0 ||
        meshB.GetFaceCount() == 0)
    {
        return false;
    }


    // =================================================
    // Get transforms.
    //
    // We bake both entities into world space so the
    // joined Entity can use:
    //
    // Position = 0
    // Rotation = 0
    // Scale    = 1
    // =================================================

    const glm::mat4 modelA =
        entityA.GetModelMatrix();


    const glm::mat4 modelB =
        entityB.GetModelMatrix();



    // =================================================
    // ENTITY A
    // =================================================


    // -------------------------------------------------
    // Copy Entity A vertices.
    // -------------------------------------------------

    for (std::size_t i = 0;
        i < meshA.GetVertexCount();
        ++i)
    {
        const glm::vec3 localPosition =
            meshA.GetVertex(i).position;


        const glm::vec3 worldPosition =
            glm::vec3(
                modelA *
                glm::vec4(
                    localPosition,
                    1.0f
                )
            );


        outMesh.AddVertex(
            worldPosition
        );
    }


    // -------------------------------------------------
    // Copy Entity A faces.
    //
    // Entity A material indices do NOT need changing
    // because its material slots are first.
    // -------------------------------------------------

    for (std::size_t i = 0;
        i < meshA.GetFaceCount();
        ++i)
    {
        const EditFace& sourceFace =
            meshA.GetFace(i);


        const std::size_t newFaceIndex =
            outMesh.AddFace(
                sourceFace.vertices
            );


        EditFace& newFace =
            outMesh.GetFace(
                newFaceIndex
            );


        // Preserve Entity A material.
        newFace.materialIndex =
            sourceFace.materialIndex;


        // Preserve Entity A UVs.
        newFace.uvs =
            sourceFace.uvs;
    }



    // =================================================
    // ENTITY B
    // =================================================


    // -------------------------------------------------
    // Entity B's vertex indices need offsetting because
    // Entity A's vertices are already in outMesh.
    // -------------------------------------------------

    const std::size_t vertexOffset =
        outMesh.GetVertexCount();


    // -------------------------------------------------
    // Copy Entity B vertices.
    // -------------------------------------------------

    for (std::size_t i = 0;
        i < meshB.GetVertexCount();
        ++i)
    {
        const glm::vec3 localPosition =
            meshB.GetVertex(i).position;


        const glm::vec3 worldPosition =
            glm::vec3(
                modelB *
                glm::vec4(
                    localPosition,
                    1.0f
                )
            );


        outMesh.AddVertex(
            worldPosition
        );
    }


    // -------------------------------------------------
    // Copy Entity B faces.
    // -------------------------------------------------

    for (std::size_t i = 0;
        i < meshB.GetFaceCount();
        ++i)
    {
        const EditFace& sourceFace =
            meshB.GetFace(i);


        std::vector<std::size_t>
            newVertices;


        newVertices.reserve(
            sourceFace.vertices.size()
        );


        // ---------------------------------------------
        // Offset Entity B vertex indices.
        // ---------------------------------------------

        for (std::size_t vertexIndex :
        sourceFace.vertices)
        {
            newVertices.push_back(
                vertexIndex +
                vertexOffset
            );
        }


        // ---------------------------------------------
        // Create new joined face.
        // ---------------------------------------------

        const std::size_t newFaceIndex =
            outMesh.AddFace(
                newVertices
            );


        EditFace& newFace =
            outMesh.GetFace(
                newFaceIndex
            );


        // ---------------------------------------------
        // IMPORTANT:
        //
        // Offset Entity B's material index so it points
        // at Entity B's copied material slots in the
        // joined Entity.
        // ---------------------------------------------

        newFace.materialIndex =
            sourceFace.materialIndex +
            materialOffsetB;


        // ---------------------------------------------
        // Preserve Entity B UVs.
        // ---------------------------------------------

        newFace.uvs =
            sourceFace.uvs;
    }

    // =================================================
    // Rebuild editable edges for the complete mesh.
    // =================================================

    outMesh.RebuildEdges();


    // =================================================
    // Final validation.
    // =================================================

    return
        outMesh.GetVertexCount() > 0 &&
        outMesh.GetFaceCount() > 0;
}



//bool MeshCombiner::Combine(const Entity& entityA, const Entity& entityB, MeshEditing& outMesh)
//{
//    outMesh.Clear();
//
//    // -------------------------------------------------
//    // Helper:
//    // obtain editable topology from an Entity.
//    //
//    // Native BoxEditor objects already have it.
//    // Imported MBX objects currently do not.
//    // -------------------------------------------------
//
//    auto GetEditableMesh =
//        [](const Entity& entity,
//            MeshEditing& temporaryMesh)
//        -> const MeshEditing*
//    {
//        const MeshEditing& editable =
//            entity.GetEditableMesh();
//
//        if (editable.GetVertexCount() > 0 &&
//            editable.GetFaceCount() > 0)
//        {
//            return &editable;
//        }
//
//        // ---------------------------------------------
//        // Imported/render-only mesh fallback.
//        // ---------------------------------------------
//
//        const MeshData& renderMesh =
//            entity.GetMeshData();
//
//        if (renderMesh.vertices.empty())
//        {
//            return nullptr;
//        }
//
//        temporaryMesh.Clear();
//
//        // MBX importer currently expands geometry into
//        // groups of three non-indexed triangle vertices.
//        for (std::size_t i = 0;
//            i + 2 < renderMesh.vertices.size();
//            i += 3)
//        {
//            const std::size_t a =
//                temporaryMesh.AddVertex(
//                    renderMesh.vertices[i].position
//                );
//
//            const std::size_t b =
//                temporaryMesh.AddVertex(
//                    renderMesh.vertices[i + 1].position
//                );
//
//            const std::size_t c =
//                temporaryMesh.AddVertex(
//                    renderMesh.vertices[i + 2].position
//                );
//
//            const std::size_t faceIndex =
//                temporaryMesh.AddFace(
//                    { a, b, c }
//                );
//
//            EditFace& face =
//                temporaryMesh.GetFace(
//                    faceIndex
//                );
//
//            face.materialIndex =
//                renderMesh.vertices[i].materialIndex;
//
//            face.uvs =
//            {
//                renderMesh.vertices[i].uv,
//                renderMesh.vertices[i + 1].uv,
//                renderMesh.vertices[i + 2].uv
//            };
//        }
//
//        temporaryMesh.RebuildEdges();
//
//        if (temporaryMesh.GetFaceCount() == 0)
//        {
//            return nullptr;
//        }
//
//        return &temporaryMesh;
//    };
//
//
//    MeshEditing temporaryA;
//    MeshEditing temporaryB;
//
//    const MeshEditing* meshAPtr =
//        GetEditableMesh(
//            entityA,
//            temporaryA
//        );
//
//    const MeshEditing* meshBPtr =
//        GetEditableMesh(
//            entityB,
//            temporaryB
//        );
//
//    if (!meshAPtr || !meshBPtr)
//    {
//        return false;
//    }
//
//    const MeshEditing& meshA =
//        *meshAPtr;
//
//    const MeshEditing& meshB =
//        *meshBPtr;
//
//    const std::size_t materialOffsetB = entityA.GetMaterialSlotCount();
//
//	// =================================================
//
//    if (meshA.GetVertexCount() == 0 ||
//        meshA.GetFaceCount() == 0)
//    {
//        return false;
//    }
//
//    if (meshB.GetVertexCount() == 0 ||
//        meshB.GetFaceCount() == 0)
//    {
//        return false;
//    }
//
//
//    const glm::mat4 modelA = entityA.GetModelMatrix();
//
//    const glm::mat4 modelB = entityB.GetModelMatrix();
//
//
//    // =================================================
//    // ENTITY A
//    // =================================================
//
//    for (std::size_t i = 0;
//        i < meshA.GetVertexCount();
//        ++i)
//    {
//        const glm::vec3 localPosition =
//            meshA.GetVertex(i).position;
//
//        const glm::vec3 worldPosition =
//            glm::vec3(
//                modelA *
//                glm::vec4(
//                    localPosition,
//                    1.0f
//                )
//            );
//
//        outMesh.AddVertex(
//            worldPosition
//        );
//    }
//
//
//    for (std::size_t i = 0;
//        i < meshA.GetFaceCount();
//        ++i)
//    {
//        const EditFace& sourceFace =
//            meshA.GetFace(i);
//
//        const std::size_t newFaceIndex =
//            outMesh.AddFace(
//                sourceFace.vertices
//            );
//
//        EditFace& newFace =
//            outMesh.GetFace(
//                newFaceIndex
//            );
//
//        newFace.materialIndex =
//            sourceFace.materialIndex;
//
//        newFace.uvs =
//            sourceFace.uvs;
//    }
//
//
//    // =================================================
//    // ENTITY B
//    // =================================================
//
//    const std::size_t vertexOffset =
//        outMesh.GetVertexCount();
//
//
//    for (std::size_t i = 0;
//        i < meshB.GetVertexCount();
//        ++i)
//    {
//        const glm::vec3 localPosition =
//            meshB.GetVertex(i).position;
//
//        const glm::vec3 worldPosition =
//            glm::vec3(
//                modelB *
//                glm::vec4(
//                    localPosition,
//                    1.0f
//                )
//            );
//
//        outMesh.AddVertex(
//            worldPosition
//        );
//    }
//
//
//    for (std::size_t i = 0;
//        i < meshB.GetFaceCount();
//        ++i)
//    {
//        const EditFace& sourceFace =
//            meshB.GetFace(i);
//
//        std::vector<std::size_t>
//            newVertices;
//
//        newVertices.reserve(
//            sourceFace.vertices.size()
//        );
//
//
//        for (std::size_t vertexIndex :
//        sourceFace.vertices)
//        {
//            newVertices.push_back(
//                vertexIndex +
//                vertexOffset
//            );
//        }
//
//
//        const std::size_t newFaceIndex =
//            outMesh.AddFace(
//                newVertices
//            );
//
//        EditFace& newFace =
//            outMesh.GetFace(
//                newFaceIndex
//            );
//
//        newFace.materialIndex =
//            sourceFace.materialIndex;
//
//        newFace.uvs =
//            sourceFace.uvs;
//    }
//
//
//    // Build all the editable edges again.
//    outMesh.RebuildEdges();
//
//
//    return
//        outMesh.GetVertexCount() > 0 &&
//        outMesh.GetFaceCount() > 0;
//
//    
//}