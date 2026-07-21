#include <BoxEngine.h>
#include "panels/ObjectExplorerPanel.h"
#include <imgui/imgui.h>
#include <imgui/ImGuiAF.h>

#include <entity/Entity.h>

void ObjectExplorerPanel::DrawObjectExplorer(BoxEngine& engine)
{

	ImGui::Begin("Object Explorer");

	ImGui::Text("Selected Entity:");

	ImGui::End();

}
