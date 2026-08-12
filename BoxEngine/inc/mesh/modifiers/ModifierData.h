#pragma once
#include <cstddef>

// used to define the type of modifier and its parameters
// this will let know which modifier is being used so we can display the correct UI and apply the correct logic to the mesh data
enum class ModifierType
{
    //DESTRUCTIVE MODIFIERS

    Extrude,
    Inset,
    Bevel,
    LoopCut,
    Bridge,
    Merge,
    DeleteFace
};

enum class ModifierTypeNoneD
{
    //NON - DESTRUCTIVE MODIFIERS
    
    Mirror,
    Subdivision,
    Solidify,
    Array,
    Scatter
};



enum class ModifierAxis
{
	None,
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

