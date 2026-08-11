#pragma once
#include <cstddef>

enum class ModifierType
{
    Extrude,
    Inset,
    Bevel,
    LoopCut
};

enum class ModifierAxis
{
    X,
    Y,
    Z
};

struct ExtrudeModifierData
{
    std::size_t faceIndex = 0;

    ModifierAxis axis = ModifierAxis::Y;

    float amount = 0.0f;
};

struct ModifierData
{
    ModifierType type = ModifierType::Extrude;

    ExtrudeModifierData extrude;

    bool enabled = true;
};

