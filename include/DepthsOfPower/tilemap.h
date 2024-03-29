#pragma once
#include <DepthsOfPower/util/basic.h>
#include <vector>
#include <glm/vec2.hpp>

struct tile
{
    int textureId;
};

enum tile_connection: u16
{
    Surrounded = 0,
    Isolated = 1,
    Top = 2,
    Right = 3,
    Bottom = 4,
    Left = 5,
    TopRight = 6,
    RightBottom = 7,
    BottomLeft = 8,
    LeftTop = 9,
    TopRightBottom = 10,
    RightBottomLeft = 11,
    BottomLeftTop = 12,
    LeftTopRight = 13,
    BottomTop = 14,
    LeftRight = 15,
};

class tilemap
{
public:

    tilemap(u32 mapWidth, u32 mapHeight, f32 tileSizeMeters);

    void Draw(glm::vec2 cameraPos);
    bool IsColliding(glm::vec2 colliderPos, glm::vec2 colliderExtent);
    void UpdateTile(u64 tileIndex, tile newData);
    void DebugSaveMapToFile(bool transparent);

    inline u32 GetWidth() { return width; };
    inline u32 GetHeight() { return height; };
    inline u32 GetSize() { return width * height; };

    // assumes bottom left of tilemap is centered at 0,0
    u64 GetTileAtLocation(glm::vec2 location);
    glm::vec2 GetWorldLocationOfTile(u64 tileIndex);
    u64 GetTopLeftTileOfRange(glm::vec2 center, glm::vec2 range);
    u64 RayTraceForTile(glm::vec2 startPos, glm::vec2 direction, int max); // set max to 0 for as far as possible, direction should be normalized (+y is up)
    tile_connection GetTileConnection(u64 tileIndex);

private:
    std::vector<tile> tiles;
    u32 width = 0;
    u32 height = 0;
    f32 tileSize = 0.f; // meters

};