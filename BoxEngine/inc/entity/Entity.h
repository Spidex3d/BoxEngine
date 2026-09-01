#pragma once
#include <glad\glad.h>
#include <glm\glm.hpp>
#include <glm/mat4x4.hpp>

#include <string>
#include <rendering\Material.h>
#include <mesh/MeshData.h>
#include <mesh/MeshEditing.h>
#include <mesh/modifiers/ModifierData.h>
#include <vector>
#include <cstddef>

//This is where we are putting together all the components of an entity, such as mesh, material, transform, etc.
class Shader;
class Camera;
class FaceExtrude;

enum class EntityPrimitiveType
{
    None,

    Cube,
    Plane,
    Sphere,
    Cylinder,
    Pyramid
};


class Entity
{
public:
    Entity(int id, const std::string& name);

    ~Entity();

    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    Entity(Entity&& other) noexcept;
    Entity& operator=(Entity&& other) noexcept;

	bool CreateFromMeshData(const MeshData& meshData);   // Create an entity from existing .mbx mesh data

	bool CreateCube();  // Create a cube from the new editable mesh data, buffers for rendering.
	bool CreatePlane(); // Create a Plane from the new editable mesh data, buffers for rendering.
    bool CreateSphere(int sectors = 32, int stacks = 16);
	// bool CreateIcoSphere(int recursionLevel = 2);
	bool CreateCylinder(int sectors = 32, int stacks = 1, float radius = 0.5f, float height = 1.0f); // follow to boxengine.h line 65
	// bool CreateCone(int sectors = 32, float radius = 0.5f, float height = 1.0f);
	// bool CreateTorus(int sides = 16, int rings = 32, float innerRadius = 0.2f, float outerRadius = 0.5f);
     bool CreatePyramid();

     // this just gives us a primitive type
     EntityPrimitiveType GetPrimitiveType() const
     {
         return m_primitiveType;
     }

     void SetPrimitiveType(EntityPrimitiveType type)
     {
         m_primitiveType = type;
     }



    void DrawMesh() const;
    // ##################################################################################
	// ############################# mesh editing functions #############################
    // ##################################### Vertex ########################################
    std::size_t GetVertexCount() const;

    const MeshVertex* GetVertex(std::size_t index) const;

    bool SetVertexPosition(std::size_t index, const glm::vec3& position);

    bool UploadMeshData();

	void AddSelectedVertex(std::size_t index); // Add a vertex index to the selected vertices list

    void ClearSelectedVertices();

    void SelectVertex(std::size_t index);

    bool IsVertexSelected(std::size_t index) const;

    const std::vector<std::size_t>& GetSelectedVertices() const;

    // ##################################### Edge ##########################################

	bool SetEdgePosition(std::size_t index, const glm::vec3& positionA, const glm::vec3& positionB);
	void ClearSelectedEdges();

    // ================================================================
	// ================== Single multi edge selection =================
    // ================================================================

	void AddSelectedEdge(std::size_t index); // for multi edge selection Ctrl + click

    // ================================================================
    // ==================== Loop edge selection =======================
    // ================================================================

	void AddLoopSelectedEdges(std::size_t loopEdgeIndex); // for selecting a loop of edges Alt + click

    void SelectEdge(std::size_t index);

   // void SelectEdges(std::size_t index);
    bool IsEdgeSelected(std::size_t index) const;

    const std::vector<std::size_t>&GetSelectedEdges() const
    {
        return m_selectedEdges;
    }

    // ##################################### Face ##########################################

    bool SetFacePosition(std::size_t index, const glm::vec3& positionA, const glm::vec3& positionB);
    void ClearSelectedFace();


    void SelectFace(std::size_t index);

    bool IsFaceSelected(std::size_t index) const;
    // ##################################### Mesh Edit ##########################################
    MeshEditing& GetEditableMesh()
    {
        return m_editableMesh;
    }
    // ###
    MeshEditing& GetBaseEditableMesh()
    {
        return m_baseEditableMesh;
    }

    const MeshEditing& GetBaseEditableMesh() const
    {
        return m_baseEditableMesh;
    }
    // ###
    const MeshEditing& GetEditableMesh() const
    {
        return m_editableMesh;
    }

    bool CreateBuffersFromMeshData();
    // ##################################### last Extrude  ##########################################
    void SetLastExtrude(std::size_t faceIndex, ModifierAxis axis, float amount,
        const MeshEditing& meshBeforeExtrude);

    bool HasLastExtrude() const
    {
        return m_hasLastExtrude;
    }

    const ExtrudeModifierData& GetLastExtrude() const
    {
        return m_lastExtrude;
    }

    bool UpdateLastExtrude(ModifierAxis axis, float amount);

