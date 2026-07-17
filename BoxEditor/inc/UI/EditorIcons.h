#pragma once
#include <UI\EditorTexture.h>

class EditorIcons
{
public:
    EditorIcons() = default;
    ~EditorIcons() = default;

    EditorIcons(const EditorIcons&) = delete;
    EditorIcons& operator=(const EditorIcons&) = delete;

    bool Initialize();
    void Shutdown();

    const EditorTexture& GetVertexIcon() const;
    const EditorTexture& GetEdgeIcon() const;
    const EditorTexture& GetFaceIcon() const;

private:
    EditorTexture m_vertexIcon;
    EditorTexture m_edgeIcon;
    EditorTexture m_faceIcon;
};