#pragma once
#include <string>



class FileDialog
{
public:

    static std::string OpenTexture();

    static std::string OpenMBX(const char* defaultExt, const char* filter);
    static std::string SaveMBX(const char* defaultExt, const char* filter);

    static std::string OpenScene();
    static std::string SaveScene();

	// BoxEditor OBJ files
    static std::string SaveOBJ();

    // BoxEditor Terrain files
    static std::string SaveTerrain();
    static std::string OpenTerrain();


private:

};