    // ##################################### End last Extrude  ##########################################
    // ##################################################################################################
    // ##################################### last Inset  ################################################

    void SetLastInset(std::size_t faceIndex, float amount, const MeshEditing& meshBeforeInset);

    bool UpdateLastInset(float amount);

    bool HasLastInset() const
    {
        return m_hasLastInset;
    }

    const InsetModifierData& GetLastInset() const
    {
        return m_lastInset;
    }

	// ##################################### End last Inset  ##########################################
	// ##################################################################################################
	// ##################################### last LoopCut  ##########################################

    void SetLastLoopCut(std::size_t edgeIndex, float amount, const MeshEditing& meshBeforeLoopCut);

    bool HasLastLoopCut() const
    {
        return m_hasLastLoopCut;
    }

    const LoopCutModifierData& GetLastLoopCut() const
    {
        return m_lastLoopCut;
    }

    bool UpdateLastLoopCut(float amount);

    // ##################################### End last loop cut  ##########################################
    
    // ###################################################################################################
    // ##################################### last Bevel  #################################################
    // ###################################################################################################

	void SetLastBevel(std::size_t edgeIndex, float Width, int Segments, float Profile,
        const MeshEditing& meshBeforeBevel
    );

    bool HasLastBevel() const
    {
        return m_hasLastBevel;
    }

    const BevelModifierData& GetLastBevel() const
    {
        return m_lastBevel;
    }

    //bool UpdateLastBevel(float width, int segments, float profile);
    bool UpdateLastBevel(float width, int segments, float profile);

    

	// so we know which operation was last performed, for the Object Explorer panel to display the correct information.
    LastOperationType GetLastOperationType() const
    {
        return m_lastOperationType;
    }

    // ########################################################################################
    // ##################################### Normals ##########################################
	// ########### recalculate normals for the mesh, useful after editing operations. #########
    void RecalculateNormals();

    // ##################################### Modifiers ##########################################
    std::vector<ModifierData>&
        GetModifiers()
    {
        return m_modifiers;
    }

    const std::vector<ModifierData>&
        GetModifiers() const
    {
        return m_modifiers;
    }

    int GetSelectedModifierIndex() const
    {
        return m_selectedModifierIndex;
    }

    void SetSelectedModifierIndex(int index)
    {
        m_selectedModifierIndex =
            index;
    }

    bool RebuildModifiers();

	// ######################################################################################
    // ###################################### Rendering #####################################
	// ######################################################################################

	// Main render function that takes Shader and Camera objects as parameters
    void RenderScene(const Shader& shader, const Camera& camera, float aspectRatio);

	// RenderPreview function for materials sphere preview, takes Shader and Camera objects as parameters
    void RenderPreview(const Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPosition);
    

    void Destroy();

    int GetID() const
    {
        return m_id;
    }

    const std::string& GetName() const
    {
        return m_name;
    }


    void SetName(const std::string& name) {
        m_name = name;
    }

    void SetPosition(const glm::vec3& position){
        m_position = position;
    }

    void SetRotation(const glm::vec3& rotation) {
        m_rotation = rotation;
    }

    void SetScale(const glm::vec3& scale) {
        m_scale = scale;
    }

    const glm::vec3& GetPosition() const  {
        return m_position;
    }

    const glm::vec3& GetRotation() const  {
        return m_rotation;
    }

    const glm::vec3& GetScale() const  {
        return m_scale;
    }
	// #####################################################################################
	// ###################################### Cylinder #####################################
	// #####################################################################################
    void SetCylinderSectors(int sectors) {
        m_cylinderSectors = sectors;   
	}
    void SetCylinderStacks(int stacks) {
        m_cylinderStacks = stacks;
    }
    void SetCylinderRadius(float radius) {
        m_cylinderRadius = radius;
    }
    void SetCylinderHeight(float height) {
        m_cylinderHeight = height;
	}
    //#####################################
    int GetCylinderSectors() const {
        return m_cylinderSectors;
    }
    int GetCylinderStacks() const {
        return m_cylinderStacks;
    }
    float GetCylinderRadius() const {
        return m_cylinderRadius;
	}
    float GetCylinderHeight() const {   
		return m_cylinderHeight;
    }

    bool UpdateCylinder();

    // -------------------------------------------------------
	// Sphere specific parameters
    // -------------------------------------------------------
    void SetSphereSectors(int sectors) {
        m_sphereSectors = sectors; // Reusing cylinder parameters for sphere
    }
    void SetSphereStacks(int stacks) {
        m_sphereStacks = stacks; // Reusing cylinder parameters for sphere
    }
    int GetSphereSectors() const {
        return m_sphereSectors; // Reusing cylinder parameters for sphere
    }
    int GetSphereStacks() const {
        return m_sphereStacks; // Reusing cylinder parameters for sphere
	}

    

