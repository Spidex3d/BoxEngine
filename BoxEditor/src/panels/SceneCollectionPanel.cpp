#include "panels/SceneCollectionPanel.h"
#include <imgui/imgui.h>
#include <imgui/ImGuiAF.h>

void SceneCollectionPanel::DrawSceneCollection()
{
    ImGui::Begin("Scene Collection window");
    ImGui::Text("Cube Editor");
    ImGui::End();
}
