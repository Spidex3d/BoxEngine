#pragma once

#include <array>

struct GLFWwindow;

class EditorInput
{
public:
    EditorInput();
    ~EditorInput();

    EditorInput(const EditorInput&) = delete;
    EditorInput& operator=(const EditorInput&) = delete;

    bool Initialize(GLFWwindow* window);
    void Shutdown();

    // Call once at the beginning of every frame.
    void BeginFrame();

    // Call after glfwPollEvents().
    void Update();

    bool IsKeyDown(int key) const;
    bool IsKeyPressed(int key) const;
    bool IsKeyReleased(int key) const;

    bool IsMouseButtonDown(int button) const;
    bool IsMouseButtonPressed(int button) const;
    bool IsMouseButtonReleased(int button) const;

    double GetMouseX() const;
    double GetMouseY() const;

    double GetMouseDeltaX() const;
    double GetMouseDeltaY() const;

    double GetScrollX() const;
    double GetScrollY() const;

    void SetEnabled(bool enabled);
    bool IsEnabled() const;

private:
    static void ScrollCallback(
        GLFWwindow* window,
        double xOffset,
        double yOffset
    );

private:
    static constexpr int KeyCount = 512;
    static constexpr int MouseButtonCount = 8;

    GLFWwindow* m_window = nullptr;

    std::array<bool, KeyCount> m_currentKeys{};
    std::array<bool, KeyCount> m_previousKeys{};

    std::array<bool, MouseButtonCount> m_currentMouseButtons{};
    std::array<bool, MouseButtonCount> m_previousMouseButtons{};

    double m_mouseX = 0.0;
    double m_mouseY = 0.0;

    double m_previousMouseX = 0.0;
    double m_previousMouseY = 0.0;

    double m_mouseDeltaX = 0.0;
    double m_mouseDeltaY = 0.0;

    double m_scrollX = 0.0;
    double m_scrollY = 0.0;

    bool m_firstMouseUpdate = true;
    bool m_enabled = true;
};




