#pragma once

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
	//int materialType = 0; // 0 = default, 1 = textured, 2 = custom shader, etc. (for future use)
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
    //void DrawMaterialList(const std::vector<MaterialEntry>& materials);
    void DrawMaterialList(BoxEngine& engine, const std::vector<MaterialEntry>& materials);
    //void DrawPreviewWindow(); // leave this in for now

    std::vector<MaterialEntry> CollectMaterials(BoxEngine& engine);

    Material* m_selectedMaterial = nullptr;

    //std::unique_ptr<MaterialPreview> m_preview;
    //Material* m_selectedMaterial = nullptr;
    //std::string m_selectedMaterialName;
    //bool m_showPreview = false;

	
};