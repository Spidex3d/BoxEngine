#pragma once

//class BoxEngine;
class Entity;
class FaceEditController;

class ParticleSystemPanel
{
public:
	ParticleSystemPanel() = default;
	~ParticleSystemPanel();

	bool Initialize();
	void Open();
	void Close();
	bool IsOpen() const;
	void Draw(Entity* entity, FaceEditController& faceEditController);

private:

	bool m_isOpen = false;




};
