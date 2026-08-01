#pragma once
#include <memory>
#include <vector>
#include <string>

class BoxEngine;
class Entity;
class MaterialPreview;
class Material;

struct MaterialEntry
{
    Material* material = nullptr;
    std::string displayName;
    int ownerEntityID = -1;
};

class MaterialBrowser
{
public:
    MaterialBrowser();
    ~MaterialBrowser();

    bool Initialize();
    void Shutdown();

    void Draw(BoxEngine& engine);
  
private:
    void DrawMaterialList(BoxEngine& engine, const std::vector<MaterialEntry>& materials);
    void DrawPreviewWindow(); // leave this in

    std::vector<MaterialEntry> CollectMaterials(BoxEngine& engine);


    std::unique_ptr<MaterialPreview> m_preview;
    Material* m_selectedMaterial = nullptr;
    std::string m_selectedMaterialName;
    bool m_showPreview = false;

	
};