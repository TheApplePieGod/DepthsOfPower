#pragma once
#include <DepthsOfPower/tilemap.h>

class world_generator
{
public:
    inline void SetSeed(int newSeed) { seed = newSeed; };
    void Generate(tilemap& map, bool savePreview = false);

private:
    int seed = 1337;

};