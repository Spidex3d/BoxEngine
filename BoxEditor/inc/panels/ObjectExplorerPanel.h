#pragma once
#include <memory>

class BoxEngine;
class Entity;
class MaterialEditor;

class ObjectExplorerPanel
{
public:
	ObjectExplorerPanel();
	~ObjectExplorerPanel();

    bool Initialize();

	void DrawObjectExplorer(BoxEngine& engine); // dedicated panel classes once they grow.

	void Shutdown();

private:
    void DrawObjectTab(
        Entity& entity
    );

    void DrawTexturesTab(
        Entity& entity
    );

    void DrawModifiersTab(
        Entity& entity
    );

    std::unique_ptr<MaterialEditor> m_materialEditor;

private:
	bool m_isOpen = false;
	int m_lastSelectedEntityID = -1;
};
