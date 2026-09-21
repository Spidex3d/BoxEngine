#include <fileManager\SceneSerializer.h>
#include <entity/Entity.h>
#include <miniBoxLog.h>

#include <fstream>
#include <iomanip>

SceneSerializer::~SceneSerializer() = default;

bool SceneSerializer::SerializeScene(
    const std::filesystem::path& filePath,
    const std::vector<std::unique_ptr<Entity>>& entities)
{
    std::filesystem::path finalPath =
        filePath;

    // ---------------------------------------------
    // Make sure this is a BoxEditor scene file.
    // ---------------------------------------------
    if (finalPath.extension() != ".mbs")
    {
        finalPath.replace_extension(".mbs");
    }

    // ---------------------------------------------
    // Create parent directory if required.
    // ---------------------------------------------
    const std::filesystem::path directory =
        finalPath.parent_path();

    if (!directory.empty())
    {
        std::error_code error;

        std::filesystem::create_directories(
            directory,
            error
        );

        if (error)
        {
            BOX_LOG_ERROR(
                "Failed to create scene directory: "
                << error.message()
            );

            return false;
        }
    }

    // ---------------------------------------------
    // Open scene file.
    // ---------------------------------------------
    std::ofstream file(
        finalPath,
        std::ios::out |
        std::ios::trunc
    );

    if (!file.is_open())
    {
        BOX_LOG_ERROR(
            "Failed to create scene file: "
            << finalPath.string()
        );

        return false;
    }

    file << std::fixed
        << std::setprecision(6);

    // ---------------------------------------------
    // Scene header.
    // ---------------------------------------------
    file << "# BoxEditor Scene\n";
    file << "version 0 1\n\n";

    file << "entities "
        << entities.size()
        << "\n\n";

    // ---------------------------------------------
    // Serialize every entity.
    // ---------------------------------------------
    for (const auto& entity : entities)
    {
        if (!entity)
        {
            continue;
        }

        if (!SerializeEntity(
            file,
            *entity))
        {
            BOX_LOG_ERROR(
                "Failed to serialize entity"
            );

            return false;
        }
    }

    if (!file.good())
    {
        BOX_LOG_ERROR(
            "Failed while writing scene file"
        );

        return false;
    }

    BOX_LOG_INFO(
        "Scene saved: "
        << finalPath.string()
    );

    return true;
}


