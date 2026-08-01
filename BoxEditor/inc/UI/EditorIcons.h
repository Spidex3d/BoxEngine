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
    // tools
	const EditorTexture& GetMoveToolIcon() const;
	const EditorTexture& GetScaleToolIcon() const;
	const EditorTexture& GetRotateToolIcon() const;
	const EditorTexture& GetLocalToolIcon() const;
	const EditorTexture& GetSnapToolIcon() const;

private:
    EditorTexture m_vertexIcon;
    EditorTexture m_edgeIcon;
    EditorTexture m_faceIcon;
    // tools
	EditorTexture m_moveToolIcon;
	EditorTexture m_ScaleToolIcon;
	EditorTexture m_RotateToolIcon;
	EditorTexture m_LocalToolIcon;
	EditorTexture m_SnapToolIcon;

};