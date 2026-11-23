#pragma once
#include <memory>

class BoxEngine;
class Entity;
class MaterialEditor;
class FaceEditController;
class EdgeEditController;
class VertexEditController;

class ObjectExplorerPanel
{
public:
	ObjectExplorerPanel();
	~ObjectExplorerPanel();

    bool Initialize();

    void DrawObjectExplorer(BoxEngine& engine, FaceEditController& faceEditController, EdgeEditController& edgeEditController,
        VertexEditController& vertexEditController);

	void Shutdown();

private:
    char nameBuffer[128]{};

    void DrawObjectTab(BoxEngine& engine, Entity& entity);

	// NOT IN USE FOR NOW, MATERIAL EDITOR PANEL IS USED INSTEAD go to line 100 in ObjectExplorerPanel.cpp
    /*void DrawTexturesTab(BoxEngine& engine, Entity& entity,
        FaceEditController& faceEditController);*/
    void DrawLightingTab(BoxEngine& engine);
    
	// modifiers tab for extrude and inset ect. controls, with the ability to confirm or cancel the operation.
    void DrawModifiersTab(BoxEngine& engine, Entity& entity, FaceEditController& faceEditController, EdgeEditController& edgeEditController,
        VertexEditController& vertexEditController);

    void ExtrudeControls(Entity& entity, FaceEditController& faceEditController);
    void InsetControls(Entity& entity, FaceEditController& faceEditController);
    void RoundInsetControls(Entity& entity, FaceEditController& faceEditController);
	void LoopCutControls(Entity& entity, EdgeEditController& edgeEditController);
	void BevelControls(Entity& entity, EdgeEditController& edgeEditController);
    void MirrorControls(Entity& entity);
    // Precise vertex movement controls
    void VertexMoveControls(Entity& entity, VertexEditController& vertexEditController);
    // Axis selected in the precise vertex move panel.
    // Default to X.
    int m_vertexMoveAxis = 1;

    void EdgeMoveControls(Entity& entity, EdgeEditController& edgeEditController);
    // 1 = X, 2 = Y, 3 = Z, 4 = Normal
    int m_edgeMoveAxis = 1;

    std::unique_ptr<MaterialEditor> m_materialEditor;

private:
	bool m_isOpen = false;
	int m_lastSelectedEntityID = -1;
};
