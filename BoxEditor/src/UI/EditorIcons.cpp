#include <UI\EditorIcons.h>
#include <miniBoxLog.h>

bool EditorIcons::Initialize()
{
    m_vertexIcon =
        LoadEditorTexture("assets/textures/icons/vertex.png");

    m_edgeIcon =
        LoadEditorTexture("assets/textures/icons/edge.png");

    m_faceIcon =
        LoadEditorTexture("assets/textures/icons/face.png");

    const bool allIconsLoaded =
        m_vertexIcon.IsValid() &&
        m_edgeIcon.IsValid() &&
        m_faceIcon.IsValid();

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