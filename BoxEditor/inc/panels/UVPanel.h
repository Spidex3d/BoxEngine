#pragma once
#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <vector>

class BoxEngine;
class Entity;

class UVPanel
{
public:

	UVPanel() = default;
    ~UVPanel();

    void Open();
    void Close();

    bool IsOpen() const;

    void Draw(BoxEngine& engine);

private:
    struct UVSelection
    {
        std::size_t faceIndex = 0;
        std::size_t cornerIndex = 0;
    };

    void DrawUVLayout(Entity& entity, const ImVec2& canvasPosition, const ImVec2& canvasSize);

    bool m_isOpen = false;

    int m_projectionType = 0;

    int m_selectedFace = -1;
    int m_selectedCorner = -1;

    bool m_draggingUV = false;

    glm::vec2 m_dragStartUV{ 0.0f };

    std::vector<UVSelection> m_selectedUVs;

};