bool SceneSerializer::SerializeEntity(
    std::ostream& output,
    const Entity& entity)
{
    const glm::vec3& position =
        entity.GetPosition();

    const glm::vec3& rotation =
        entity.GetRotation();

    const glm::vec3& scale =
        entity.GetScale();

    output << "entity\n";

    output
        << "id "
        << entity.GetID()
        << '\n';

    output
        << "name "
        << entity.GetName()
        << '\n';

    output
        << "primitive "
        << static_cast<int>(
            entity.GetPrimitiveType()
            )
        << '\n';

    output
        << "visible "
        << (entity.IsVisible() ? 1 : 0)
        << '\n';

    output
        << "position "
        << position.x << ' '
        << position.y << ' '
        << position.z
        << '\n';

    output
        << "rotation "
        << rotation.x << ' '
        << rotation.y << ' '
        << rotation.z
        << '\n';

    output
        << "scale "
        << scale.x << ' '
        << scale.y << ' '
        << scale.z
        << '\n';

    // =================================================
    // EDITABLE MESH
    // =================================================

    const MeshEditing& mesh =
        entity.GetEditableMesh();

    output
        << "shading "
        << static_cast<int>(
            mesh.GetShadingMode()
            )
        << '\n';


    // -------------------------------------------------
    // Vertices
    // -------------------------------------------------

    output
        << "vertices "
        << mesh.GetVertexCount()
        << '\n';

    for (const EditVertex& vertex :
        mesh.GetVertices())
    {
        output
            << "v "
            << vertex.position.x << ' '
            << vertex.position.y << ' '
            << vertex.position.z
            << '\n';
    }


    // -------------------------------------------------
    // Faces
    // -------------------------------------------------

    output
        << "faces "
        << mesh.GetFaceCount()
        << '\n';

    for (const EditFace& face :
        mesh.GetFaces())
    {
        output
            << "f "
            << face.vertices.size();

        for (std::size_t vertexIndex :
        face.vertices)
        {
            output
                << ' '
                << vertexIndex;
        }

        // Save the material assigned to this face.
        output
            << " m "
            << face.materialIndex;

        output << '\n';
    }


    // -------------------------------------------------
    // Loose edges
    // -------------------------------------------------

    std::size_t looseEdgeCount = 0;

    for (const EditEdge& edge :
        mesh.GetEdges())
    {
        if (edge.loose)
        {
            ++looseEdgeCount;
        }
    }

    output
        << "loose_edges "
        << looseEdgeCount
        << '\n';

    for (const EditEdge& edge :
        mesh.GetEdges())
    {
        if (!edge.loose)
        {
            continue;
        }

        output
            << "e "
            << edge.vertexA << ' '
            << edge.vertexB
            << '\n';
    }

    // =================================================
// MATERIAL SLOTS
// =================================================

    output
        << "materials "
        << entity.GetMaterialSlotCount()
        << '\n';

    for (std::size_t i = 0;
        i < entity.GetMaterialSlotCount();
        ++i)
    {
        const Material& material =
            entity.GetMaterialSlot(i);

        const glm::vec4& baseColor =
            material.GetBaseColor();

        const glm::vec3& emissionColor =
            material.GetEmissionColor();

        output
            << "material "
            << i
            << '\n';

        output
            << "material_name "
            << material.GetName()
            << '\n';

        output
            << "material_type "
            << static_cast<int>(
                material.GetType()
                )
            << '\n';

        output
            << "base_color "
            << baseColor.r << ' '
            << baseColor.g << ' '
            << baseColor.b << ' '
            << baseColor.a
            << '\n';

        output
            << "metallic "
            << material.GetMetallic()
            << '\n';

        output
            << "roughness "
            << material.GetRoughness()
            << '\n';

        output
            << "alpha "
            << material.GetAlpha()
            << '\n';

        output
            << "emission_color "
            << emissionColor.r << ' '
            << emissionColor.g << ' '
            << emissionColor.b
            << '\n';

        output
            << "emission_strength "
            << material.GetEmissionStrength()
            << '\n';

        output
            << "transmission "
            << material.GetTransmission()
            << '\n';

        output
            << "ior "
            << material.GetIOR()
            << '\n';

        // ---------------------------------------------
        // Base color texture
        // ---------------------------------------------

        output
            << "use_base_texture "
            << (material.UsesBaseColorTexture() ? 1 : 0)
            << '\n';

        output
            << "base_texture "
            << material.GetBaseColorTexturePath()
            << '\n';

        // ---------------------------------------------
        // Normal map
        // ---------------------------------------------

        output
            << "use_normal_texture "
            << (material.UsesNormalTexture() ? 1 : 0)
            << '\n';

        output
            << "normal_texture "
            << material.GetNormalTexturePath()
            << '\n';

        output
            << "normal_strength "
            << material.GetNormalStrength()
            << '\n';

        output << "endmaterial\n";
    }


    output << "endentity\n\n";

    return output.good();
}

