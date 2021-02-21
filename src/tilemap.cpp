#include <DepthsOfPower/tilemap.h>
#include <DepthsOfPower/engine.h>
#include <iostream>

extern engine* Engine;

void tilemap::Draw(glm::vec2 cameraPos)
{
    int drawRangeX = 17;
    int drawRangeY = 12;
    int drawCount = (drawRangeX * 2 + 1) * (drawRangeY * 2 + 1);
    u64 centerTile = GetTileAtLocation(cameraPos);
    u64 centerTileX = centerTile % width;

    // tile index calculation even on edges
    u64 topLeftTile = 0;
    if (centerTile + (drawRangeY * width) >= tiles.size())
        topLeftTile = tiles.size() - width + centerTileX;
    else
        topLeftTile = centerTile + (drawRangeY * width);
    topLeftTile -= drawRangeX;

    std::vector<diamond_vertex> vertices(drawCount * 4);
    std::vector<u16> indices(drawCount * 6);

    const u16 baseIndices[6] = { 0, 3, 2, 2, 1, 0 };

    f32 scaleX = MetersToPixels(tileSize);
    f32 scaleY = MetersToPixels(tileSize);
    glm::vec4 color = { 1.f, 1.f, 1.f, 1.f };

    u32 index = 0;
    for (u32 y = 0; y < drawRangeY * 2 + 1; y++)
    {
        for (u32 x = 0; x < drawRangeX * 2 + 1; x++)
        {
            u64 tileIndex = topLeftTile + x - (y * width);
            if (tileIndex >= tiles.size())
                tileIndex = tiles.size() - 1;
            u32 tileX = tileIndex % width;
            u32 tileY = tileIndex / width;

            int vertexIndex = 4 * index;
            int indicesIndex = 6 * index;

            vertices[vertexIndex] =     { {(-0.5f * scaleX) + (tileX * scaleX), (-0.5f * scaleY) + (tileY * scaleY)}, color, {0.0f, 1.0f}, tiles[tileIndex].textureId };
            vertices[vertexIndex + 1] = { {(0.5f * scaleX) + (tileX * scaleX), (-0.5f * scaleY) + (tileY * scaleY)}, color, {1.0f, 1.0f}, tiles[tileIndex].textureId };
            vertices[vertexIndex + 2] = { {(0.5f * scaleX) + (tileX * scaleX), (0.5f * scaleY) + (tileY * scaleY)}, color, {1.0f, 0.0f}, tiles[tileIndex].textureId };
            vertices[vertexIndex + 3] = { {(-0.5f * scaleX) + (tileX * scaleX), (0.5f * scaleY) + (tileY * scaleY)}, color, {0.0f, 0.0f}, tiles[tileIndex].textureId };
            indices[indicesIndex] =     baseIndices[0] + vertexIndex;
            indices[indicesIndex + 1] = baseIndices[1] + vertexIndex;
            indices[indicesIndex + 2] = baseIndices[2] + vertexIndex;
            indices[indicesIndex + 3] = baseIndices[3] + vertexIndex;
            indices[indicesIndex + 4] = baseIndices[4] + vertexIndex;
            indices[indicesIndex + 5] = baseIndices[5] + vertexIndex;

            index++;
        }
    }

    diamond_transform baseTransform = diamond_transform();

    Engine->GetRenderer().BindVertices(vertices.data(), static_cast<u32>(vertices.size()));
    Engine->GetRenderer().BindIndices(indices.data(), static_cast<u32>(indices.size()));
    Engine->GetRenderer().DrawIndexed(static_cast<u32>(indices.size()), static_cast<u32>(vertices.size()), -1, baseTransform);
}

void tilemap::UpdateTile(u64 tileIndex, tile newData)
{
    tiles[tileIndex] = newData;
}

u64 tilemap::GetTileAtLocation(glm::vec2 location)
{
    u64 TileX = (u64)round((location.x) / MetersToPixels(tileSize));
    u64 FinalTile = TileX + (u64)(width * round((location.y) / MetersToPixels(tileSize)));
    if (FinalTile < 0 || FinalTile > tiles.size() - 1)
        FinalTile = 0;

    return FinalTile;
}