#pragma once

class BoxEngine;
class Entity;
class FaceEditController;

/*├── metal/
        ├── plastic/
        ├── glass/
        ├── wood/
        ├── stone_brick_tile/
        ├── fabric/
        ├── sky/
        ├── plant_soil/
        └── misc/ 
This will have a combo at the top but MaterialEditor.cpp will set it up*/

class MaterialDisplayPanel
{
	public:
		MaterialDisplayPanel() = default;
		~MaterialDisplayPanel();

		bool Initialize();
		// display a saved material in the material display panel on image buttons
		void Display(BoxEngine& engine, Entity& entity);
};

/*class NodeEditorPanel
{
public:

    bool Initialize();

    void Open();
    void Close();

    void Draw(
        BoxEngine& engine,
        Material& material
    );

private:

    bool m_isOpen = false;

    MaterialNodeEditor m_nodeEditor;
};*/