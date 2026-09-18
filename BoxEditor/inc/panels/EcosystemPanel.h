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
	
	// -------------------------------- floor generator ----------------
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

	// ------------------------------ Rock Generator ----------------
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


	// Rocks, floor, water, terrain, grass, plants & tree generation

private:
	bool m_isOpen = false;
	// ----------------- Ecosystem Tabs -----------------
	void FloorTab(BoxEngine& engine);
	EcoSystemAction RocksTab(BoxEngine& engine);
	void TerrainTab(BoxEngine& engine);
	void WaterTab(BoxEngine& engine);
	void PlantsTab(BoxEngine& engine);
	void GrassTab(BoxEngine& engine);
	void TreesTab(BoxEngine& engine);
	void SkyTab(BoxEngine& engine);
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


};
