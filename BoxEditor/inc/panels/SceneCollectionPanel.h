#pragma once

class BoxEngine;
class Entity;

class SceneCollectionPanel
{
public:
    SceneCollectionPanel() = default;
    ~SceneCollectionPanel() = default;

    void DrawSceneCollection(BoxEngine& engine);

    Entity* GetSelectedEntity() const;

private:
    Entity* m_selectedEntity = nullptr;
};


//
//class SceneCollectionPanel
//{
//public:
//
//	void DrawSceneCollection(); // dedicated panel classes once they grow.
//
//private:
//
//};
