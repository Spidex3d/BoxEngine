#include <iostream>
#include <miniBoxLog.h>
#include <BoxDiffs.h>
#include <Helpers.h>
#include <ImGuiLayer.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/ImGuiAF.h>

#include <GLFW/glfw3.h>

bool ImGuiLayer::Initialize(GLFWwindow* window)
{

	Helpers helpers; // from Helpers.h, used to get asset paths - fonts, textures etc.
    // ImGui set up
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // enable viewports/docking depending on flag
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    if (m_enableDocking) {
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    }

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char* glsl_version = "#version 460 core";
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Fonts
    ImFontConfig fontconfig;
    fontconfig.MergeMode = true;
    fontconfig.PixelSnapH = true;
    static const ImWchar ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };


    fontconfig.GlyphOffset = ImVec2(0.0f, 1.0f);
	std::string fontPath = helpers.GetAssetPath(FONT_PATH_MAIN_REL); // comic sans font
    io.Fonts->AddFontFromFileTTF(fontPath.c_str(), FONT_SIZE);

    std::string AFfontPath = helpers.GetAssetPath(FA_SOLID_PATH);
    io.Fonts->AddFontFromFileTTF(AFfontPath.c_str(), FONT_SIZE, &fontconfig, ranges);

    m_initialized = true;
    BOX_LOG_INFO("ImGui initialized successfully");
    return true;
}

void ImGuiLayer::BeginFrame()
{
    // New Frame
    if (!m_initialized)
        return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::RenderImGui()
{
    
    if (!m_initialized)
        return;

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backupContext = glfwGetCurrentContext();

        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();

        glfwMakeContextCurrent(backupContext);
    }
}

void ImGuiLayer::ImGuiShutdown()
{
    if (!m_initialized)
        return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    m_initialized = false;

    BOX_LOG_INFO("ImGui shutdown complete");
}
