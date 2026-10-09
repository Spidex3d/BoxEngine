#pragma once
#include <memory>


class BoxEngine;
// use this for the button actions in the ecosystem panel, to select which type of ecosystem element to generate
enum class EcoSystemAction
{
	None,
	AddFloor,
	Addterrain,
	AddRocks,
	AddWater,
	AddPlants,
	AddGrass,
	AddTrees,
	JoinTree,
	AddSky,
	AddEnvironment

};


class EcosystemPanel
{
public:
	EcosystemPanel() = default;
	~EcosystemPanel();

	bool Initialize();

	void Open();
	

	void Close();
	

	bool IsOpen() const;
	
	//EcoSystemAction Draw(BoxEngine& engine, Entity& entity);
	EcoSystemAction Draw(BoxEngine& engine);
	
	// ----------------------------------------------------------------------------------------
	// floor generator 
	// ---------------------------------------------------
	float GetFloorWidth() const
	{
		return m_floorWidth;
	}

	float GetFloorDepth() const
	{
		return m_floorDepth;
	}

	int GetFloorSubdivisionsX() const
	{
		return m_floorSubdivisionsX;
	}

	int GetFloorSubdivisionsZ() const
	{
		return m_floorSubdivisionsZ;
	}
	// ------------------------------ floor generator End ----------------

	// -----------------------------------------------------------------------------------
	// Rock Generator 
	// --------------------------------------------
	int GetRockSeed() const
	{
		return m_rockSeed;
	}

	float GetRockRadius() const
	{
		return m_rockRadius;
	}

	int GetRockSubdivisions() const
	{
		return m_rockSubdivisions;
	}

	float GetRockRoughness() const
	{
		return m_rockRoughness;
	}

	float GetRockFlattening() const
	{
		return m_rockFlattening;
	}
	// ------------------------------ Rock Generator End ----------------

	// ---------------------------------------------------------------------------------------------------------
	// Tree Generator
	// -------------------------------------------------

	int GetTreeSeed() const
	{
		return m_treeSeed;
	}

	float GetTreeTrunkHeight() const
	{
		return m_treeTrunkHeight;
	}

	float GetTreeTrunkRadius() const
	{
		return m_treeTrunkRadius;
	}

	int GetTreeBranchCount() const
	{
		return m_treeBranchCount;
	}

	float GetTreeBranchLength() const
	{
		return m_treeBranchLength;
	}

	float GetTreeBranchAngle() const
	{
		return m_treeBranchAngle;
	}

	float GetTreeLeafSize() const
	{
		return m_treeLeafSize;
	}

	// -----------------------------------------------------------------------------------------------------------
	// Grass Generator
	// -----------------------------------------------------
	int GetGrassSeed() const
	{
		return m_grassSeed;
	}
	float GetGrassClumpSize() const
	{
		return m_grassClumpSize;
	}
	int GetGrassClumpCount() const
	{
		return m_grassClumpCount;
	}
	int GetGrassClumpDensity() const
	{
		return m_grassClumpDensity;
	}
	float GetGrassClumpCurve() const
	{
		return m_grassClumpCurve;
	}
	int GetGrassClumpRandomness() const
	{
		return m_grassClumpRandomness;
	}
	float GetGrassClumpRotation() const
	{
		return m_grassClumpRotation;
	}
	float GetGrassClumpScale() const
	{
		return m_grassClumpScale;
	}
	float GetGrassClumpOffset() const
	{
		return m_grassClumpOffset;
	}
	// clump randomness, clump rotation, on / off
	bool GetGrassRandomDirection() const
	{
		return m_grassRndDir;
	}
	// clump size, clump count, clump density, clump curve, clump randomness,
	// clump rotation, clump scale, clump offset, 




private:
	bool m_isOpen = false;
	// ----------------- Ecosystem Tabs -----------------
	void FloorTab(BoxEngine& engine);			 // Floor	
	EcoSystemAction RocksTab(BoxEngine& engine); // Rocks
	void TerrainTab(BoxEngine& engine);
	void WaterTab(BoxEngine& engine);
	void PlantsTab(BoxEngine& engine);
	EcoSystemAction GrassTab(BoxEngine& engine); // Grass
	EcoSystemAction TreesTab(BoxEngine& engine); // Trees
	EcoSystemAction SkyTab(BoxEngine& engine);
	void EnvironmentTab(BoxEngine& engine);

	// ---------------- floor generator ----------------
	
	// Floor settings
	float m_floorWidth = 20.0f;
	float m_floorDepth = 20.0f;

	int m_floorSubdivisionsX = 20;
	int m_floorSubdivisionsZ = 20;
	// ---------------- floor generator End -------------

	// ---------------- Rock Generator ----------------

	int m_rockSeed = 1234;

	float m_rockRadius = 1.0f;

	int m_rockSubdivisions = 2;

	float m_rockRoughness = 0.22f;

	float m_rockFlattening = 0.20f;

	// -------------- Rock Generator End --------------

	// ---------------- Grass Generator ----------------
	int m_grassSeed = 1234;
	int m_grassClumpCount = 10;
	float m_grassClumpSize = 1.0f;
	int m_grassClumpDensity = 5;
	float m_grassClumpCurve = 0.5f;
	int m_grassClumpRandomness = 2;
	float m_grassClumpRotation = 0.0f;
	float m_grassClumpScale = 1.0f;
	float m_grassClumpOffset = 0.0f;
	bool m_grassRndDir = false;

	// ---------------- Grass Generator End --------------

	// ---------------- Tree Generator ----------------

	int m_treeSeed = 1234;

	float m_treeTrunkHeight = 3.0f;

	float m_treeTrunkRadius = 0.40f;

	int m_treeBranchCount = 3;

	float m_treeBranchLength = 0.65f;

	float m_treeBranchAngle = 80.0f;

	float m_treeLeafSize = 1.0f;

	// -------------- Tree Generator End --------------

};
