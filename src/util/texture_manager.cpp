#include <DepthsOfPower/util/texture_manager.h>

int texture_manager::RegisterTexture(diamond& renderer, const char* referenceName, const char* filepath)
{
    std::string name = referenceName;
    int id = renderer.RegisterTexture(filepath);
    idTable[name] = id;
    return id;
}

int texture_manager::GetTextureId(const char* referenceName)
{
    std::string name = referenceName;
    if (idTable.find(name) != idTable.end())
        return idTable[name];
    else
        return 0; // default texture
}