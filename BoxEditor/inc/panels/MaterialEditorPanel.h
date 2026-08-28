#pragma once
#include <panels/MaterialEditor.h>
#include <memory>

class BoxEngine;
class Entity;
class FaceEditController;

//class BoxEngine;
//class Entity;
//class MaterialEditor;
//class FaceEditController;
//class EdgeEditController;

class MaterialEditorPanel
{
public:
	MaterialEditorPanel() = default;
	~MaterialEditorPanel();

	bool Initialize();	

	void Open();
	void Close();
	bool IsOpen() const;

	void Draw(BoxEngine& engine, Entity& entity, FaceEditController& faceEditController);


private:
	bool m_isOpen = false;	
	std::unique_ptr<MaterialEditor> m_materialEditor;

};

