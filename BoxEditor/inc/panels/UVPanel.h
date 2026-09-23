#pragma once

class BoxEngine;

class UVPanel
{
public:

	UVPanel() = default;
    ~UVPanel();

    void Open();
    void Close();

    bool IsOpen() const;

    void Draw(BoxEngine& engine);

private:
    bool m_isOpen = false;

    int m_projectionType = 0;

};
