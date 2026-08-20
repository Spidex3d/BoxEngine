#include "preview/MaterialPreview.h"
#include <entity/Entity.h>
#include <rendering/Material.h>

#include <Helpers.h>
#include <miniBoxLog.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdint>
#include <string>

MaterialPreview::~MaterialPreview()
{
    Shutdown();
}

bool MaterialPreview::Initialize(
    int width,
    int height)
{
    Shutdown();

    m_width =
        width > 0 ? width : 200;

    m_height =
        height > 0 ? height : 200;

    Helpers helpers;

    const std::string vertexShaderPath =
        helpers.GetAssetPath(
            "assets/shader/basicCube.vert"
        );

    const std::string fragmentShaderPath =
        helpers.GetAssetPath(
            "assets/shader/basicCube.frag"
        );

    m_shader =
        std::make_unique<Shader>(
            vertexShaderPath,
            fragmentShaderPath
        );

    if (!m_shader ||
        m_shader->ID() == 0)
    {
        BOX_LOG_ERROR(
            "MaterialPreview failed to create shader"
        );

        m_shader.reset();
        return false;
    }

    m_previewSphere =
        std::make_unique<Entity>(
            -1000,
            "Material Preview Sphere"
        );

    if (!m_previewSphere->CreateSphere(
        48,
        24))
    {
        BOX_LOG_ERROR(
            "MaterialPreview failed to create sphere"
        );

        m_previewSphere.reset();
        m_shader.reset();

        return false;
    }

    /*
     * create the framebuffer
     * if your Framebuffer class follows the
     * same behaviour as the scene framebuffer.
     */
   	// used for the sphere preview, so we can render the sphere with the material applied to it.
    if (!m_framebuffer.Create(
        m_width,
        m_height))
    {
        BOX_LOG_ERROR(
            "MaterialPreview failed to create framebuffer"
        );

        m_previewSphere.reset();
        m_shader.reset();

        return false;
    }


    m_initialized = true;

    BOX_LOG_INFO(
        "MaterialPreview initialized"
    );

    return true;
}

void MaterialPreview::RenderPreview(
    const Material& material)
{
    if (!m_initialized ||
        !m_shader ||
        !m_previewSphere ||
        !m_framebuffer.IsValid())
    {
        return;
    }

     // Old material path.
 // Still needed for textures, metallic,
 // roughness, etc.
    m_previewSphere->GetMaterial() =
        material;


    // New material-slot colour path.
    //
    // Preview sphere vertices use materialIndex 0,
    // so slot 0 must contain the material currently
    // being previewed.
    if (m_previewSphere
        ->GetMaterialSlotCount() > 0)
    {
        m_previewSphere
            ->GetMaterialSlot(0) =
            material;
    }

    m_framebuffer.Bind();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glClearColor(
        0.08f,
        0.08f,
        0.09f,
        1.0f
    );

    glClear(
        GL_COLOR_BUFFER_BIT |
        GL_DEPTH_BUFFER_BIT
    );

    const float aspect =
        static_cast<float>(m_width) /
        static_cast<float>(m_height);

    const glm::mat4 view =
        glm::lookAt(
            glm::vec3(
                1.5f,
                1.0f,
                2.2f
            ),
            glm::vec3(0.0f),
            glm::vec3(
                0.0f,
                1.0f,
                0.0f
            )
        );

    const glm::mat4 projection =
        glm::perspective(
            glm::radians(35.0f),
            aspect,
            0.1f,
            100.0f
        );

    m_shader->Use();

    m_shader->setVec3(
        "uLightDirection",
        glm::normalize(
            glm::vec3(
                -1.0f,
                -1.0f,
                -0.5f
            )
        )
    );

    // the new bit 
    m_previewSphere->RenderPreview(
        *m_shader,
        view,
        projection,
        glm::vec3(1.5f, 1.0f, 2.2f)
	);  

    Framebuffer::Unbind();
}

void MaterialPreview::Draw(const Material& material)
{
    if (!m_initialized)
    {
        ImGui::TextDisabled("Material preview is not initialized.");

        return;
    }

    RenderPreview(material);

	// i would like to add a list of all the materials that are currently loaded in the editor,
    // and allow the user to select one of them to preview it in the material preview window.
    
    const GLuint previewTexture = m_framebuffer.GetColorTexture();

    if (previewTexture == 0)
    {
        ImGui::TextDisabled(
            "No preview texture."
        );

        return;
    }

    const ImVec2 previewSize(static_cast<float>(m_width - 100), static_cast<float>(m_height - 100));
    // Flip framebuffer texture vertically.
	// draw the sphere preview texture in the ImGui window
    ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<intptr_t>(previewTexture)), previewSize, ImVec2(0.0f, 1.0f),ImVec2(1.0f, 0.0f));

}

void MaterialPreview::Shutdown()
{
    m_initialized = false;

    if (m_previewSphere)
    {
        m_previewSphere.reset();
    }

    m_shader.reset();

    m_framebuffer.Destroy();
}