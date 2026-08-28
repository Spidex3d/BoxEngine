#pragma once
#include <memory>
#include <string>

class BoxEngine;
class Entity;
class MaterialPreview;
class Material;
class FaceEditController;

class MaterialEditor
{
public:
    MaterialEditor();
    ~MaterialEditor();

    bool Initialize();

    void Shutdown();

    void Draw(BoxEngine& engine, Entity& entity, FaceEditController& faceEditController);

       
private:

    void DrawMaterialProperties(BoxEngine& engine, Entity& entity, Material& material);

    void DrawEmissionControls(class Material& material);

    void DrawTextureProperties(BoxEngine& engine, Entity& entity, Material& material);

    void DrawFaceMaterialProperties(BoxEngine& engine, Entity& entity, FaceEditController& faceEditController);

    

    std::unique_ptr<MaterialPreview> m_preview;

    

};
