#pragma once

class BoxEngine;

class RenderPanel
{
public:

	RenderPanel() = default;
	~RenderPanel() = default;

    bool Initialize();

    void Open();

    void Close();

    bool IsOpen() const;

    void Draw(BoxEngine& engine);

private:

    bool m_isOpen = false;



};
