#include "App.h"
#include "BoxWindow.h"
#include <miniBoxLog.h>
#include <imgui/imgui.h>

bool App::Init()
{
    WindowConfig config;
    config.width = 800;
    config.height = 600;
    config.title = "Box Editor Test Window";
    config.resizable = true;
    config.vsync = true;

    m_window = std::make_unique<BoxWindow>(config);

    if (!m_window->GetWindow())
    {
        BOX_LOG_DEBUG("App: Failed to create BoxWindow");
        m_window.reset();
        return false;
    }

    m_imgui = std::make_unique<ImGuiLayer>();

    m_imgui->SetEnableDocking(true);

    if (!m_imgui->Initialize(m_window->GetWindow()))
    {
        BOX_LOG_ERROR("Failed to initialize ImGui");

        m_imgui.reset();
        m_window->Shutdown();
        m_window.reset();

        return false;
    }
   
    m_isRunning = true;

    BOX_LOG_INFO("App initialized successfully");
    return true;
}

int App::Run()
{

    if (!m_window || !m_window->GetWindow())
    {
        BOX_LOG_DEBUG("App::Run called without a valid window");
        return -1;
    }

    GLFWwindow* nativeWindow = m_window->GetWindow();
        bool dockspaceOpen = true;
   
    while (m_isRunning && !glfwWindowShouldClose(nativeWindow))
    {
        glfwPollEvents();

        glClearColor(0.15f, 0.18f, 0.22f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        m_imgui->BeginFrame();

        m_imgui->MainDockSpace(&dockspaceOpen);

		m_imgui->testwindow(); // test window for ImGui    
        
		
		m_imgui->RenderImGui();

        // Engine update/render and ImGui will go here later.

        glfwSwapBuffers(nativeWindow);
    }

    Shutdown();
    return 0;
}




void App::Shutdown()
{
	// shutdown the window and ImGui context and go to bed.
    if (!m_isRunning && !m_window && !m_imgui)
        return;

    m_isRunning = false;

    // ImGui requires a valid GLFW window and OpenGL context during shutdown.
    if (m_imgui)
    {
        m_imgui->ImGuiShutdown();
        m_imgui.reset();
    }

    if (m_window)
    {
        m_window->Shutdown();
        m_window.reset();
    }

    BOX_LOG_INFO("App shutdown complete");

}


