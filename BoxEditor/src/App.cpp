#include "App.h"
#include "BoxWindow.h"
#include <miniBoxLog.h>

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

    m_isRunning = true;

    BOX_LOG_INFO("App initialized successfully");
    return true;
}

bool App::Run()
{

    if (!m_window || !m_window->GetWindow())
    {
        BOX_LOG_DEBUG("App::Run called without a valid window");
        return -1;
    }

    GLFWwindow* nativeWindow = m_window->GetWindow();

    while (m_isRunning && !glfwWindowShouldClose(nativeWindow))
    {
        glfwPollEvents();

       // glClearColor(0.15f, 0.18f, 0.22f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT);

        // Engine update/render and ImGui will go here later.

        glfwSwapBuffers(nativeWindow);
    }

    Shutdown();
    return 0;
}




void App::Shutdown()
{
	// shutdown the window and ImGui context and go to bed.
    if (!m_isRunning && !m_window)
        return;

    m_isRunning = false;

    if (m_window)
    {
        m_window->Shutdown();
        m_window.reset();
    }

    BOX_LOG_INFO("App shutdown complete");
}


