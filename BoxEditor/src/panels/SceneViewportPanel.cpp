#include "panels\SceneViewportPanel.h"
#include <imgui/imgui.h>

void SceneViewportPanel::DrawSceneViewport()
{
    ImGui::Begin("Scene Viewport window");
    ImGui::Text("Main Scene");
    ImGui::End();

}
