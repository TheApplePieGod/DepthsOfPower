#include <DepthsOfPower/engine.h>
#include <fstream>
#include <iostream>

extern engine* Engine;

void skeleton::Initialize(const char* path)
{
    std::ifstream ifs(path);
    nlohmann::json jObj;
    ifs >> jObj;

    identifier = jObj["name"];

    nlohmann::json rootBoneJson = jObj["bones"][0];
    ParseBoneJson(rootBoneJson, -1);

    Engine->GetRenderer().SyncTextureUpdates();
}

void skeleton::Draw(diamond_transform baseTransform)
{
    glm::vec2 locationMeters = { PixelsToMeters(baseTransform.location.x), PixelsToMeters(baseTransform.location.y) };
    DrawBone(0, locationMeters); // root
}

void skeleton::DrawBone(int boneIndex, glm::vec2 bonePos)
{
    diamond& renderer = Engine->GetRenderer();
    bone& bone = bones[boneIndex];

    glm::vec2 direction = { cos(DegreesToRadians(bone.currentRotation)), sin(DegreesToRadians(bone.currentRotation)) };
    glm::vec2 boneEnd = bonePos + bone.currentPosition + direction * bone.length;
    diamond_transform boneTransform;
    //boneTransform.location = (boneEnd + bonePos) * 0.5f; // midpoint
    boneTransform.location = bonePos;
    boneTransform.location = { MetersToPixels(boneTransform.location.x), MetersToPixels(boneTransform.location.y) };
    boneTransform.rotation = -bone.currentRotation;
    boneTransform.scale = { MetersToPixels(bone.size.x), MetersToPixels(bone.size.y) };

    renderer.DrawQuad(bone.textureId, boneTransform); 

    for (int childIndex : bone.children)
    {
        DrawBone(childIndex, boneEnd);
    }
}

int skeleton::ParseBoneJson(nlohmann::json jObj, int parentIndex)
{
    bone newBone;
    std::string texturePath = jObj["texture"];
    newBone.name = jObj["name"];
    newBone.textureId = Engine->GetTextureManager().RegisterTexture(Engine->GetRenderer(), (identifier + "_" + newBone.name).c_str(), texturePath.c_str());
    newBone.length = jObj["length"];
    newBone.size = { jObj["size"][0], jObj["size"][1] };
    newBone.baseRotation = jObj["rotation"];
    newBone.basePosition = { jObj["position"][0], jObj["position"][1] };

    if (parentIndex != -1) // root
        newBone.basePosition += bones[parentIndex].basePosition;

    newBone.currentPosition = newBone.basePosition;
    newBone.currentRotation = newBone.baseRotation;

    int boneIndex = static_cast<int>(bones.size());
    bones.push_back(newBone);

    for (nlohmann::json boneJson : jObj["children"])
    {
        int childIndex = ParseBoneJson(boneJson, boneIndex);
        bones[boneIndex].children.push_back(childIndex);
    }

    return boneIndex;
}