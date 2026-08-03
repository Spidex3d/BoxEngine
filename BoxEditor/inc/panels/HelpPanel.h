#pragma once

class HelpPanel
{
public:
    HelpPanel() = default;
    ~HelpPanel() = default;

    void Open();
    void Close();

    bool IsOpen() const;

    void Draw();

private:
    bool m_isOpen = false;


    

};
