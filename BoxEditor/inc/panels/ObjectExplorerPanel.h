#pragma once


class BoxEngine;
class Entity;

class ObjectExplorerPanel
{
public:
	ObjectExplorerPanel() = default;
	~ObjectExplorerPanel() = default;

	void DrawObjectExplorer(BoxEngine& engine); // dedicated panel classes once they grow.

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

private:
	bool m_isOpen = false;
	int m_lastSelectedEntityID = -1;
};
