#pragma once
#include <memory>
#include <imgui/imgui.h>
#include <rendering/Framebuffer.h>
#include <shader/Shader.h>

class Entity;
class Material;

class MaterialPreview
{
public:
    MaterialPreview() = default;
    ~MaterialPreview();

    MaterialPreview(
        const MaterialPreview&) = delete;

    MaterialPreview& operator=(
        const MaterialPreview&) = delete;

    bool Initialize(
        int width = 200,
        int height = 200
    );

    void Render(
        const Material& material
    );

    void Draw(const Material& material);

    void Shutdown();

private:
    Framebuffer m_framebuffer;

    std::unique_ptr<Shader> m_shader;
    std::unique_ptr<Entity> m_previewSphere;

    int m_width = 200;
    int m_height = 200;

    bool m_initialized = false;


    void ButtonStyle()
    {
        ImGui::PushID("Buttons_Style");

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // normal
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.16f, 0.70f, 0.16f, 1.0f)); // hover
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.10f, 0.50f, 0.10f, 1.0f)); // active/click
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.8f, 1.0f)); // active/click

        ImGui::GetStyle().FrameBorderSize = 0.3f; // Add a border to the button
        ImGui::GetStyle().FrameRounding = 6.0f; // rounded corners of buttons
    }
    void ButtonStyleEnd()
    {
        ImGui::PopStyleColor(4);
        ImGui::PopID();
    }
};