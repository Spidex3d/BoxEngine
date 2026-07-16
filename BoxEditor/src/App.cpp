#include "App.h"
#include "BoxWindow.h"
#include <miniBoxLog.h>

#include <imgui/imgui.h>

bool App::Init()
{
    WindowConfig config;
    config.width = 1280;
    config.height = 720;
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

	// ################################### Initialize ImGuiLayer and panels #################################################
	m_imgui = std::make_unique<ImGuiLayer>();                         // ImGuiLayer for the main window docking and rendering
	m_imgMenu = std::make_unique<MainMenuBar>();                      // Main Menu Bar panel for the main window
	m_imgScene = std::make_unique<SceneViewportPanel>();              // Scene Viewport panel for the main window    
	m_imgSceneCollection = std::make_unique<SceneCollectionPanel>();  // Scene Collection panel for the main window

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
	m_window->SetIcon(m_window->GetWindow()); // set the window icon from the assets folder

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
		// ########################################### Main Menu Bar ###########################################
        switch (m_imgMenu->DrawMainMenu())
        {
        case MenuAction::Exit:
            glfwSetWindowShouldClose(
                m_window->GetWindow(),
                GLFW_TRUE);
            break;
        
        default:
            break;
        }
		// ########################################### Scene Viewport and Scene Collection Panels #################
		m_imgScene->DrawSceneViewport(); // Draw the Scene Viewport panel
		m_imgSceneCollection->DrawSceneCollection(); // Draw the Scene Collection panel
         
        
		
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

    if (m_imgMenu)
    {
        m_imgMenu.reset();
    }

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