//bool SceneSerializer::DeserializeScene(const std::filesystem::path& filePath)
bool SceneSerializer::DeserializeScene(const std::filesystem::path& filePath, std::vector<std::unique_ptr<Entity>>& loadedEntities)
{
    loadedEntities.clear();
    // ---------------------------------------------
    // Open scene file.
    // ---------------------------------------------
    std::ifstream file(filePath);

    if (!file.is_open())
    {
        BOX_LOG_ERROR(
            "Failed to open scene file: "
            << filePath.string()
        );

        return false;
    }

    // ---------------------------------------------
    // Read scene header.
    //
    // First line:
    // # BoxEditor Scene
    // ---------------------------------------------
    std::string line;

    if (!std::getline(file, line))
    {
        BOX_LOG_ERROR(
            "Scene file is empty"
        );

        return false;
    }

    if (line != "# BoxEditor Scene")
    {
        BOX_LOG_ERROR(
            "Invalid BoxEditor scene file"
        );

        return false;
    }

    // ---------------------------------------------
    // Read version.
    //
    // version 0 1
    // ---------------------------------------------
    std::string versionToken;

    int versionMajor = 0;
    int versionMinor = 0;

    if (!(file
        >> versionToken
        >> versionMajor
        >> versionMinor))
    {
        BOX_LOG_ERROR(
            "Failed to read scene version"
        );

        return false;
    }

    if (versionToken != "version")
    {
        BOX_LOG_ERROR(
            "Invalid scene version header"
        );

        return false;
    }

    // ---------------------------------------------
    // Check supported version.
    // ---------------------------------------------
    if (versionMajor != MBSENE_VERSION_MAJOR ||
        versionMinor != MBSENE_VERSION_MINOR)
    {
        BOX_LOG_ERROR(
            "Unsupported scene version: "
            << versionMajor
            << "."
            << versionMinor
        );

        return false;
    }

    // ---------------------------------------------
    // Read entity count.
    //
    // entities 4
    // ---------------------------------------------
    std::string entitiesToken;

    std::size_t entityCount = 0;

    if (!(file
        >> entitiesToken
        >> entityCount))
    {
        BOX_LOG_ERROR(
            "Failed to read scene entity count"
        );

        return false;
    }

    if (entitiesToken != "entities")
    {
        BOX_LOG_ERROR(
            "Invalid scene entities header"
        );

        return false;
    }

    BOX_LOG_INFO(
        "Scene file opened successfully. "
        << "Version="
        << versionMajor
        << "."
        << versionMinor
        << " Entities="
        << entityCount
    );


    // ---------------------------------------------
// Read basic entity information.
// ---------------------------------------------
    for (std::size_t i = 0; i < entityCount; ++i)
    {
        std::string token;

        // entity
        if (!(file >> token) || token != "entity")
        {
            BOX_LOG_ERROR(
                "Expected entity block"
            );

            return false;
        }

        // -----------------------------------------
        // ID
        // -----------------------------------------
        int entityID = -1;

        if (!(file >> token >> entityID) ||
            token != "id")
        {
            BOX_LOG_ERROR(
                "Failed to read entity ID"
            );

            return false;
        }

        // -----------------------------------------
        // Name
        // -----------------------------------------
        std::string entityName;

        if (!(file >> token) ||
            token != "name")
        {
            BOX_LOG_ERROR(
                "Failed to read entity name"
            );

            return false;
        }

        // Remove the space left after "name".
        file >> std::ws;

        std::getline(
            file,
            entityName
        );

        // -----------------------------------------
        // Primitive type
        // -----------------------------------------
        int primitiveType = 0;

        if (!(file >> token >> primitiveType) ||
            token != "primitive")
        {
            BOX_LOG_ERROR(
                "Failed to read primitive type"
            );

            return false;
        }

        // -----------------------------------------
        // Visibility
        // -----------------------------------------
        int visible = 1;

        if (!(file >> token >> visible) ||
            token != "visible")
        {
            BOX_LOG_ERROR(
                "Failed to read entity visibility"
            );

            return false;
        }

        // -----------------------------------------
        // Position
        // -----------------------------------------
        glm::vec3 position(0.0f);

        if (!(file
            >> token
            >> position.x
            >> position.y
            >> position.z) ||
            token != "position")
        {
            BOX_LOG_ERROR(
                "Failed to read entity position"
            );

            return false;
        }

        // -----------------------------------------
        // Rotation
        // -----------------------------------------
        glm::vec3 rotation(0.0f);

        if (!(file
            >> token
            >> rotation.x
            >> rotation.y
            >> rotation.z) ||
            token != "rotation")
        {
            BOX_LOG_ERROR(
                "Failed to read entity rotation"
            );

            return false;
        }

        // -----------------------------------------
        // Scale
        // -----------------------------------------
        glm::vec3 scale(1.0f);

        if (!(file
            >> token
            >> scale.x
            >> scale.y
            >> scale.z) ||
            token != "scale")
        {
            BOX_LOG_ERROR(
                "Failed to read entity scale"
            );

            return false;
        }

        // -----------------------------------------
        // Shading
        // -----------------------------------------
        int shadingMode = 0;

        if (!(file >> token >> shadingMode) ||
            token != "shading")
        {
            BOX_LOG_ERROR(
                "Failed to read shading mode"
            );

            return false;
        }

        // -----------------------------------------
        // Temporary debug output.
        // -----------------------------------------
        BOX_LOG_INFO(
            "Loaded entity header: "
            << "ID=" << entityID
            << " Name=" << entityName
            << " Primitive=" << primitiveType
            << " Visible=" << visible
            << " Position=("
            << position.x << ", "
            << position.y << ", "
            << position.z << ")"
            << " Rotation=("
            << rotation.x << ", "
            << rotation.y << ", "
            << rotation.z << ")"
            << " Scale=("
            << scale.x << ", "
            << scale.y << ", "
            << scale.z << ")"
            << " Shading=" << shadingMode
        );

        
        // ------------------------------------------------
        // -----------------------------------------
        // Editable vertices
        // -----------------------------------------
        std::size_t vertexCount = 0;

        if (!(file >> token >> vertexCount) ||
            token != "vertices")
        {
            BOX_LOG_ERROR(
                "Failed to read editable vertex count"
            );

            return false;
        }

        std::vector<glm::vec3> loadedVertices;

        loadedVertices.reserve(vertexCount);

        for (std::size_t vertexIndex = 0;
            vertexIndex < vertexCount;
            ++vertexIndex)
        {
            glm::vec3 vertexPosition(0.0f);

            if (!(file
                >> token
                >> vertexPosition.x
                >> vertexPosition.y
                >> vertexPosition.z) ||
                token != "v")
            {
                BOX_LOG_ERROR(
                    "Failed to read editable vertex "
                    << vertexIndex
                );

                return false;
            }

            loadedVertices.push_back(
                vertexPosition
            );
        }

        BOX_LOG_INFO(
            "Loaded "
            << loadedVertices.size()
            << " editable vertices for "
            << entityName
        );

        // -----------------------------------------
        // Editable faces
        // -----------------------------------------
        std::size_t faceCount = 0;

        if (!(file >> token >> faceCount) ||
            token != "faces")
        {
            BOX_LOG_ERROR(
                "Failed to read editable face count"
            );

            return false;
        }

        struct LoadedFace
        {
            std::vector<std::size_t> vertices;
            std::size_t materialIndex = 0;
        };

        std::vector<LoadedFace> loadedFaces;

        loadedFaces.reserve(faceCount);

        for (std::size_t faceIndex = 0;
            faceIndex < faceCount;
            ++faceIndex)
        {
            std::size_t faceVertexCount = 0;

            if (!(file >> token >> faceVertexCount) ||
                token != "f")
            {
                BOX_LOG_ERROR(
                    "Failed to read editable face "
                    << faceIndex
                );

                return false;
            }

            LoadedFace loadedFace;

            loadedFace.vertices.reserve(
                faceVertexCount
            );

            for (std::size_t vertexIndex = 0;
                vertexIndex < faceVertexCount;
                ++vertexIndex)
            {
                std::size_t index = 0;

                if (!(file >> index))
                {
                    BOX_LOG_ERROR(
                        "Failed to read vertex index for face "
                        << faceIndex
                    );

                    return false;
                }

                // Make sure the scene file isn't referring
                // to a vertex that doesn't exist.
                if (index >= loadedVertices.size())
                {
                    BOX_LOG_ERROR(
                        "Invalid vertex index "
                        << index
                        << " in face "
                        << faceIndex
                    );

                    return false;
                }

                loadedFace.vertices.push_back(
                    index
                );
            }

            // -----------------------------------------
            // Face material
            //
            // m <materialIndex>
            // -----------------------------------------
            std::string materialToken;

            if (!(file
                >> materialToken
                >> loadedFace.materialIndex) ||
                materialToken != "m")
            {
                BOX_LOG_ERROR(
                    "Failed to read material index for face "
                    << faceIndex
                );

                return false;
            }

            loadedFaces.push_back(
                std::move(loadedFace)
            );
        }

        BOX_LOG_INFO(
            "Loaded "
            << loadedFaces.size()
            << " editable faces for "
            << entityName
        );

        for (std::size_t i = 0;
            i < loadedFaces.size();
            ++i)
        {
            BOX_LOG_INFO(
                "Face "
                << i
                << " Vertices="
                << loadedFaces[i].vertices.size()
                << " Material="
                << loadedFaces[i].materialIndex
            );
        }

        // -----------------------------------------
        // Loose edges
        // -----------------------------------------
        std::size_t looseEdgeCount = 0;

        if (!(file >> token >> looseEdgeCount) ||
            token != "loose_edges")
        {
            BOX_LOG_ERROR(
                "Failed to read loose edge count"
            );

            return false;
        }

        struct LoadedEdge
        {
            std::size_t vertexA = 0;
            std::size_t vertexB = 0;
        };

        std::vector<LoadedEdge> loadedLooseEdges;

        loadedLooseEdges.reserve(
            looseEdgeCount
        );

        for (std::size_t edgeIndex = 0;
            edgeIndex < looseEdgeCount;
            ++edgeIndex)
        {
            LoadedEdge edge;

            if (!(file
                >> token
                >> edge.vertexA
                >> edge.vertexB) ||
                token != "e")
            {
                BOX_LOG_ERROR(
                    "Failed to read loose edge "
                    << edgeIndex
                );

                return false;
            }

            // Validate the vertex indices.
            if (edge.vertexA >= loadedVertices.size() ||
                edge.vertexB >= loadedVertices.size())
            {
                BOX_LOG_ERROR(
                    "Invalid vertex index in loose edge "
                    << edgeIndex
                );

                return false;
            }

            loadedLooseEdges.push_back(
                edge
            );
        }

        BOX_LOG_INFO(
            "Loaded "
            << loadedLooseEdges.size()
            << " loose edges for "
            << entityName
        );

        // =================================================
        // MATERIAL SLOTS
        // =================================================

        std::size_t materialCount = 0;

        if (!(file >> token >> materialCount) ||
            token != "materials")
        {
            BOX_LOG_ERROR("Failed to read material count. Token=[" << token << "]");
            /*BOX_LOG_ERROR(
                "Failed to read material count for "
                << entityName
            );*/

            return false;
        }

        BOX_LOG_INFO("Material count for " << entityName << " = " << materialCount);

        std::vector<Material> loadedMaterials;

        loadedMaterials.reserve(materialCount);

        for (std::size_t materialIndex = 0;
            materialIndex < materialCount;
            ++materialIndex)
        {
            std::size_t savedMaterialIndex = 0;

            if (!(file >> token >> savedMaterialIndex) ||
                token != "material")
            {
                BOX_LOG_INFO("Reading material slot " << savedMaterialIndex);
                /*BOX_LOG_ERROR(
                    "Failed to read material header"
                );*/

                return false;
            }

            Material material;

            // -----------------------------------------
            // Name
            // -----------------------------------------
            if (!(file >> token) ||
                token != "material_name")
            {
                BOX_LOG_ERROR(
                    "Failed to read material name. Token=["
                    << token
                    << "]"
                );

                return false;
            }

            std::string materialName;

            // Consume the single space after "material_name".
            if (file.peek() == ' ')
            {
                file.get();
            }

            std::getline(
                file,
                materialName
            );

            material.SetName(
                materialName
            );

            BOX_LOG_INFO(
                "Material name = ["
                << materialName
                << "]"
            );

           

            // -----------------------------------------
            // Type
            // -----------------------------------------
            int materialType = 0;

            if (!(file >> token >> materialType) ||
                token != "material_type")
            {
                BOX_LOG_ERROR(
                    "Failed to read material type. Token=["
                    << token
                    << "]"
                );

                return false;
            }

            BOX_LOG_INFO(
                "Material type = "
                << materialType
            );

            material.SetType(
                static_cast<MaterialType>(materialType)
            );


        // -----------------------------------------
        // Base colour
        // -----------------------------------------

        glm::vec4 baseColor(1.0f);
        
        if (!(file
            >> token
            >> baseColor.r
            >> baseColor.g
            >> baseColor.b
            >> baseColor.a) ||
            token != "base_color")
        {
            BOX_LOG_ERROR(
                "Failed to read base_color. Token=["
                << token << "]"
            );
        
            return false;
        }

        material.SetBaseColor(baseColor);
        
        BOX_LOG_INFO(
            "Base color = ("
            << baseColor.r << ", "
            << baseColor.g << ", "
            << baseColor.b << ", "
            << baseColor.a << ")"
        );

        // -----------------------------------------
        // Metallic
        // -----------------------------------------
        float metallic = 0.0f;
        
        if (!(file >> token >> metallic) ||
            token != "metallic")
        {
            BOX_LOG_ERROR(
                "Failed to read metallic. Token=["
                << token << "]"
            );
        
            return false;
        }
        
        material.SetMetallic(metallic);

        // -----------------------------------------
        // Roughness
        // -----------------------------------------

        float roughness = 0.5f;
        
        if (!(file >> token >> roughness) ||
            token != "roughness")
        {
            BOX_LOG_ERROR(
                "Failed to read roughness. Token=["
                << token << "]"
            );
        
            return false;
        }
        
        material.SetRoughness(roughness);

        // -----------------------------------------
        // Alpha
        // -----------------------------------------

        float alpha = 1.0f;
        
        if (!(file >> token >> alpha) ||
            token != "alpha")
        {
            BOX_LOG_ERROR(
                "Failed to read alpha. Token=["
                << token << "]"
            );
        
            return false;
        }
        
        material.SetAlpha(alpha);

        // -----------------------------------------
        // Emission colour
        // -----------------------------------------

        glm::vec3 emissionColor(0.0f);
        
        if (!(file
            >> token
            >> emissionColor.r
            >> emissionColor.g
            >> emissionColor.b) ||
            token != "emission_color")
        {
            BOX_LOG_ERROR(
                "Failed to read emission_color. Token=["
                << token << "]"
            );
        
            return false;
        }

        material.SetEmissionColor(emissionColor);

        // -----------------------------------------
        // Emission strength
        // -----------------------------------------

        float emissionStrength = 0.0f;

        if (!(file >> token >> emissionStrength) ||
            token != "emission_strength")
        {
        BOX_LOG_ERROR(
        "Failed to read emission_strength. Token=["
         << token << "]"
            );

            return false;
        }

        material.SetEmissionStrength(emissionStrength);

        // -----------------------------------------
        // Transmission
        // -----------------------------------------

        float transmission = 0.0f;

        if (!(file >> token >> transmission) ||
             token != "transmission")
        {
         BOX_LOG_ERROR(
        "Failed to read transmission. Token=[" << token << "]");

        return false;
        }

        material.SetTransmission(transmission);

        // -----------------------------------------
        // IOR
        // -----------------------------------------

        float ior = 1.5f;

        if (!(file >> token >> ior) ||
           token != "ior")
        {
         BOX_LOG_ERROR(
        "Failed to read ior. Token=["
        << token << "]"
        );

        return false;
        }

        material.SetIOR(ior);

        BOX_LOG_INFO(
         "Material scalar properties loaded successfully");


        // -----------------------------------------
        // Base texture
        // -----------------------------------------
            int useBaseTexture = 0;

            if (!(file >> token >> useBaseTexture) ||
                token != "use_base_texture")
            {
                return false;
            }

            if (!(file >> token) ||
                token != "base_texture")
            {
                return false;
            }

            std::string baseTexturePath;

            if (file.peek() == ' ')
            {
                file.get();
            }

            std::getline(
                file,
                baseTexturePath
            );
           
            material.SetBaseColorTexturePath(
                baseTexturePath
            );

            material.SetUseBaseColorTexture(
                useBaseTexture != 0
            );

            // -----------------------------------------
            // Normal texture
            // -----------------------------------------
            int useNormalTexture = 0;

            if (!(file >> token >> useNormalTexture) ||
                token != "use_normal_texture")
            {
                return false;
            }

            if (!(file >> token) ||
                token != "normal_texture")
            {
                return false;
            }

            std::string normalTexturePath;

            if (file.peek() == ' ')
            {
                file.get();
            }

            std::getline(
                file,
                normalTexturePath
            );

            material.SetNormalTexturePath(
                normalTexturePath
            );

            material.SetUseNormalTexture(
                useNormalTexture != 0
            );

            float normalStrength = 1.0f;

            if (!(file >> token >> normalStrength) ||
                token != "normal_strength")
            {
                return false;
            }

            material.SetNormalStrength(
                normalStrength
            );

            // -----------------------------------------
            // End material
            // -----------------------------------------
            if (!(file >> token) ||
                token != "endmaterial")
            {
                BOX_LOG_ERROR(
                    "Expected endmaterial"
                );

                return false;
            }

            loadedMaterials.push_back(
                std::move(material)
            );
}


        // =================================================
        // BUILD EDITABLE ENTITY
        // =================================================

        auto entity = std::make_unique<Entity>(entityID, entityName);

        // -----------------------------------------
        // Restore material slots.
        // -----------------------------------------
        entity->ClearMaterialSlots();

        for (const Material& material :
            loadedMaterials)
        {
            entity->AddMaterialSlot(
                material
            );
        }

        // -----------------------------------------
        // Restore entity properties.
        // -----------------------------------------
        entity->SetPosition(position);
        entity->SetRotation(rotation);
        entity->SetScale(scale);
        entity->SetVisible(visible != 0);

        // -----------------------------------------
        // Reconstruct editable topology.
        // -----------------------------------------
        MeshEditing& mesh =
            entity->GetEditableMesh();

        mesh.Clear();

        mesh.SetShadingMode(
            static_cast<ShadingMode>(shadingMode)
        );

        // -----------------------------------------
        // Vertices
        // -----------------------------------------
        for (const glm::vec3& vertexPosition :
            loadedVertices)
        {
            mesh.AddVertex(
                vertexPosition
            );
        }

        // -----------------------------------------
        // Faces
        // -----------------------------------------
        for (const LoadedFace& loadedFace :
            loadedFaces)
        {
            const std::size_t faceIndex =
                mesh.AddFace(
                    loadedFace.vertices
                );

            mesh.GetFace(faceIndex).materialIndex =
                loadedFace.materialIndex;
        }

        // -----------------------------------------
        // Rebuild ordinary edges from the faces.
        // -----------------------------------------

        mesh.RebuildEdges();

        // -----------------------------------------
        // Restore loose edges.
        // -----------------------------------------

        for (const LoadedEdge& loadedEdge :
            loadedLooseEdges)
        {
            mesh.AddEdge(
                loadedEdge.vertexA,
                loadedEdge.vertexB,
                true
            );
        }

        // -----------------------------------------
        // Build render MeshData + GPU buffers.
        // -----------------------------------------

        if (!entity->RebuildFromEditableMesh())
        {
            BOX_LOG_ERROR(
                "Failed to rebuild loaded entity: "
                << entityName
            );

            return false;
        }

        BOX_LOG_INFO(
            "Successfully rebuilt loaded entity: "
            << entityName
            << " Vertices="
            << mesh.GetVertexCount()
            << " Faces="
            << mesh.GetFaceCount()
            << " Edges="
            << mesh.GetEdgeCount()
        );

        if (!(file >> token) ||
            token != "endentity")
        {
            BOX_LOG_ERROR(
                "Expected endentity for "
                << entityName
            );

            return false;
        }

        loadedEntities.push_back(std::move(entity));
    }

    return true;
}
