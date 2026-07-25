#include "preview/MaterialPreview.h"
#include <entity/Entity.h>
#include <rendering/Material.h>

#include <Helpers.h>
#include <miniBoxLog.h>

//#include <imgui/imgui.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdint>
#include <string>

//void ButtonStyle()
//{
//    ImGui::PushID("Material_Buttons");
//
//    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // normal
//    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.16f, 0.70f, 0.16f, 1.0f)); // hover
//    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.10f, 0.50f, 0.10f, 1.0f)); // active/click
//    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.8f, 1.0f)); // active/click
//
//    ImGui::GetStyle().FrameBorderSize = 0.3f; // Add a border to the button
//    ImGui::GetStyle().FrameRounding = 6.0f; // rounded corners of buttons
//}
//void ButtonStyleEnd()
//{
//    ImGui::PopStyleColor(4);
//    ImGui::PopID();
//}



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

void MaterialPreview::Render(
    const Material& material)
{
    if (!m_initialized ||
        !m_shader ||
        !m_previewSphere ||
        !m_framebuffer.IsValid())
    {
        return;
    }

    /*
     * Copy the currently edited material onto
     * the private preview sphere.
     */
    m_previewSphere->GetMaterial() =
        material;

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

    m_previewSphere->Render(
        *m_shader,
        view,
        projection
    );

    Framebuffer::Unbind();
}

void MaterialPreview::Draw(const Material& material)
{
    if (!m_initialized)
    {
        ImGui::TextDisabled(
            "Material preview is not initialized."
        );

        return;
    }

    Render(material);

    const GLuint previewTexture =
        m_framebuffer.GetColorTexture();

    if (previewTexture == 0)
    {
        ImGui::TextDisabled(
            "No preview texture."
        );

        return;
    }

    const ImVec2 previewSize(
        static_cast<float>(m_width),
        static_cast<float>(m_height)
    );
    // Flip framebuffer texture vertically.
	// draw the sphere preview texture in the ImGui window
    ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<intptr_t>(previewTexture)), previewSize, ImVec2(0.0f, 1.0f),ImVec2(1.0f, 0.0f));

	ButtonStyle(); // Apply custom button style

	ImGui::Spacing();
    if(ImGui::Button("Save Material", ImVec2(90.0f, 0.0f)))
    {
		// open a file dialog to save the material to a file, or save it to a default location.
	}
	ImGui::SameLine();
    if (ImGui::Button("Load Material", ImVec2(90.0f, 0.0f))) {
		// open a file dialog to load a material from a file, or load it from a default location.
    }

	ButtonStyleEnd(); // End custom button style


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