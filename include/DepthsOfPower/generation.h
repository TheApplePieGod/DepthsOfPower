#pragma once
#include <DepthsOfPower/tilemap.h>
#include <glm/vec4.hpp>

struct biome
{
    int maxRadius; // in tiles
    f32 frequency; // a frequency of 1 means that every tile will be selected to be this biome
    f32 edgeDistortionFactor;
    glm::vec2 depthRange = glm::vec2(0.f, 1.f); // (highest, lowest) 1(top)-0(bottom) range of where this biome is allowed to spawn
    int textureId;
};

class world_generator
{
public:
    inline void SetSeed(int newSeed) { seed = newSeed; };
    void Generate(tilemap& map, bool savePreview = false);
    void Test(tilemap& map);

private:
    int seed = 1337;
    void GenerateBiomes(tilemap& map, const std::vector<f32>& biomeNoise);
    void SetupMap(tilemap& map, const std::vector<f32>& basicNoise);
    void CreateCaves(tilemap& map, const std::vector<f32>& basicNoise, const std::vector<f32>& caveNoise);

};