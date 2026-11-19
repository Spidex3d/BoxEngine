#include "panels/RenderPanel.h"
#include <imgui/imgui.h>
#include <miniBoxLog.h>

#include <BoxEngine.h>

bool RenderPanel::Initialize()
{
	BOX_LOG_INFO("RenderPanel initialized");

	return true;
}

void RenderPanel::Open()
{
	m_isOpen = true;
}

void RenderPanel::Close()
{
	m_isOpen = false;
}

bool RenderPanel::IsOpen() const
{
	return m_isOpen;
}

void RenderPanel::Draw(BoxEngine& engine)
{
	if (!m_isOpen)
	{
		return;
	}

	ImGui::PushID("Buttons");

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // normal
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.16f, 0.70f, 0.16f, 1.0f)); // hover
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.10f, 0.50f, 0.10f, 1.0f)); // active/click
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.8f, 1.0f)); // active/click

	ImGui::GetStyle().FrameBorderSize = 0.3f; // Add a border to the button
	ImGui::GetStyle().FrameRounding = 6.0f; // rounded corners of buttons

	if (!ImGui::Begin(
		"Render Panel",
		&m_isOpen,
		ImGuiWindowFlags_NoCollapse))
	{
		ImGui::End();
		return;
	}

	// =================================================
	// GRID ON/OFF
	// =================================================

	ImGui::SeparatorText("Grid Enabled");

	bool gridEnabled = engine.IsGridVisible();

	if (ImGui::Checkbox("Enable Grid", &gridEnabled))
	{
		engine.SetGridVisible(gridEnabled);
	}

	// =================================================
	// SAMPLING MODE
	// =================================================

	int samplingMode = 0; // temporary variable for sampling mode, default to 5

	ImGui::SeparatorText("Sampling Mode");

	ImGui::InputInt("Samples", &samplingMode);

	

	// =============================================
	// RENDER BUTTON GREEN
	// =============================================

	if (ImGui::Button("Render Scene", ImVec2(140.0f, 32.0f)))
	{
		//engine.SetGridVisible(false);
		engine.StartFinalRender();
		
	}

	ImGui::PopStyleColor(4); // Pop the 4 style colors when we pushed Render button GREEN
	

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // normal
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0.0f, 0.0f, 1.0f)); // hover
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.50f, 0.0f, 0.0f, 1.0f)); // active/click
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.8f, 1.0f)); // active/click

	// =============================================
	// CANCEL RENDER BUTTON RED
	// =============================================

	if (ImGui::Button("Cancel Render", ImVec2(140.0f, 32.0f)))
	{
		//engine.SetGridVisible(true);
		engine.CancelFinalRender();

	}

	// ============================================
	// RENDER PROGRESS
	// ============================================

	ImGui::Spacing();
	ImGui::SeparatorText("Render Progress");

	const float progress =
		engine.GetFinalRenderProgress();

	ImGui::ProgressBar(
		progress,
		ImVec2(-1.0f, 22.0f)
	);

	if (engine.IsFinalRendering())
	{
		ImGui::Text("Rendering...");
	}
	else if (progress >= 1.0f)
	{
		ImGui::Text("Render complete");
	}

	
	ImGui::End();
	ImGui::PopID();
	ImGui::PopStyleColor(4); // Pop the 4 style colors when we pushed Cancel Render button RED


}
