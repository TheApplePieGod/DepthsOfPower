#include <DepthsOfPower/tilemap.h>
#include <DepthsOfPower/engine.h>
#include <iostream>
#include <map>
#include <glm/vec3.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

extern engine* Engine;

tilemap::tilemap(u32 mapWidth, u32 mapHeight, f32 tileSizeMeters)
{
    width = mapWidth;
    height = mapHeight;
    tileSize = tileSizeMeters;
    tiles.resize(width * height);
}

bool tilemap::IsColliding(glm::vec2 colliderPos, glm::vec2 colliderExtent)
{
    u64 topLeftTile = GetTileAtLocation({ colliderPos.x - colliderExtent.x, colliderPos.y + colliderExtent.y });
    u64 topRightTile = GetTileAtLocation({ colliderPos.x + colliderExtent.x, colliderPos.y + colliderExtent.y });
    u64 bottomLeftTile = GetTileAtLocation({ colliderPos.x - colliderExtent.x, colliderPos.y - colliderExtent.y });

    if (topLeftTile < 0)
        topLeftTile = 0;
    if (topRightTile < 0)
        topRightTile = 0;
    if (bottomLeftTile < 0)
        bottomLeftTile = 0;

    u64 xRange = topRightTile - topLeftTile + 1;
    u64 yRange = ((topLeftTile - bottomLeftTile) / width) + 1;
    bool collision = false;
    for (int y = 0; y < yRange; y++)
    {
        for (int x = 0; x < xRange; x++)
        {
            u64 tileIndex = topLeftTile + x - (y * width);
            if (tileIndex >= tiles.size())
                tileIndex = tiles.size() - 1;

            if (tiles[tileIndex].textureId != 0) // has collision
            {
                collision = true;
                break;
            }
        }
        if (collision)
            break;
    }

    return collision;
}

u64 tilemap::GetTopLeftTileOfRange(glm::vec2 center, glm::vec2 range)
{
    u64 centerTile = GetTileAtLocation(center);
    u64 centerTileX = centerTile % width;

    // tile index calculation even on edges
    u64 topLeftTile = 0;
    if (centerTile + (range.y * width) >= tiles.size())
        topLeftTile = tiles.size() - width + centerTileX;
    else
        topLeftTile = centerTile + (range.y * width);
    topLeftTile -= range.x;

    return topLeftTile;
}