	bool UpdateSphere(); // Function to update sphere parameters and rebuild mesh if necessary


	// ###################################### Visibility #####################################
    void SetVisible(bool visible)  {
        m_visible = visible;
    }

    bool IsVisible() const  {
        return m_visible;
    }
	// ###################################### Mesh Data ###################################
    const MeshData& GetMeshData() const
    {
        return m_meshData;
    }

    // ###################################### Piking ######################################
    glm::mat4 GetModelMatrix() const;
    const glm::vec3& GetAABBMin() const;
    const glm::vec3& GetAABBMax() const;

    // ###################################### Material #####################################

    Material& GetMaterial()
    {
        return m_material;
    }

    const Material& GetMaterial() const
    {
        return m_material;
    }
    // ##############
    bool SetFaceMaterial(std::size_t faceIndex, std::size_t materialIndex);

    std::size_t AddMaterialSlot(const Material& material);

    std::size_t AddMaterialSlot()
    {
        m_materialSlots.emplace_back();

        return
            m_materialSlots.size() - 1;
    }

    std::size_t GetMaterialSlotCount() const
    {
        return m_materialSlots.size();
    }

    Material& GetMaterialSlot(
        std::size_t index
    )
    {
        return m_materialSlots.at(index);
    }

    const Material& GetMaterialSlot(
        std::size_t index
    ) const
    {
        return m_materialSlots.at(index);
    }

    void ClearMaterialSlots()
    {
        m_materialSlots.clear();
    }
private:
        // ###################################### Material #####################################
        Material m_material; // Each entity has its own material, which can be modified independently.
        std::vector<Material> m_materialSlots;

       

private: // modifiers
        // LastExtrude
        bool m_hasLastExtrude = false;
        ExtrudeModifierData m_lastExtrude;
        MeshEditing m_lastExtrudeBaseMesh;
        // LastInset
        bool m_hasLastInset = false;
        InsetModifierData m_lastInset;
		MeshEditing m_lastInsetBaseMesh;
        // LastLoopCut
        bool m_hasLastLoopCut = false;
        LoopCutModifierData m_lastLoopCut;
		MeshEditing m_lastLoopCutBaseMesh;
        // LastBevel
        bool m_hasLastBevel = false;
        BevelModifierData m_lastBevel;
		MeshEditing m_lastBevelBaseMesh;

		// This variable keeps track of the last operation performed on the mesh, which can be either extrusion or inset ect:
        LastOperationType m_lastOperationType = LastOperationType::None;

private:

    EntityPrimitiveType m_primitiveType = EntityPrimitiveType::None;

	// ############################# Mesh editing data for the entity #############################
	std::vector<std::size_t>m_selectedVertices; // Store the indices of selected vertices for editing
	std::vector<std::size_t>m_selectedEdges;    // Store the indices of selected edges for editing
	std::vector<std::size_t>m_selectedFaces;    // Store the indices of selected faces for editing
	// ############################# Mesh data for the entity ###############################
    MeshData m_meshData;
	// ############################# Mesh Editing ###############################
    MeshEditing m_baseEditableMesh; // this is the untouched starting topology.
    MeshEditing m_editableMesh;     // this is what you currently display/edit.
	// ###################################### Rendering #####################################
    void RenderInternal(const Shader& shader, const glm::mat4& view,
        const glm::mat4& projection, const glm::vec3& cameraPosition
    );
private:
	

	// ###################################### Modifiers #####################################
    std::vector<ModifierData>m_modifiers;

    int m_selectedModifierIndex = -1;
	// ###################################### Modifiers end #####################################


    glm::mat4 CalculateModelMatrix() const;

    glm::vec3 m_aabbMin = glm::vec3(-0.5f); // piking
    glm::vec3 m_aabbMax = glm::vec3(0.5f);  // piking
private:
    int m_id = -1;
    std::string m_name;
	// all entities have a position, rotation, and scale, which is used to calculate the model matrix for rendering.
    glm::vec3 m_position{ 0.0f };
    glm::vec3 m_rotation{ 0.0f };
    glm::vec3 m_scale{ 1.0f };
	//  Cylinder specific parameters
	int m_cylinderSectors = 32;
	int m_cylinderStacks = 1;
	float m_cylinderRadius = 0.5f;
	float m_cylinderHeight = 1.0f;
	// Sphere specific parameters
	int m_sphereSectors = 32;
	int m_sphereStacks = 16;

    bool m_visible = true;

    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;

    GLsizei m_vertexCount = 0;
    GLsizei m_indexCount = 0;

    bool m_useIndices = false;

   
};