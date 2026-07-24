#include "App.h"
#include "BoxWindow.h"
#include <miniBoxLog.h>
#include <BoxEngine.h>
#include <UI/EditorIcons.h>
#include "camera/Camera.h"
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
    m_imgObjectExplorer = std::make_unique<ObjectExplorerPanel>();

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

    m_lastFrameTime = static_cast<float>(glfwGetTime());
        
    while (m_isRunning && !glfwWindowShouldClose(nativeWindow))
    {
        const float currentFrameTime = static_cast<float>(glfwGetTime());
        m_deltaTime = currentFrameTime - m_lastFrameTime;
        m_lastFrameTime = currentFrameTime;

		m_input->BeginFrame(); // Begin a new input frame for the EditorInput class mouse and keyboard input handling

        glfwPollEvents();

        glClearColor(0.15f, 0.18f, 0.22f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

		m_input->Update(); // Update input with a fixed delta time (for now)
		HandleInput(); // Handle input events

		m_imgui->BeginFrame(); // Start a new ImGui frame

        m_imgui->MainDockSpace(&dockspaceOpen);
		// ########################################### Main Menu Bar ###########################################
        		
        MenuAction menuAction = m_imgMenu->DrawMainMenu();

        HandleMenuAction(menuAction, *m_engine);


		// ############################################ Scene Viewport and Scene Collection Panels #################
        ViewportAction viewportAction = m_sceneViewport->DrawSceneViewport(*m_engine, *m_editorIcons);

        HandleViewportAction(viewportAction, *m_engine);
        
		m_imgSceneCollection->DrawSceneCollection(*m_engine); // Draw the Scene Collection panel cubes and other entities in the scene
         
		m_imgObjectExplorer->DrawObjectExplorer(*m_engine); // Draw the Object Explorer panel for the selected entity

		m_imgui->RenderImGui();

        // Engine update/render and ImGui will go here later.

        glfwSwapBuffers(nativeWindow);
    }

    Shutdown();
    return 0;
}
// handle the menu actions from the MainMenuBar
void App::HandleMenuAction(
    MenuAction action,
    BoxEngine& engine)
{
    switch (action)
    {
    case MenuAction::Exit:
        glfwSetWindowShouldClose(
            m_window->GetWindow(),
            GLFW_TRUE
        );
        break;

    case MenuAction::AddCube:
        engine.AddEditableCube();
        break;

    case MenuAction::AddSphere:
		engine.AddEditableSphere(glm::vec3(0.0f));
         
		 break;

    case MenuAction::None:
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
	case ViewportAction::AddEditableSphere:
		engine.AddEditableSphere(glm::vec3(0.0f));
		break;


    case ViewportAction::None:
    default:
        break;
    }
}

void App::HandleInput()
{
    if (!m_input ||
        !m_window ||
        !m_engine)
    {
        return;
    }
    
	// keyboard input handling for camera movement and application exit
    if (m_input->IsKeyPressed(GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(
            m_window->GetWindow(),
            GLFW_TRUE
        );
    }

    Camera& camera =
        m_engine->GetCamera();

    const bool middleMouseDown =
        m_input->IsMouseButtonDown(
            GLFW_MOUSE_BUTTON_MIDDLE
        );

    const bool shiftDown =
        m_input->IsKeyDown(
            GLFW_KEY_LEFT_SHIFT
        ) ||
        m_input->IsKeyDown(
            GLFW_KEY_RIGHT_SHIFT
        );

    const float mouseDeltaX =
        static_cast<float>(
            m_input->GetMouseDeltaX()
            );

    const float mouseDeltaY =
        static_cast<float>(
            m_input->GetMouseDeltaY()
            );

    if (middleMouseDown)
    {
        if (shiftDown)
        {
            camera.ProcessPan(
                mouseDeltaX,
                mouseDeltaY
            );
        }
        else
        {
            camera.ProcessOrbit(
                mouseDeltaX,
                -mouseDeltaY
            );
        }
    }

    const float scroll =
        static_cast<float>(
            m_input->GetScrollY()
            );

    if (scroll != 0.0f)
    {
        camera.ProcessOrbitZoom(scroll);
    }
	// use this to center the camera on the origin and reset the orbit distance to 10.0f
    if (m_input->IsKeyPressed(GLFW_KEY_HOME))
    {
        camera.FocusOn(
            glm::vec3(0.0f),
            10.0f
        );
    }

   
}


// shutdown the window and ImGui context and go to bed.
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


