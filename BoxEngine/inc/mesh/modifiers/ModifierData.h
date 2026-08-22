#pragma once
#include <cstddef>

// used to define the type of modifier and its parameters
// this will let know which modifier is being used so we can display the correct UI and apply the correct logic to the mesh data

// =====================================================
// Modelling operation type
// =====================================================

enum class ModifierType
{
    None,

    // Destructive modelling operations
    Extrude,
    Inset,
    LoopCut,
    Bevel,
    Bridge,
    Merge,
    DeleteFace,

    // Non-destructive modifiers
    Mirror,
    Subdivision,
    Solidify,
    Array,
    Scatter
};
enum class LastOperationType
{
    None,
    Extrude,
    Inset,
	LoopCut,
    Bevel
};


// =====================================================
// Axis
// =====================================================

enum class ModifierAxis
{
    None,
    X,
    Y,
    Z
};


// =====================================================
// Extrude
// =====================================================

struct ExtrudeModifierData
{
    std::size_t faceIndex = 0;

    ModifierAxis axis =
        ModifierAxis::None;

    float extrudeAmount =
        0.0f;
};


// =====================================================
// Inset
// =====================================================

struct InsetModifierData
{
    std::size_t faceIndex = 0;

    float insetAmount = 0.0f;
};

// ====================================================
// LoopCut
// ====================================================

struct LoopCutModifierData
{
    std::size_t edgeIndex = 0;

    float cutAmount = 0.5f;
};

// ====================================================
// Bevel
// ====================================================

struct BevelModifierData
{
    std::size_t edgeIndex = 0;
    float width = 0.1f;
    //std::size_t segments = 1;
    int segments = 1;
    float profile = 0.5f;
};

// =====================================================
// Generic modifier / operation data
// =====================================================

struct ModifierData
{
    ModifierType type = ModifierType::None;

	bool modEnabled = true; // whether the modifier is enabled or not

    ExtrudeModifierData extrude;

    InsetModifierData inset;
};

