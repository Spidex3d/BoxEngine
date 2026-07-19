#include "EditorInput.h"

#include <GLFW/glfw3.h>

EditorInput::EditorInput() = default;

EditorInput::~EditorInput()
{
    Shutdown();
}

bool EditorInput::Initialize(GLFWwindow* window)
{
    if (!window)
    {
        return false;
    }

    m_window = window;

    glfwSetWindowUserPointer(
        m_window,
        this
    );

    glfwSetScrollCallback(
        m_window,
        EditorInput::ScrollCallback
    );

    glfwGetCursorPos(
        m_window,
        &m_mouseX,
        &m_mouseY
    );

    m_previousMouseX = m_mouseX;
    m_previousMouseY = m_mouseY;

    m_firstMouseUpdate = true;

    return true;
}

void EditorInput::Shutdown()
{
    if (m_window)
    {
        /*
        Only clear these callbacks if EditorInput owns them.

        Later, if you create a shared window event system, callback
        ownership can move there.
        */
        glfwSetScrollCallback(
            m_window,
            nullptr
        );

        m_window = nullptr;
    }

    m_currentKeys.fill(false);
    m_previousKeys.fill(false);

    m_currentMouseButtons.fill(false);
    m_previousMouseButtons.fill(false);

    m_mouseDeltaX = 0.0;
    m_mouseDeltaY = 0.0;

    m_scrollX = 0.0;
    m_scrollY = 0.0;
}

void EditorInput::BeginFrame()
{
    m_previousKeys = m_currentKeys;
    m_previousMouseButtons = m_currentMouseButtons;

    m_mouseDeltaX = 0.0;
    m_mouseDeltaY = 0.0;

    m_scrollX = 0.0;
    m_scrollY = 0.0;
}

void EditorInput::Update()
{
    if (!m_window)
    {
        return;
    }

    for (int key = 0; key < KeyCount; ++key)
    {
        const int state =
            glfwGetKey(
                m_window,
                key
            );

        m_currentKeys[key] =
            state == GLFW_PRESS ||
            state == GLFW_REPEAT;
    }

    for (
        int button = 0;
        button < MouseButtonCount;
        ++button)
    {
        const int state =
            glfwGetMouseButton(
                m_window,
                button
            );

        m_currentMouseButtons[button] =
            state == GLFW_PRESS;
    }

    double newMouseX = 0.0;
    double newMouseY = 0.0;

    glfwGetCursorPos(
        m_window,
        &newMouseX,
        &newMouseY
    );

    if (m_firstMouseUpdate)
    {
        m_previousMouseX = newMouseX;
        m_previousMouseY = newMouseY;
        m_firstMouseUpdate = false;
    }

    m_mouseDeltaX =
        newMouseX - m_previousMouseX;

    m_mouseDeltaY =
        newMouseY - m_previousMouseY;

    m_mouseX = newMouseX;
    m_mouseY = newMouseY;

    m_previousMouseX = newMouseX;
    m_previousMouseY = newMouseY;
}

bool EditorInput::IsKeyDown(int key) const
{
    if (!m_enabled ||
        key < 0 ||
        key >= KeyCount)
    {
        return false;
    }

    return m_currentKeys[key];
}

bool EditorInput::IsKeyPressed(int key) const
{
    if (!m_enabled ||
        key < 0 ||
        key >= KeyCount)
    {
        return false;
    }

    return
        m_currentKeys[key] &&
        !m_previousKeys[key];
}

bool EditorInput::IsKeyReleased(int key) const
{
    if (!m_enabled ||
        key < 0 ||
        key >= KeyCount)
    {
        return false;
    }

    return
        !m_currentKeys[key] &&
        m_previousKeys[key];
}

bool EditorInput::IsMouseButtonDown(int button) const
{
    if (!m_enabled ||
        button < 0 ||
        button >= MouseButtonCount)
    {
        return false;
    }

    return m_currentMouseButtons[button];
}

bool EditorInput::IsMouseButtonPressed(int button) const
{
    if (!m_enabled ||
        button < 0 ||
        button >= MouseButtonCount)
    {
        return false;
    }

    return
        m_currentMouseButtons[button] &&
        !m_previousMouseButtons[button];
}

bool EditorInput::IsMouseButtonReleased(int button) const
{
    if (!m_enabled ||
        button < 0 ||
        button >= MouseButtonCount)
    {
        return false;
    }

    return
        !m_currentMouseButtons[button] &&
        m_previousMouseButtons[button];
}

double EditorInput::GetMouseX() const
{
    return m_mouseX;
}

double EditorInput::GetMouseY() const
{
    return m_mouseY;
}

double EditorInput::GetMouseDeltaX() const
{
    return m_enabled
        ? m_mouseDeltaX
        : 0.0;
}

double EditorInput::GetMouseDeltaY() const
{
    return m_enabled
        ? m_mouseDeltaY
        : 0.0;
}

double EditorInput::GetScrollX() const
{
    return m_enabled
        ? m_scrollX
        : 0.0;
}

double EditorInput::GetScrollY() const
{
    return m_enabled
        ? m_scrollY
        : 0.0;
}

void EditorInput::SetEnabled(bool enabled)
{
    m_enabled = enabled;
}

bool EditorInput::IsEnabled() const
{
    return m_enabled;
}

void EditorInput::ScrollCallback(
    GLFWwindow* window,
    double xOffset,
    double yOffset)
{
    auto* input =
        static_cast<EditorInput*>(
            glfwGetWindowUserPointer(window)
            );

    if (!input)
    {
        return;
    }

    input->m_scrollX += xOffset;
    input->m_scrollY += yOffset;
}