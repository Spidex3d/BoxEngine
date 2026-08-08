#pragma once
#include <glad\glad.h>
#include <glm\glm.hpp>
#include <glm/mat4x4.hpp>

#include <string>
#include <rendering\Material.h>
#include <mesh/MeshData.h>
#include <mesh/MeshEditing.h>
#include <vector>
#include <cstddef>

//This is where we are putting together all the components of an entity, such as mesh, material, transform, etc.
class Shader;
class Camera;

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
	// bool CreateCylinder(int sectors = 32, int stacks = 16, float radius = 0.5f, float height = 1.0f);
	// bool CreateCone(int sectors = 32, float radius = 0.5f, float height = 1.0f);
	// bool CreateTorus(int sides = 16, int rings = 32, float innerRadius = 0.2f, float outerRadius = 0.5f);
     bool CreatePyramid();

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
    void SelectEdge(std::size_t index);

   // void SelectEdges(std::size_t index);
    bool IsEdgeSelected(std::size_t index) const;

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

    const MeshEditing& GetEditableMesh() const
    {
        return m_editableMesh;
    }

    bool CreateBuffersFromMeshData();

    // ##################################### Normals ##########################################
    void RecalculateNormals();


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

    void SetPosition(const glm::vec3& position)
    {
        m_position = position;
    }

    void SetRotation(const glm::vec3& rotation)
    {
        m_rotation = rotation;
    }

    void SetScale(const glm::vec3& scale)
    {
        m_scale = scale;
    }

    const glm::vec3& GetPosition() const
    {
        return m_position;
    }

    const glm::vec3& GetRotation() const
    {
        return m_rotation;
    }

    const glm::vec3& GetScale() const
    {
        return m_scale;
    }

    void SetVisible(bool visible)
    {
        m_visible = visible;
    }

    bool IsVisible() const
    {
        return m_visible;
    }
	// ###################################### Mesh Data ###################################
	//Material GetMeshData() const;
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
private:
	// ############################# Mesh editing data for the entity #############################
	std::vector<std::size_t>m_selectedVertices; // Store the indices of selected vertices for editing
	std::vector<std::size_t>m_selectedEdges;    // Store the indices of selected edges for editing
	std::vector<std::size_t>m_selectedFaces;    // Store the indices of selected faces for editing
	// ############################# Mesh data for the entity ###############################
    MeshData m_meshData;
	// ############################# Mesh Editing ###############################
    MeshEditing m_editableMesh;
	// ###################################### Rendering #####################################
    void RenderInternal(const Shader& shader, const glm::mat4& view,
        const glm::mat4& projection, const glm::vec3& cameraPosition
    );
private:
    Material m_material; // Each entity has its own material, which can be modified independently.



    glm::mat4 CalculateModelMatrix() const;

    glm::vec3 m_aabbMin = glm::vec3(-0.5f); // piking
    glm::vec3 m_aabbMax = glm::vec3(0.5f);  // piking
private:
    int m_id = -1;
    std::string m_name;

    glm::vec3 m_position{ 0.0f };
    glm::vec3 m_rotation{ 0.0f };
    glm::vec3 m_scale{ 1.0f };

    bool m_visible = true;

    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;

    GLsizei m_vertexCount = 0;
    GLsizei m_indexCount = 0;

    bool m_useIndices = false;

   
};