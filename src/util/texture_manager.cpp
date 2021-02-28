#include <DepthsOfPower/util/texture_manager.h>

void texture_manager::RegisterTexture(diamond& renderer, const char* referenceName, const char* filepath)
{
    std::string name = referenceName;
    idTable[name] = renderer.RegisterTexture(filepath);
}

int texture_manager::GetTextureId(const char* referenceName)
{
    std::string name = referenceName;
    if (idTable.find(name) != idTable.end())
        return idTable[name];
    else
        return 0; // default texture
}