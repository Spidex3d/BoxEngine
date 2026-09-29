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
	const EditorTexture& GetMaterialIcon() const;
    // tools
	const EditorTexture& GetMoveToolIcon() const;
	const EditorTexture& GetScaleToolIcon() const;
	const EditorTexture& GetRotateToolIcon() const;
	const EditorTexture& GetLocalToolIcon() const;
	const EditorTexture& GetSnapToolIcon() const;
	const EditorTexture& GetDropToolIcon() const;
	// Engine Start - Stop
	const EditorTexture& GetPlayIcon() const;
	const EditorTexture& GetPauseIcon() const;
	const EditorTexture& GetStopIcon() const;

private:
    EditorTexture m_vertexIcon;
    EditorTexture m_edgeIcon;
    EditorTexture m_faceIcon;
	EditorTexture m_materialIcon;
    // tools
	EditorTexture m_moveToolIcon;
	EditorTexture m_ScaleToolIcon;
	EditorTexture m_RotateToolIcon;
	EditorTexture m_LocalToolIcon;
	EditorTexture m_SnapToolIcon;
	EditorTexture m_DropToolIcon;
    // Engine Start - Stop
    EditorTexture m_PlayIcon;
	EditorTexture m_PauseIcon;
	EditorTexture m_StopIcon;

};