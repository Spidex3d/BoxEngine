#pragma once

enum class MenuAction
{
    None,
    Exit,
    NewScene,
    OpenScene,
    SaveScene
};

// dedicated panel classes once they grow.
class MainMenuBar
{
public:
	
	 MenuAction DrawMainMenu(); 

private:
	// Add any private members or methods if needed

};

//MenuAction DrawMainMenu(); // dedicated panel classes once they grow.
   /*void DrawSceneViewport();
   void DrawObjectExplorer();
   void DrawInspector();*/