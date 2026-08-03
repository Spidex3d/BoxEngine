#pragma once
#include <string>
#include <filesystem>
#include <imgui/imgui.h>

namespace fs = std::filesystem;

class Helpers
{
public:

	std::string GetAssetPath(const std::string& relativePath);

	static fs::path FindAssetsRoot();

	// Centers the text horizontally within the current ImGui window and applies a specified color to it.
	void CenterTextColored(const ImVec4& color, const char* text);

private:

	


};

const ImVec4 COLOR_LIGHTBLUE(0.43f, 0.7f, 0.89f, 1.0f);
const ImVec4 COLOR_LIGHTGREEN(0.0f, 0.9f, 0.0f, 1.0f);
const ImVec4 COLOR_NONE(0.0f, 0.0f, 0.0f, 0.0f);