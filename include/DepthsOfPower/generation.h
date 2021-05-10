#pragma once
#include <DepthsOfPower/tilemap.h>
#include <glm/vec4.hpp>

struct biome
{
    int maxRadius; // in tiles
    f32 frequency; // a frequency of 1 means that every tile will be selected to be this biome
    f32 edgeDistortionFactor;
    int textureId;
};

struct heat_level
{
    int baseTextureId;
    std::vector<biome> biomes;
};

struct generation_parameters
{
    f32 groundStartThreshold = 0.8f; // affects when the sky ends and the ground starts generating
    f32 cavePerturbScale = 0.25f; // affects how jagged cave generation is
    f32 caveThreshold = 1.2f; // affects how big caves are (-1 to 1)
    f32 caveMaxThreshold = 0.7f; // affects the maximum size of caves regardless of depth
    f32 caveDepthScale = 1.f; // how much depth affects cave size
};

class world_generator
{
public:
    inline void SetSeed(int newSeed) { seed = newSeed; };
    void Generate(tilemap& map, bool savePreview = false);

private:
    int seed = 1337;
    void InitializeGenerationData();
    void GenerateBiomes(tilemap& map);
    std::vector<heat_level> heatLevels; // array of texture indexes representing unique heat levels
    generation_parameters genParams;

};