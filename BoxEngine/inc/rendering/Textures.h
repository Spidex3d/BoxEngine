#pragma once
#include <glad/glad.h>
#include <string>
#include <filesystem>

class Texture
{
public:
    Texture() = default;
    ~Texture();

    bool LoadFromFile(const std::string& filePath);

    void Bind(unsigned int slot = 0) const;

    void Unbind() const;

    GLuint GetID() const
    {
        return m_id;
    }

    int GetWidth() const
    {
        return m_width;
    }

    int GetHeight() const
    {
        return m_height;
    }
	// Get the source path of the texture file used in the model save and load process
    const std::filesystem::path& GetSourcePath() const
    {
        return m_sourcePath;
    }
private:
	std::filesystem::path m_sourcePath; // The source path of the texture file used in the model save and load process

private:
    GLuint m_id = 0;
    int m_width = 0;
    int m_height = 0;
    int m_channels = 0;
};
