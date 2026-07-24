#pragma once
#include <memory>

class Entity;
class MaterialPreview;
class Material;

class MaterialEditor
{
public:
    MaterialEditor();
    ~MaterialEditor();

    bool Initialize();

    void Shutdown();

    void Draw(Entity& entity);

private:

    void DrawMaterialProperties(Entity& entity);

    void DrawEmissionControls(class Material& material);

    void DrawTextureProperties(Entity& entity);

    std::unique_ptr<MaterialPreview> m_preview;
};
