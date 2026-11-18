#include <fileManager\SceneSerializer.h>
#include <entity/Entity.h>
#include <miniBoxLog.h>

#include <fstream>
#include <iomanip>


namespace
{
    // Read a named field from our scene file.
    //
    // Example:
    // seed 12345
    //
    // ReadSceneField(file, "seed", scatter.seed);

    template<typename... Args>
    bool ReadSceneField(
        std::istream& input,
        const char* expected,
        Args&... values)
    {
        std::string token;

        if (!(input >> token))
            return false;

        if (token != expected)
        {
            BOX_LOG_ERROR(
                "Expected [" << expected
                << "] but found [" << token << "]"
            );

            return false;
        }

        return static_cast<bool>(
            (input >> ... >> values)
            );
    }

    bool ReadSceneMarker(
        std::istream& input,
        const char* expected)
    {
        std::string token;

        if (!(input >> token))
            return false;

        if (token != expected)
        {
            BOX_LOG_ERROR(
                "Expected [" << expected
                << "] but found [" << token << "]"
            );

            return false;
        }

        return true;
    }
}


namespace
{
    bool ReadScatterModifier(
        std::istream& input,
        ModifierData& modifier)
    {
        if (!ReadSceneMarker(
            input, "scatter_modifier"))
            return false;

        modifier.type = ModifierType::Scatter;

        ParticleScatterData& scatter =
            modifier.scatter;

        int enabled = 0;

        if (!ReadSceneField(input, "enabled", enabled))
            return false;

        modifier.modEnabled = enabled != 0;

        if (!ReadSceneField(input, "seed", scatter.seed))
            return false;

        if (!ReadSceneField(input, "max_slope", scatter.maxSlope))
            return false;

        if (!ReadSceneField(input, "min_scale", scatter.minScale))
            return false;

        if (!ReadSceneField(input, "max_scale", scatter.maxScale))
            return false;

        if (!ReadSceneField(input, "random_rotation",
            scatter.randomRotation))
            return false;

        if (!ReadSceneField(input, "rock_surface_offset",
            scatter.rockSurfaceOffset))
            return false;

        if (!ReadSceneField(input, "grass_clearance",
            scatter.grassClearance))
            return false;

        // ========================================
        // SELECTED FACES
        // ========================================

        std::size_t faceCount = 0;

        if (!ReadSceneField(
            input, "selected_faces", faceCount))
            return false;

        if (faceCount > 1000000)
            return false;

        scatter.faceIndices.clear();

        for (std::size_t i = 0; i < faceCount; ++i)
        {
            std::size_t faceIndex = 0;

            if (!(input >> faceIndex))
                return false;

            scatter.faceIndices.push_back(faceIndex);
        }

        // ========================================
        // SCATTER ENTRIES
        // ========================================

        std::size_t entryCount = 0;

        if (!ReadSceneField(
            input, "scatter_entries", entryCount))
            return false;

        if (entryCount > 10000)
            return false;

        scatter.entries.clear();
        scatter.entries.reserve(entryCount);

        for (std::size_t i = 0; i < entryCount; ++i)
        {
            if (!ReadSceneMarker(
                input, "scatter_entry"))
                return false;

            ScatterEntry entry;

            // Read the quoted name.
            if (!ReadSceneMarker(
                input, "entry_name"))
                return false;

            if (!(input >> std::quoted(entry.name)))
                return false;

            int entryType = 0;
            int entryEnabled = 0;

            if (!ReadSceneField(
                input, "entry_type", entryType))
                return false;

            if (entryType < 0 || entryType > 3)
                return false;

            entry.type =
                static_cast<ParticleScatterType>(entryType);

            if (!ReadSceneField(
                input, "entry_enabled", entryEnabled))
                return false;

            entry.enabled = entryEnabled != 0;

            if (!ReadSceneField(
                input, "asset_id", entry.assetID))
                return false;

            if (!ReadSceneField(
                input, "rock_count", entry.rockCount))
                return false;

            if (!ReadSceneField(
                input, "density", entry.density))
                return false;

            if (!ReadSceneField(
                input, "entry_min_scale", entry.minScale))
                return false;

            if (!ReadSceneField(
                input, "entry_max_scale", entry.maxScale))
                return false;

            if (!ReadSceneField(
                input, "min_rotation", entry.minRotation))
                return false;

            if (!ReadSceneField(
                input, "max_rotation", entry.maxRotation))
                return false;

            if (!ReadSceneField(
                input, "seed_offset", entry.seedOffset))
                return false;

            if (!ReadSceneField(
                input, "surface_alignment",
                entry.surfaceAlignment))
                return false;

            // ====================================
            // GRASS SETTINGS
            // ====================================

            if (!ReadSceneField(
                input, "blade_count", entry.bladeCount))
                return false;

            if (!ReadSceneField(
                input, "blade_spread", entry.bladeSpread))
                return false;

            if (!ReadSceneField(
                input, "broken_percentage",
                entry.brokenBladePercentage))
                return false;

            if (!ReadSceneField(
                input, "min_blade_height",
                entry.minBladeHeight))
                return false;

            if (!ReadSceneField(
                input, "max_blade_height",
                entry.maxBladeHeight))
                return false;

            if (!ReadSceneField(
                input, "min_blade_width",
                entry.minBladeWidth))
                return false;

            if (!ReadSceneField(
                input, "max_blade_width",
                entry.maxBladeWidth))
                return false;

            if (!ReadSceneField(
                input, "min_blade_tilt",
                entry.minBladeTilt))
                return false;

            if (!ReadSceneField(
                input, "max_blade_tilt",
                entry.maxBladeTilt))
                return false;

            // ====================================
            // GRASS BLADE SHAPE
            // ====================================

            if (!ReadSceneField(
                input, "blade_curvature",
                entry.bladeCurvature))
                return false;

            if (!ReadSceneField(
                input, "blade_concavity",
                entry.bladeConcavity))
                return false;

            if (!ReadSceneField(
                input, "blade_break_position",
                entry.bladeBreakPosition))
                return false;

            if (!ReadSceneField(
                input, "blade_break_angle",
                entry.bladeBreakAngle))
                return false;

            if (!ReadSceneMarker(
                input, "endscatter_entry"))
                return false;

            scatter.entries.push_back(
                std::move(entry)
            );
        }

        return ReadSceneMarker(
            input, "endscatter_modifier");
    }
}





