#pragma once
#include <Diamond/diamond.h>
#include <map>
#include <string>

class texture_manager
{
public:
    int RegisterTexture(diamond& renderer, const char* referenceName, const char* filepath); // wraps the diamond API
    int GetTextureId(const char* referenceName);

private:
    std::map<std::string, int> idTable;

};