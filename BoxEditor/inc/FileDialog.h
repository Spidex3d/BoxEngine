#pragma once
#include <string>



class FileDialog
{
public:

    static std::string OpenTexture();
    static std::string OpenMBX(const char* defaultExt, const char* filter);
    static std::string SaveMBX(const char* defaultExt, const char* filter);


private:

};
