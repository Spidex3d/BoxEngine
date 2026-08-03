#pragma once
#include <string>



class FileDialog
{
public:

    static std::string OpenTexture();
    static std::string OpenModel();
    static std::string SaveMBX(const char* defaultExt, const char* filter);


private:

};