void tilemap::Draw(glm::vec2 cameraPos)
{
    int drawRangeX = 17;
    int drawRangeY = 12;
    int drawCount = (drawRangeX * 2 + 1) * (drawRangeY * 2 + 1);
    
    u64 topLeftTile = GetTopLeftTileOfRange(cameraPos, { drawRangeX, drawRangeY });

    std::vector<diamond_vertex> vertices(drawCount * 4);
    std::vector<u16> indices(drawCount * 6);

    const u16 baseIndices[6] = { 0, 3, 2, 2, 1, 0 };
    const f32 numVariations = 16.f;
    const f32 variationStep = 1.f / numVariations;
    const f32 epsilonX = 0.008f;
    const f32 epsilonY = 0.12f;
    const glm::vec2 baseTextureCoords[4] = { {0.0f + epsilonX, 1.0f - epsilonY}, {1.0f / numVariations - epsilonX, 1.0f - epsilonY}, {1.0f / numVariations - epsilonX, 0.0f + epsilonY}, {0.0f + epsilonX, 0.0f + epsilonY} };

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

            if (tiles[tileIndex].textureId != 0)
            {
                u32 tileX = tileIndex % width;
                u32 tileY = tileIndex / width;

                int vertexIndex = 4 * index;
                int indicesIndex = 6 * index;

                tile_connection connection = GetTileConnection(tileIndex);

                vertices[vertexIndex] =     { {(-0.5f * scaleX) + (tileX * scaleX), (-0.5f * scaleY) + (tileY * scaleY)}, color, baseTextureCoords[0], tiles[tileIndex].textureId };
                vertices[vertexIndex + 1] = { {(0.5f * scaleX) + (tileX * scaleX), (-0.5f * scaleY) + (tileY * scaleY)}, color, baseTextureCoords[1], tiles[tileIndex].textureId };
                vertices[vertexIndex + 2] = { {(0.5f * scaleX) + (tileX * scaleX), (0.5f * scaleY) + (tileY * scaleY)}, color, baseTextureCoords[2], tiles[tileIndex].textureId };
                vertices[vertexIndex + 3] = { {(-0.5f * scaleX) + (tileX * scaleX), (0.5f * scaleY) + (tileY * scaleY)}, color, baseTextureCoords[3], tiles[tileIndex].textureId };
                vertices[vertexIndex].texCoord.x     += variationStep * static_cast<f32>(connection);
                vertices[vertexIndex + 1].texCoord.x += variationStep * static_cast<f32>(connection);
                vertices[vertexIndex + 2].texCoord.x += variationStep * static_cast<f32>(connection);
                vertices[vertexIndex + 3].texCoord.x += variationStep * static_cast<f32>(connection);
                indices[indicesIndex] =     baseIndices[0] + vertexIndex;
                indices[indicesIndex + 1] = baseIndices[1] + vertexIndex;
                indices[indicesIndex + 2] = baseIndices[2] + vertexIndex;
                indices[indicesIndex + 3] = baseIndices[3] + vertexIndex;
                indices[indicesIndex + 4] = baseIndices[4] + vertexIndex;
                indices[indicesIndex + 5] = baseIndices[5] + vertexIndex;
                index++;
            }
        }
    }

    diamond_transform baseTransform = diamond_transform();

    Engine->GetRenderer().BindVertices(vertices.data(), static_cast<u32>(vertices.size()));
    Engine->GetRenderer().BindIndices(indices.data(), static_cast<u32>(indices.size()));
    Engine->GetRenderer().DrawIndexed(static_cast<u32>(indices.size()), static_cast<u32>(vertices.size()), -1, baseTransform);
}

void tilemap::DebugSaveMapToFile(bool transparent)
{
    unsigned char* pixelData = new unsigned char[width * height * 4];
    std::map<int, glm::vec3> colorMap;

    // manual color overrides
    colorMap[Engine->GetTextureManager().GetTextureId("dirt")] = { 138, 84, 52 };
    colorMap[Engine->GetTextureManager().GetTextureId("stone")] = { 100, 100, 100 };
    colorMap[Engine->GetTextureManager().GetTextureId("hot_stone")] = { 109, 90, 90 };
    colorMap[Engine->GetTextureManager().GetTextureId("cold_stone")] = { 90, 90, 109 };
    colorMap[Engine->GetTextureManager().GetTextureId("limestone")] = { 184, 173, 53 };
    colorMap[Engine->GetTextureManager().GetTextureId("marble")] = { 157, 175, 175 };
    colorMap[Engine->GetTextureManager().GetTextureId("granite")] = { 20, 60, 60 };
    colorMap[Engine->GetTextureManager().GetTextureId("basalt")] = { 20, 20, 20 };
    colorMap[Engine->GetTextureManager().GetTextureId("iron_ore")] = { 150, 135, 120 };
    colorMap[Engine->GetTextureManager().GetTextureId("copper_ore")] = { 194, 119, 48 };

    u64 pixelIndex = width * 4 * (height - 1);
    for (u64 i = 0; i < tiles.size(); i++)
    {
        glm::vec3 color;
        if (tiles[i].textureId == 0)
            color = { 255.f, 255.f, 255.f };
        else
        {
            if (colorMap.count(tiles[i].textureId) == 0)
            {
                color = { rand() * 255, rand() * 255, rand() * 255 };
                colorMap[tiles[i].textureId] = color;
            }
            else
                color = colorMap[tiles[i].textureId];
        }

        pixelData[pixelIndex] = (int)color.x;
        pixelData[pixelIndex + 1] = (int)color.y;
        pixelData[pixelIndex + 2] = (int)color.z;
        pixelData[pixelIndex + 3] = (tiles[i].textureId == 0 && transparent ? 0 : 255);

        pixelIndex += 4;
        if (pixelIndex % (width * 4) == 0)
            pixelIndex -= width * 4 * 2;
    }

    stbi_write_png("map.png", width, height, 4, pixelData, width * 4);
    delete[] pixelData;
}

