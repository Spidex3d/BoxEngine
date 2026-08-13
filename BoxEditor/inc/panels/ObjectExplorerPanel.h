#pragma once
#include <memory>

class BoxEngine;
class Entity;
class MaterialEditor;
class FaceEditController;


class ObjectExplorerPanel
{
public:
	ObjectExplorerPanel();
	~ObjectExplorerPanel();

    bool Initialize();

    void DrawObjectExplorer(BoxEngine& engine, FaceEditController& faceEditController);

	void Shutdown();

private:
    char nameBuffer[128]{};

    void DrawObjectTab(BoxEngine& engine,
        Entity& entity
    );

    void DrawTexturesTab(BoxEngine& engine,
        Entity& entity
    );
    
	// modifiers tab for extrude and inset ect. controls, with the ability to confirm or cancel the operation.
    void DrawModifiersTab(BoxEngine& engine, Entity& entity, FaceEditController& faceEditController);

    void ExtrudeControls(Entity& entity, FaceEditController& faceEditController);
    void InsetControls(Entity& entity, FaceEditController& faceEditController);




    std::unique_ptr<MaterialEditor> m_materialEditor;

private:
	bool m_isOpen = false;
	int m_lastSelectedEntityID = -1;
};
