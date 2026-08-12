#include "App.h"
#include "BoxWindow.h"
#include <miniBoxLog.h>
#include <BoxEngine.h>
#include <UI/EditorIcons.h>
#include "camera/Camera.h"
#include <EditorInput.h>
#include <imgui/imgui.h>
#include "panels/HelpPanel.h"


#include <FileDialog.h>

App::App() = default;
App::~App() = default;

bool App::Init()
{
    WindowConfig config;
    config.width = 1280;
    config.height = 720;
    config.title = "3d Box Editor";
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
	m_imgui = std::make_unique<ImGuiLayer>();                           // ImGuiLayer for the main window docking and rendering
	m_imgMenu = std::make_unique<MainMenuBar>();                        // Main Menu Bar panel for the main window
	m_imgScene = std::make_unique<SceneViewportPanel>();                // Scene Viewport panel for the main window    
	m_imgSceneCollection = std::make_unique<SceneCollectionPanel>();    // Scene Collection panel for the main window
	m_imgObjectExplorer = std::make_unique<ObjectExplorerPanel>();      // Object Explorer panel for the main window

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
   
    m_imgObjectExplorer = std::make_unique<ObjectExplorerPanel>();

    if (!m_imgObjectExplorer->Initialize())
    {
        BOX_LOG_ERROR(
            "ObjectExplorerPanel failed to initialize"
        );

        m_imgObjectExplorer.reset();
        return false;
    }

    if (!m_materialBrowser.Initialize())
    {
        BOX_LOG_ERROR("Failed to initialize Material Browser");
        return false;
    }
    
    m_helpPanel = std::make_unique<HelpPanel>();
   
    m_mbxManager = std::make_unique<mbxManager>();

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
         
        if (m_imgObjectExplorer)
        {
            m_imgObjectExplorer->DrawObjectExplorer(*m_engine, m_sceneViewport->GetFaceEditController());
        }

        if (m_helpPanel)
        {
            m_helpPanel->Draw();
        }

        // In your Draw() or Update() method
        m_materialBrowser.Draw(*m_engine); // Pass your BoxEngine instance

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
		// ################################## save and load mbx actions #####################################################
    case MenuAction::Exportmbx:
        {
            Entity* selectedEntity = engine.GetSelectedEntity();

            if (!selectedEntity)
            {
                BOX_LOG_ERROR("No entity selected for export");

                break;
            }

            const std::string path = FileDialog::SaveMBX("mbx", "MBX Files\0*.mbx\0All Files\0*.*\0\0");

            if (!path.empty())
            {
                m_mbxManager->ExportMBX(*selectedEntity, path);
            }

            // implement export to .mbx file format
            BOX_LOG_INFO("Export .mbx menu action triggered");
            break;
        }
		// implement import from .mbx file format
    case MenuAction::Importmbx:
    {
        const std::string path = FileDialog::OpenMBX("mbx", "MBX Files\0*.mbx\0All Files\0*.*\0\0");

        if (path.empty())
        {
            break;
        }

        MBXImportData importedData;

        if (!m_mbxManager->ImportMBX(
            path,
            importedData))
        {
            BOX_LOG_ERROR("Failed to import MBX: " << path);

            break;
        }

        Entity* importedEntity = engine.AddImportedMesh(
                importedData.objectName, importedData.mesh );

        if (!importedEntity)
        {
            break;
        }

        Material& material = importedEntity->GetMaterial();

        material.SetBaseColor(importedData.baseColor);

        material.SetMetallic(importedData.metallic);

        material.SetRoughness(importedData.roughness);

        material.SetAlpha(importedData.alpha);

        material.SetEmissionColor(importedData.emissionColor);

        material.SetEmissionStrength(importedData.emissionStrength);

        if (!importedData
            .baseColorTexturePath
            .empty())
        {
            const std::string texturePath =
                importedData
                .baseColorTexturePath
                .string();

            const GLuint textureID =
                engine.LoadTexture(
                    texturePath
                );

            if (textureID != 0)
            {
                material.SetBaseColorTexture(
                    textureID,
                    texturePath
                );

                material.SetUseBaseColorTexture(
                    true
                );
            }
            else
            {
                BOX_LOG_WARNING("MBX loaded, but texture failed: " << texturePath);
            }
        }

        BOX_LOG_INFO("Imported MBX successfully: " << path);

        break;
    }

	// ############################################ Handle other menu actions #####################################################


    case MenuAction::Exit:
        glfwSetWindowShouldClose(
            m_window->GetWindow(),
            GLFW_TRUE
        );
        break;

    case MenuAction::AddCube:
        engine.AddEditableCube();
        break;
    case MenuAction::AddPlane:
        engine.AddEditablePlane();
        break;

    case MenuAction::AddSphere:
		engine.AddEditableSphere(glm::vec3(0.0f));
         
		 break;
    case MenuAction::AddPyramid:
        engine.AddEditablePyramid(glm::vec3(0.0f));

         break;
    // ###################### Modifiers
    case MenuAction::UseExtrudeModifier:
           // Implement the logic to apply the extrude modifier to the selected entity
         BOX_LOG_INFO("Use Extrude Modifier menu action triggered");
           
		break;

    case MenuAction::Help:
        if (m_helpPanel)
        {
            m_helpPanel->Open();
        }
		BOX_LOG_INFO("Help menu action triggered");
		break;
    case MenuAction::About:

        // engine.
        BOX_LOG_INFO("About menu action triggered");
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
    case ViewportAction::AddMbxModel:
    {
        const std::string path = FileDialog::OpenMBX("mbx", "MBX Files\0*.mbx\0All Files\0*.*\0\0");

        if (path.empty())
        {
            break;
        }

        MBXImportData importedData;

        if (!m_mbxManager->ImportMBX(
            path,
            importedData))
        {
            BOX_LOG_ERROR("Failed to import MBX: " << path);

            break;
        }

        Entity* importedEntity = engine.AddImportedMesh(
            importedData.objectName, importedData.mesh);

        if (!importedEntity)
        {
            break;
        }

        Material& material = importedEntity->GetMaterial();

        material.SetBaseColor(importedData.baseColor);

        material.SetMetallic(importedData.metallic);

        material.SetRoughness(importedData.roughness);

        material.SetAlpha(importedData.alpha);

        material.SetEmissionColor(importedData.emissionColor);

        material.SetEmissionStrength(importedData.emissionStrength);

        if (!importedData
            .baseColorTexturePath
            .empty())
        {
            const std::string texturePath =
                importedData
                .baseColorTexturePath
                .string();

            const GLuint textureID =
                engine.LoadTexture(
                    texturePath
                );

            if (textureID != 0)
            {
                material.SetBaseColorTexture(
                    textureID,
                    texturePath
                );

                material.SetUseBaseColorTexture(
                    true
                );
            }
            else
            {
                BOX_LOG_WARNING("MBX loaded, but texture failed: " << texturePath);
            }
        }

        BOX_LOG_INFO("Imported MBX successfully: " << path);

        break;
    }



    case ViewportAction::AddEditableCube:

        engine.AddEditableCube();

        break;
    case ViewportAction::AddEditablePlane:

        engine.AddEditablePlane();

        break;
	case ViewportAction::AddEditableSphere:
		engine.AddEditableSphere(glm::vec3(0.0f));
		break;
    case ViewportAction::AddEditablePyramid:

		engine.AddEditablePyramid(glm::vec3(0.0f, -0.5f, 0.0f)); // -0.5f on y to sit it on the ground plane

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
    if (m_imgObjectExplorer)
    {
        m_imgObjectExplorer->Shutdown();
        m_imgObjectExplorer.reset();
    }

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
	// Shutdown the Scene help panel
    if (m_helpPanel)
    {
    m_helpPanel.reset();
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


