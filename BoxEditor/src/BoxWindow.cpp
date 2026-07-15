#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <Helpers.h>
#include <BoxDiffs.h>
#include <BoxWindow.h>
#include <miniBoxLog.h>


// initialize static refcount
int BoxWindow::s_glfwRefCount = 0;

void BoxWindow::SetIcon(GLFWwindow* window)
{
    Helpers helpers;

    std::string iconPath = helpers.GetAssetPath(ICON_PATH);
    GLFWimage images[1];
    images[0].pixels = stbi_load(iconPath.c_str(), &images[0].width, &images[0].height, 0, 4); // rgba = png
    glfwSetWindowIcon(window, 1, images);
    stbi_image_free(images[0].pixels);
}
BoxWindow::BoxWindow(const WindowConfig & config)
    : m_config(config)
{
    if (s_glfwRefCount == 0) {
        if (!glfwInit()) {
            BOX_LOG_DEBUG("BoxWindow: Failed to initialize GLFW");
            return;
        }
    }
    ++s_glfwRefCount;


    // Default to OpenGL 4.6 core; adjust if you need different version/profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, m_config.resizable ? GLFW_TRUE : GLFW_FALSE);

    window = glfwCreateWindow(m_config.width, m_config.height, m_config.title, nullptr, nullptr);
    if (!window) {
        BOX_LOG_DEBUG("BoxWindow: Failed to create GLFW window");
        --s_glfwRefCount;
        if (s_glfwRefCount == 0) glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        BOX_LOG_DEBUG("Failed to initialize GLAD");
        return;
    }
    BOX_LOG_INFO("GLAD initialized");

    BOX_LOG_INFO(
        "OpenGL Version: "
        << reinterpret_cast<const char*>(glGetString(GL_VERSION))
    );
    BOX_LOG_INFO(
        "Renderer: "
        << reinterpret_cast<const char*>(glGetString(GL_RENDERER))
    );
    // Set vsync as requested
    SetVSync(m_config.vsync);
}

BoxWindow::~BoxWindow()
{
    Shutdown();
}

void BoxWindow::Update()
{
    if (!window)
        return;

    glfwSwapBuffers(window);
}

bool BoxWindow::ShouldClose() const
{
    if (!window)
        return true;

    return glfwWindowShouldClose(window) == GLFW_TRUE;
}

void BoxWindow::SetVSync(bool enabled)
{
    if (!window) return;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(enabled ? 1 : 0);
    m_config.vsync = enabled;
}

void BoxWindow::Shutdown()
{
    if (window)
    {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    if (s_glfwRefCount > 0)
    {
        --s_glfwRefCount;

        if (s_glfwRefCount == 0)
            glfwTerminate();
    }
}