SceneSerializer::~SceneSerializer() = default;

bool SceneSerializer::SerializeScene(
    const std::filesystem::path& filePath,
    const std::vector<std::unique_ptr<Entity>>& entities, const SceneSkyData& skyData)
{
    std::filesystem::path finalPath = filePath;

    

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

    BOX_LOG_INFO("Scene file opened for writing: " << (file.is_open() ? "YES" : "NO"));


    if (!file.is_open())
    {
        BOX_LOG_ERROR(
            "Failed to create scene file: "
            << finalPath.string()
        );

        return false;
    }

    file << std::fixed << std::setprecision(6);

    // ---------------------------------------------
    // Scene header.
    // ---------------------------------------------
    file << "# BoxEditor Scene\n";
   // file << "version 0 1\n\n";
    file << "version "
        << MBSENE_VERSION_MAJOR << ' '
        << MBSENE_VERSION_MINOR << "\n\n";


    // ============================================
    // SAVE SKYBOX
    // ============================================

    file << "skybox\n";

    file << "enabled "
        << (skyData.enabled ? 1 : 0)
        << '\n';

    file << "texture_path "
        << std::quoted(skyData.texturePath)
        << '\n';

    file << "endskybox\n\n";


    file << "entities " << entities.size() << "\n\n";



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

namespace
{
    void WriteScatterModifier(
        std::ostream& output,
        const ModifierData& modifier)
    {
        const ParticleScatterData& scatter =
            modifier.scatter;

        output << "scatter_modifier\n";

        output << "enabled "
            << (modifier.modEnabled ? 1 : 0) << '\n';

        output << "seed "
            << scatter.seed << '\n';

        output << "max_slope "
            << scatter.maxSlope << '\n';

        output << "min_scale "
            << scatter.minScale << '\n';

        output << "max_scale "
            << scatter.maxScale << '\n';

        output << "random_rotation "
            << scatter.randomRotation << '\n';

        output << "rock_surface_offset "
            << scatter.rockSurfaceOffset << '\n';

        output << "grass_clearance "
            << scatter.grassClearance << '\n';

        // -----------------------------------------
        // SELECTED FACES
        // -----------------------------------------

        output << "selected_faces "
            << scatter.faceIndices.size();

        for (std::size_t faceIndex :
        scatter.faceIndices)
        {
            output << ' ' << faceIndex;
        }

        output << '\n';

        // -----------------------------------------
        // INDIVIDUAL GRASS AND ROCK ENTRIES
        // -----------------------------------------

        output << "scatter_entries "
            << scatter.entries.size() << '\n';

        for (const ScatterEntry& entry :
            scatter.entries)
        {
            output << "scatter_entry\n";

            output << "entry_name "
                << std::quoted(entry.name) << '\n';

            output << "entry_type "
                << static_cast<int>(entry.type) << '\n';

            output << "entry_enabled "
                << (entry.enabled ? 1 : 0) << '\n';

            output << "asset_id "
                << entry.assetID << '\n';

            output << "rock_count "
                << entry.rockCount << '\n';

            output << "density "
                << entry.density << '\n';

            output << "entry_min_scale "
                << entry.minScale << '\n';

            output << "entry_max_scale "
                << entry.maxScale << '\n';

            output << "min_rotation "
                << entry.minRotation << '\n';

            output << "max_rotation "
                << entry.maxRotation << '\n';

            output << "seed_offset "
                << entry.seedOffset << '\n';

            output << "surface_alignment "
                << entry.surfaceAlignment << '\n';

            // Grass clump settings

            output << "blade_count "
                << entry.bladeCount << '\n';

            output << "blade_spread "
                << entry.bladeSpread << '\n';

            output << "broken_percentage "
                << entry.brokenBladePercentage << '\n';

            output << "min_blade_height "
                << entry.minBladeHeight << '\n';

            output << "max_blade_height "
                << entry.maxBladeHeight << '\n';

            output << "min_blade_width "
                << entry.minBladeWidth << '\n';

            output << "max_blade_width "
                << entry.maxBladeWidth << '\n';

            output << "min_blade_tilt "
                << entry.minBladeTilt << '\n';

            output << "max_blade_tilt "
                << entry.maxBladeTilt << '\n';

            // Grass shape settings

            output << "blade_curvature "
                << entry.bladeCurvature << '\n';

            output << "blade_concavity "
                << entry.bladeConcavity << '\n';

            output << "blade_break_position "
                << entry.bladeBreakPosition << '\n';

            output << "blade_break_angle "
                << entry.bladeBreakAngle << '\n';

            output << "endscatter_entry\n";
        }

        output << "endscatter_modifier\n";
    }
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
            << "use_base_texture  "
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

    // =================================================
    // SURFACE SCATTER MODIFIERS
    // =================================================

    const auto& modifiers = entity.GetModifiers();

    std::size_t scatterModifierCount = 0;

    for (const ModifierData& modifier : modifiers)
    {
        if (modifier.type == ModifierType::Scatter)
        {
            ++scatterModifierCount;
        }
    }

    // Save the number of Surface Scatter modifiers.
    output << "scatter_modifiers "
        << scatterModifierCount << '\n';

    // Save each Surface Scatter modifier.
    for (const ModifierData& modifier : modifiers)
    {
        if (modifier.type != ModifierType::Scatter)
            continue;

        WriteScatterModifier(output, modifier);
    }

    // =================================================
    // END ENTITY
    // =================================================

    output << "endentity\n\n";

    return output.good();


}

bool SceneSerializer::DeserializeScene(const std::filesystem::path& filePath, std::vector<std::unique_ptr<Entity>>& loadedEntities,
    SceneSkyData& skyData)
{
    loadedEntities.clear();
    skyData = SceneSkyData{};
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
    // version 0 3
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


    // ============================================
    // CHECK SUPPORTED SCENE VERSION
    // ============================================

    BOX_LOG_INFO(
        "Reading scene version: "
        << versionMajor << "." << versionMinor
    );

    // We support versions 0.1, 0.2 and 0.3.
    if (versionMajor != 0 ||
        (versionMinor != 1 &&
            versionMinor != 2 &&
            versionMinor != 3))
    {
        BOX_LOG_ERROR(
            "Unsupported scene version: "
            << versionMajor << "."
            << versionMinor
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


    // ============================================
    // LOAD SKYBOX - VERSION 0.3
    // ============================================

    if (versionMinor >= 3)
    {
        std::string skyToken;

        if (!(file >> skyToken) ||
            skyToken != "skybox")
        {
            BOX_LOG_ERROR("Expected skybox block");
            return false;
        }

        int skyEnabled = 0;

        if (!(file >> skyToken >> skyEnabled) ||
            skyToken != "enabled")
        {
            BOX_LOG_ERROR("Failed to read sky enabled");
            return false;
        }

        skyData.enabled = (skyEnabled != 0);

        if (!(file >> skyToken) ||
            skyToken != "texture_path")
        {
            BOX_LOG_ERROR("Expected sky texture_path");
            return false;
        }

        if (!(file >> std::quoted(skyData.texturePath)))
        {
            BOX_LOG_ERROR("Failed to read sky texture path");
            return false;
        }

        if (!(file >> skyToken) ||
            skyToken != "endskybox")
        {
            BOX_LOG_ERROR("Expected endskybox");
            return false;
        }

        BOX_LOG_INFO(
            "Scene skybox: Enabled="
            << skyData.enabled
            << " Path="
            << skyData.texturePath
        );
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
            BOX_LOG_ERROR("Failed to read entity name");

            return false;
        }

        // Remove only the space after "name".
        if (file.peek() == ' ')
        {
            file.get();
        }

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
            BOX_LOG_ERROR("Failed to read primitive type");

            return false;
        }

        // -----------------------------------------
        // Visibility
        // -----------------------------------------
        int visible = 1;

        if (!(file >> token >> visible) ||
            token != "visible")
        {
            BOX_LOG_ERROR("Failed to read entity visibility");

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
            
            BOX_LOG_INFO(
                "Loaded base texture settings: Use="
                << useBaseTexture
                << " Path=["
                << baseTexturePath
                << "]"
            );


            material.SetBaseColorTexturePath(baseTexturePath);

            material.SetUseBaseColorTexture(useBaseTexture != 0);
            

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

            BOX_LOG_INFO(
                "Loaded normal texture settings: Use="
                << useNormalTexture
                << " Path=["
                << normalTexturePath
                << "]"
            );

            material.SetNormalTexturePath(normalTexturePath);

            material.SetUseNormalTexture(useNormalTexture != 0);

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

            
            loadedMaterials.push_back(std::move(material));
        }


        // =================================================
        // LOAD SURFACE SCATTER MODIFIERS
        // =================================================

        std::vector<ModifierData> loadedScatterModifiers;

        // Version 0.1 files do not contain modifiers.
        // Version 0.2 files contain the new scatter block.
        if (versionMinor >= 2)
        {
            std::size_t modifierCount = 0;

            if (!ReadSceneField(
                file, "scatter_modifiers", modifierCount))
            {
                BOX_LOG_ERROR(
                    "Failed to read scatter modifier count"
                );

                return false;
            }

            if (modifierCount > 10000)
                return false;

            for (std::size_t modifierIndex = 0;
                modifierIndex < modifierCount;
                ++modifierIndex)
            {
                ModifierData modifier;

                if (!ReadScatterModifier(file, modifier))
                {
                    BOX_LOG_ERROR(
                        "Failed to read scatter modifier "
                        << modifierIndex
                    );

                    return false;
                }

                // Ensure all saved face indices are valid
                // for the mesh we just loaded.
                for (std::size_t faceIndex :
                modifier.scatter.faceIndices)
                {
                    if (faceIndex >= loadedFaces.size())
                    {
                        BOX_LOG_ERROR(
                            "Invalid scatter face index: "
                            << faceIndex
                        );

                        return false;
                    }
                }

                loadedScatterModifiers.push_back(
                    std::move(modifier)
                );
            }

            BOX_LOG_INFO(
                "Loaded "
                << loadedScatterModifiers.size()
                << " Surface Scatter modifiers"
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
            BOX_LOG_ERROR("Failed to rebuild loaded entity: " << entityName);

            return false;
        }

        // ============================================
        // RESTORE SURFACE SCATTER MODIFIERS
        // ============================================

        // The entity's editable mesh has now been
        // reconstructed, so its face indices exist.

        auto& entityModifiers = entity->GetModifiers();

        entityModifiers.insert(
            entityModifiers.end(),
            loadedScatterModifiers.begin(),
            loadedScatterModifiers.end()
        );

        BOX_LOG_INFO(
            "Restored "
            << loadedScatterModifiers.size()
            << " scatter modifiers to "
            << entityName
        );

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
