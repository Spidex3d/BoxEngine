#include <UI\EditorIcons.h>
#include <miniBoxLog.h>

bool EditorIcons::Initialize()
{
    m_vertexIcon = LoadEditorTexture("assets/textures/icons/vertex.png");

    m_edgeIcon = LoadEditorTexture("assets/textures/icons/edge.png");

    m_faceIcon = LoadEditorTexture("assets/textures/icons/face.png");

    m_materialIcon = LoadEditorTexture("assets/textures/icons/material.png");

    // tools
	m_moveToolIcon = LoadEditorTexture("assets/textures/icons/move.png");
	m_ScaleToolIcon = LoadEditorTexture("assets/textures/icons/scale.png");
	m_RotateToolIcon = LoadEditorTexture("assets/textures/icons/rotate.png");
	m_LocalToolIcon = LoadEditorTexture("assets/textures/icons/local.png");
	m_SnapToolIcon = LoadEditorTexture("assets/textures/icons/snap.png");
	m_DropToolIcon = LoadEditorTexture("assets/textures/icons/drop.png");

    const bool allIconsLoaded =
        m_vertexIcon.IsValid() &&
        m_edgeIcon.IsValid() &&
        m_faceIcon.IsValid() &&
		m_materialIcon.IsValid() &&
		m_moveToolIcon.IsValid() &&
		m_ScaleToolIcon.IsValid() &&
		m_RotateToolIcon.IsValid() &&
		m_LocalToolIcon.IsValid() &&
		m_SnapToolIcon.IsValid() &&
		m_DropToolIcon.IsValid();


    if (!allIconsLoaded)
    {
        BOX_LOG_ERROR("One or more editor icons failed to load");
        Shutdown();
        return false;
    }

    BOX_LOG_INFO("Editor icons initialized successfully");
    return true;
}

void EditorIcons::Shutdown()
{
    DestroyEditorTexture(m_vertexIcon);
    DestroyEditorTexture(m_edgeIcon);
    DestroyEditorTexture(m_faceIcon);
    DestroyEditorTexture(m_materialIcon);
	DestroyEditorTexture(m_moveToolIcon);
}

const EditorTexture& EditorIcons::GetVertexIcon() const
{
    return m_vertexIcon;
}

const EditorTexture& EditorIcons::GetEdgeIcon() const
{
    return m_edgeIcon;
}

const EditorTexture& EditorIcons::GetFaceIcon() const
{
    return m_faceIcon;
}

const EditorTexture& EditorIcons::GetMaterialIcon() const
{
    return m_materialIcon;
}

const EditorTexture& EditorIcons::GetMoveToolIcon() const
{
    return m_moveToolIcon;
}

const EditorTexture& EditorIcons::GetScaleToolIcon() const
{
	return m_ScaleToolIcon;
}

const EditorTexture& EditorIcons::GetRotateToolIcon() const
{
	return m_RotateToolIcon;
}

const EditorTexture& EditorIcons::GetLocalToolIcon() const
{
	return m_LocalToolIcon;
}

const EditorTexture& EditorIcons::GetSnapToolIcon() const
{
	return m_SnapToolIcon;
}

const EditorTexture& EditorIcons::GetDropToolIcon() const
{
	return m_DropToolIcon;
}
