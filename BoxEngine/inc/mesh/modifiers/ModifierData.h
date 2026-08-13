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
    Inset
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

    float insetAmount =
        0.0f;
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


//enum class ModifierType
//{
//    //DESTRUCTIVE MODIFIERS
//    Extrude,
//    Inset,
//    Bevel,
//    LoopCut,
//    Bridge,
//    Merge,
//    DeleteFace
//};
//
//enum class ModifierTypeNoneD
//{
//    //NON - DESTRUCTIVE MODIFIERS
//    Mirror,
//    Subdivision,
//    Solidify,
//    Array,
//    Scatter
//};
//
//enum class ModifierAxis
//{
//	None,
//    X,
//    Y,
//    Z
//};
//
//struct ExtrudeModifierData
//{
//    std::size_t faceIndex = 0;
//    ModifierAxis axis = ModifierAxis::Y;
//	float extrudeAmount = 0.0f; // changed from amount to extrudeAmount to be more specific
//};
//
//struct InsetModifierData
//{
//    std::size_t faceIndex = 0;
//    ModifierAxis axis = ModifierAxis::Y;
//    float insetAmount = 0.0f;
//};
//
//struct ModifierData
//{
//	ModifierType type = ModifierType::Extrude; // type of modifier, default to Extrude
//    ExtrudeModifierData extrude;
//	bool extrudeEnabled = true; // changed from enabled to extrudeEnabled to be more specific
//
//	ModifierType type1 = ModifierType::Inset; // type1 of modifier, default to Inset
//	InsetModifierData inset;
//	bool insetEnabled = true; // changed from enabled to insetEnabled to be more specific
//
//
//};

