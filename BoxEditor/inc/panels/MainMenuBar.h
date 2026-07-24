#pragma once
//#include <glad/glad.h>

// this is for the main menu bar, which is a dedicated panel in the ImGui layer.
// It handles menu actions like New, Open, Save, and Exit.
enum class MenuAction
{
    None,
    Exit,
    NewScene,
    OpenScene,
    SaveScene,

    AddCube,
    AddSphere
};



// dedicated panel classes once they grow.
class MainMenuBar
{
public:
	
	 MenuAction DrawMainMenu(); 
	

private:
	// Add any private members or methods if needed

};