void tilemap::UpdateTile(u64 tileIndex, tile newData)
{
    tiles[tileIndex] = newData;
}

glm::vec2 tilemap::GetWorldLocationOfTile(u64 tileIndex)
{
    u32 tileX = tileIndex % width;
    u32 tileY = tileIndex / width;
    return { tileX * MetersToPixels(tileSize), tileY * MetersToPixels(tileSize) };
}

u64 tilemap::GetTileAtLocation(glm::vec2 location)
{
    u64 TileX = (u64)round((location.x) / MetersToPixels(tileSize));
    u64 FinalTile = TileX + (u64)(width * round((location.y) / MetersToPixels(tileSize)));
    if (FinalTile < 0 || FinalTile > tiles.size() - 1)
        FinalTile = 0;

    return FinalTile;
}

u64 tilemap::RayTraceForTile(glm::vec2 startPos, glm::vec2 direction, int max)
{
    int count = 0;
    u64 currentTile = GetTileAtLocation(startPos);
    while ((max == 0 || count < max) && currentTile >= 0 && currentTile < tiles.size())
    {
        if (tiles[currentTile].textureId != 0)
            break; 

        u32 tileX = currentTile % width;
        u32 tileY = currentTile / width;
        if (tileX + (int)direction.x >= width || tileX + (int)direction.x < 0)
            break;
        if (tileY + (int)direction.y >= tiles.size() || tileY + (int)direction.y < 0)
            break;

        currentTile += (int)direction.x;
        currentTile = (signed)(currentTile + width * (int)direction.y);

        count++;
    }
    return currentTile;
}

tile_connection tilemap::GetTileConnection(u64 tileIndex)
{
    int id = tiles[tileIndex].textureId;
    bool connectedBottom = tileIndex >= width && tiles[tileIndex - width].textureId == id;
    bool connectedTop = tileIndex < width * (height - 1) && tiles[tileIndex + width].textureId == id;
    bool connectedLeft = tileIndex > 0 && tiles[tileIndex - 1].textureId == id;
    bool connectedRight = tileIndex < tiles.size() - 1 && tiles[tileIndex + 1].textureId == id;

    if (connectedBottom && connectedTop && connectedLeft && connectedRight)
        return tile_connection::Surrounded;
    if (!connectedBottom && !connectedTop && !connectedLeft && !connectedRight)
        return tile_connection::Isolated;
    if (!connectedBottom && connectedTop && !connectedLeft && !connectedRight)
        return tile_connection::Top;
    if (!connectedBottom && !connectedTop && !connectedLeft && connectedRight)
        return tile_connection::Right;
    if (connectedBottom && !connectedTop && !connectedLeft && !connectedRight)
        return tile_connection::Bottom;
    if (!connectedBottom && !connectedTop && connectedLeft && !connectedRight)
        return tile_connection::Left;
    if (!connectedBottom && connectedTop && !connectedLeft && connectedRight)
        return tile_connection::TopRight;
    if (connectedBottom && !connectedTop && !connectedLeft && connectedRight)
        return tile_connection::RightBottom;
    if (connectedBottom && !connectedTop && connectedLeft && !connectedRight)
        return tile_connection::BottomLeft;
    if (!connectedBottom && connectedTop && connectedLeft && !connectedRight)
        return tile_connection::LeftTop;
    if (connectedBottom && connectedTop && !connectedLeft && connectedRight)
        return tile_connection::TopRightBottom;
    if (connectedBottom && !connectedTop && connectedLeft && connectedRight)
        return tile_connection::RightBottomLeft;
    if (connectedBottom && connectedTop && connectedLeft && !connectedRight)
        return tile_connection::BottomLeftTop;
    if (!connectedBottom && connectedTop && connectedLeft && connectedRight)
        return tile_connection::LeftTopRight;
    if (connectedBottom && connectedTop && !connectedLeft && !connectedRight)
        return tile_connection::BottomTop;
    if (!connectedBottom && !connectedTop && connectedLeft && connectedRight)
        return tile_connection::LeftRight;
    return tile_connection::Isolated;
}