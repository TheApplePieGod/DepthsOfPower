#pragma once
#include <DepthsOfPower/util/basic.h>
#include <vector>
#include <vec2.hpp>

struct tile
{
    int textureId;
};

class tilemap
{
public:
    tilemap(u32 mapWidth, u32 mapHeight, f32 tileSizeMeters)
    {
        width = mapWidth;
        height = mapHeight;
        tileSize = tileSizeMeters;
        tiles.resize(width * height);
        for (u64 i = 0; i < tiles.size(); i++)
        {
            tiles[i].textureId = 2;
        }
    }

    void Draw(glm::vec2 cameraPos);
    void UpdateTile(u64 tileIndex, tile newData);

    // assumes bottom left of tilemap is centered at 0,0
    u64 GetTileAtLocation(glm::vec2 location);

private:
    std::vector<tile> tiles;
    u32 width = 0;
    u32 height = 0;
    f32 tileSize = 0.f; // meters

};