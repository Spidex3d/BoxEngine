#pragma once
#include <cstddef>
#include <vector>
#include <string>
#include <cstdint>
// used to define the type of modifier and its parameters
// this will let know which modifier is being used so we can display the correct UI and apply the correct logic to the mesh data

enum class ParticleScatterType
{
    Grass,
    Rocks,
    Trees,
    CustomMesh
};
// =====================================================
// INDIVIDUAL PARTICLE SCATTER ENTRY
// =====================================================

struct ScatterEntry
{
    // Name displayed in the Particle System panel
    std::string name = "New Particle";

    // Category
    ParticleScatterType type =
        ParticleScatterType::Grass;

    // Enable or disable this individual entry
    //bool enabled = true;
    bool enabled = false;

    // Number of clumps per square metre
    float density = 10.0f;

    // Variation
    float minScale = 0.8f;
    float maxScale = 1.2f;

    float minRotation = 0.0f;
    float maxRotation = 360.0f;

    // Independent random distribution
    std::uint32_t seedOffset = 0;

    // Which asset to scatter
    // 0 = Fine Grass
    // 1 = Meadow Grass
    // 2 = Wild Grass
    int assetID = 0;

    // =====================================================
    // GROWTH DIRECTION
    // =====================================================

    // 0.0 = Vertical growth
    // 0.5 = Partially follows the surface
    // 1.0 = Fully aligned with surface normal
    float surfaceAlignment = 0.0f;

    // =====================================================
    // GRASS CLUMP SETTINGS
    // =====================================================

    // Number of blades per clump
    int bladeCount = 7;

    // Maximum distance from clump centre (metres)
    float bladeSpread = 0.035f;

    // Individual blade height variation
    float minBladeHeight = 0.8f;
    float maxBladeHeight = 1.2f;

    // Individual blade width variation
    float minBladeWidth = 0.8f;
    float maxBladeWidth = 1.2f;

    // Outward tilt
    float minBladeTilt = 5.0f;
    float maxBladeTilt = 25.0f;


};

struct ParticleScatterData
{
    // =================================================
   // NEW MULTI-PARTICLE SYSTEM
   // =================================================

   // Every entry represents one grass, rock,
   // plant or other particle asset.
    std::vector<ScatterEntry> entries;

    // Overall seed for this modifier
    std::uint32_t seed = 12345;

    // Selected faces shared by all entries
    std::vector<std::size_t> faceIndices;

    // Maximum permitted surface slope
    float maxSlope = 45.0f;

    // =================================================
    // LEGACY SETTINGS
    // Keep these until the new renderer is working.
    // =================================================

    ParticleScatterType type =
        ParticleScatterType::Grass;

    int count = 100;

    float minScale = 0.8f;
    float maxScale = 1.2f;

    float randomRotation = 360.0f;

    
};





// =====================================================
// Modelling operation type
// =====================================================

enum class ModifierType
{
    None,

    // Destructive modelling operations
    Extrude,
    Inset,
	RoundInset,
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
	RoundInset,
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
// Inset Data
// =====================================================

struct InsetModifierData
{
    std::size_t faceIndex = 0;

    float insetAmount = 0.0f;
};
// =====================================================
// Round Inset Data
// =====================================================

struct RoundInsetModifierData
{
	std::size_t faceIndex = 0;
	float insetAmount = 0.0f;
	int segments = 1;
	float roundness = 0.5f;
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

    // New Particle Scatter
    ParticleScatterData scatter;
};

// ----------------------------------------------------------------
// Particle system modifier data
// ----------------------------------------------------------------

