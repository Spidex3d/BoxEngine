#include "App.h"
#include "BoxWindow.h"
#include <miniBoxLog.h>
#include <BoxEngine.h>
#include <UI/EditorIcons.h>
#include <EditorInput.h>
#include <imgui/imgui.h>

App::App() = default;
App::~App() = default;

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

	m_editorIcons = std::make_unique<EditorIcons>();  // add the EditorIcons class to manage the icons for the editor

    if (!m_editorIcons->Initialize())
    {
        BOX_LOG_ERROR("Failed to initialize editor icons");
        return false;
    }
      
    m_sceneViewport = std::make_unique<SceneViewportPanel>();   

    m_input = std::make_unique<EditorInput>();

    if (!m_input->Initialize(
        m_window->GetWindow()))
    {
        BOX_LOG_ERROR(
            "Failed to initialize editor input"
        );

        m_input.reset();
        return false;
    }

	// Initialize the engine after the window and ImGui have been initialized
    m_engine = std::make_unique<BoxEngine>();

    if (!m_engine->Initialize())
    {
        BOX_LOG_ERROR("Failed to initialize BoxEngine");
        m_engine.reset();
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

		m_input->Update(); // Update input with a fixed delta time (for now)
		HandleInput(); // Handle input events

		m_imgui->BeginFrame(); // Start a new ImGui frame

        m_imgui->MainDockSpace(&dockspaceOpen);
		// ########################################### Main Menu Bar ###########################################

		HandleMenuAction(m_imgMenu->DrawMainMenu());

        
		// ############################################ Scene Viewport and Scene Collection Panels #################
        ViewportAction viewportAction = m_sceneViewport->DrawSceneViewport(*m_engine, *m_editorIcons);

        HandleViewportAction(viewportAction, *m_engine);
        
		m_imgSceneCollection->DrawSceneCollection(); // Draw the Scene Collection panel
         
		m_imgui->RenderImGui();

        // Engine update/render and ImGui will go here later.

        glfwSwapBuffers(nativeWindow);
    }

    Shutdown();
    return 0;
}
// handle the menu actions from the MainMenuBar
void App::HandleMenuAction(MenuAction action)
{
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
}
// handel the viewport actions from the SceneViewportPanel
void App::HandleViewportAction(ViewportAction action, BoxEngine& engine)
{
    switch (action)
    {
    case ViewportAction::AddEditableCube:

        engine.AddEditableCube();

        break;



    case ViewportAction::None:
    default:
        break;
    }
}

void App::HandleInput()
{
    if (!m_input || !m_window)
    {
        return;
    }

    if (m_input->IsKeyPressed(GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(m_window->GetWindow(), GLFW_TRUE);
    }

    if (m_input->IsKeyPressed(GLFW_KEY_SPACE))
    {
        BOX_LOG_INFO("Space pressed");
    }

    if (m_input->IsKeyReleased(GLFW_KEY_SPACE))
    {
        BOX_LOG_INFO("Space released");
    }

    if (m_input->IsMouseButtonDown(
        GLFW_MOUSE_BUTTON_RIGHT))
    {
        BOX_LOG_INFO(
            "Mouse delta: "
            << m_input->GetMouseDeltaX()
            << ", "
            << m_input->GetMouseDeltaY()
        );
    }

    if (m_input->GetScrollY() != 0.0)
    {
        BOX_LOG_INFO(
            "Mouse wheel: "
            << m_input->GetScrollY()
        );
    }
}


void App::Shutdown()
{
    if (m_engine)
    {
        m_engine->Shutdown();
        m_engine.reset();
    }

    if (m_editorIcons)
    {
        m_editorIcons->Shutdown();
        m_editorIcons.reset();
    }

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


