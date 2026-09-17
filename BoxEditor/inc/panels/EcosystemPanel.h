#pragma once

class BoxEngine;
class Entity;

class EcosystemPanel
{
public:
	EcosystemPanel() = default;
	~EcosystemPanel();

	bool Initialize();

	void Open();
	

	void Close();
	

	bool IsOpen() const;
	
	void Draw(BoxEngine& engine, Entity& entity);

	// Rocks, floor, water, terrain, grass, plants & tree generation

private:

	void FloorTab(BoxEngine& engine, Entity& entity);
	void RocksTab(BoxEngine& engine, Entity& entity);
	void TerrainTab(BoxEngine& engine, Entity& entity);
	void WaterTab(BoxEngine& engine, Entity& entity);
	void PlantsTab(BoxEngine& engine, Entity& entity);
	void GrassTab(BoxEngine& engine, Entity& entity);
	void TreesTab(BoxEngine& engine, Entity& entity);


	bool m_isOpen = false;

};
