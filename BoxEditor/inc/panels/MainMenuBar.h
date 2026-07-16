#pragma once

// this is for the main menu bar, which is a dedicated panel in the ImGui layer.
// It handles menu actions like New, Open, Save, and Exit.
enum class MenuAction
{
    None,
    Exit,
    NewScene,
    OpenScene,
    SaveScene
};

enum class VeiwPortMenuAction
{
    None,
	EditMode,
	obgectMode,
	vertexMode,
	edgeMode,
	faceMode,
    ResetCamera,
    ImportModel,
    ExportModel
};

// dedicated panel classes once they grow.
class MainMenuBar
{
public:
	
	 MenuAction DrawMainMenu(); 
	 VeiwPortMenuAction DrawViewportMenu(); // dedicated panel classes once they grow.

private:
	// Add any private members or methods if needed

};
