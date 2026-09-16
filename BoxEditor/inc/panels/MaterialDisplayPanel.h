#pragma once
#include <glad/glad.h>
#include <memory>
#include <unordered_map>
#include <string>
#include <preview/MaterialPreview.h>

class BoxEngine;
class Entity;
class FaceEditController;
//class MaterialPreview;

class MaterialDisplayPanel
{
	public:
		MaterialDisplayPanel() = default;
		~MaterialDisplayPanel();

		bool Initialize();

        void Open();
        void Close();
        bool IsOpen() const;

		// display a saved material in the material display panel on image buttons
		// void Draw(BoxEngine& engine, Entity& entity);
        void Draw(BoxEngine& engine, Entity& entity, FaceEditController& faceEditController);

        GLuint GetOrLoadTexture(BoxEngine& engine, const std::string& path);

private:

	bool m_isOpen = false;
    int m_directorySelected = 0;

    //std::unique_ptr<MaterialPreview>m_materialPreview;
    std::unordered_map<std::string, std::unique_ptr<MaterialPreview>> m_materialPreviews;

    std::unordered_map<std::string, GLuint>m_textureCache;

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