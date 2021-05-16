#include <DepthsOfPower/engine.h>
#include <fstream>
#include <iostream>
#include <glm/gtx/compatibility.hpp>

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

    initialized = true;
}

void skeleton::Draw(diamond_transform baseTransform)
{
    Assert(initialized);

    glm::vec2 locationMeters = { PixelsToMeters(baseTransform.location.x), PixelsToMeters(baseTransform.location.y) };
    DrawBone(0, locationMeters, baseTransform.zPosition); // root
}

void skeleton::DrawBone(int boneIndex, glm::vec2 bonePos, f32 zPosition)
{
    diamond& renderer = Engine->GetRenderer();
    bone& bone = bones[boneIndex];

    glm::vec2 direction = { cos(DegreesToRadians(bone.currentRotation)), sin(DegreesToRadians(bone.currentRotation)) };
    glm::vec2 boneBaseDirection = { cos(DegreesToRadians(bone.baseRotation)), sin(DegreesToRadians(bone.baseRotation)) };
    glm::vec2 boneEnd = bonePos + bone.currentPosition + direction * bone.length;
    glm::vec2 midpoint = (boneEnd + bonePos + bone.currentPosition) * 0.5f; // midpoint

    diamond_transform boneTransform;
    boneTransform.location = { MetersToPixels(midpoint.x), MetersToPixels(midpoint.y) };
    boneTransform.scale = { bone.scale.x * (MetersToPixels(bone.length) * abs(boneBaseDirection.x) + MetersToPixels(1.f) * abs(boneBaseDirection.y)), bone.scale.y * (MetersToPixels(bone.length) * abs(boneBaseDirection.y) + MetersToPixels(1.f) * abs(boneBaseDirection.x)) };
    boneTransform.zPosition = zPosition;

    // modify the image rotation based on the base rotation
    boneTransform.rotation = -bone.currentRotation + bone.baseRotation - bone.textureRotation;
    renderer.DrawQuad(bone.textureId, boneTransform);

    boneTransform.rotation = -bone.currentRotation;

    for (int childIndex : bone.children)
    {
        DrawBone(childIndex, boneEnd, zPosition);
    }
}

int skeleton::GetBoneIdFromName(std::string name)
{
    for (u32 i = 0; i < bones.size(); i++)
    {
        if (bones[i].name == name)
            return static_cast<int>(i);
    }
    return -1;
}

inline bool SortKeyframes(const keyframe& a, const keyframe& b)
{
    return a.time < b.time;
}

animation skeleton::LoadAnimation(const char* path)
{
    std::ifstream ifs(path);
    nlohmann::json jObj;
    ifs >> jObj;

    Assert(initialized && jObj["skeleton"] == identifier);

    animation newAnimation;
    newAnimation.skeleton = identifier;
    newAnimation.name = jObj["name"];

    for (nlohmann::json keyframeObj : jObj["keyframes"])
    {
        keyframe newKeyframe;
        newKeyframe.time = keyframeObj["time"];

        for (nlohmann::json boneObj : keyframeObj["bones"])
        {
            bone_delta newDelta;
            newDelta.position = { boneObj["position"][0], boneObj["position"][0] };
            newDelta.rotation = boneObj["rotation"];
            newDelta.id = GetBoneIdFromName(boneObj["name"]);

            newKeyframe.bones.push_back(newDelta);
        }

        newAnimation.keyframes.push_back(newKeyframe);
    }
    std::sort(newAnimation.keyframes.begin(), newAnimation.keyframes.end(), SortKeyframes);

    return newAnimation;
}

void skeleton::PlayAnimation(animation animation, bool loop)
{
    Assert(identifier == animation.skeleton);

    currentAnimation = animation;
    currentAnimationTime = 0.f;
    looping = loop;
    TickAnimation(0);
}

void skeleton::TickAnimation(f32 deltaTime)
{
    if (currentAnimation.keyframes.size() == 0)
        return;

    currentAnimationTime += deltaTime * 0.001f;

    // find starting keyframe
    int startKeyframeIndex = -1;
    for (u32 i = 0; i < currentAnimation.keyframes.size() - 1; i++)
    {
        if (currentAnimationTime >= currentAnimation.keyframes[i].time && currentAnimationTime < currentAnimation.keyframes[i + 1].time)
        {
            startKeyframeIndex = i;
            break;
        }
    }
    if (startKeyframeIndex == -1) // if time exceeds all keyframes, reset if looping, otherwise stop the animation
    {
        if (!looping)
        {
            currentAnimation.keyframes.clear();
            return;
        }

        currentAnimationTime = 0;
        startKeyframeIndex = 0;
    }

    f32 lerpFactor = (currentAnimationTime - currentAnimation.keyframes[startKeyframeIndex].time) / (currentAnimation.keyframes[startKeyframeIndex + 1].time - currentAnimation.keyframes[startKeyframeIndex].time);

    for (const bone_delta& delta1 : currentAnimation.keyframes[startKeyframeIndex].bones)
    {
        // only do the lerp if the bone also is also changing positions in the next keyframe
        for (const bone_delta& delta2 : currentAnimation.keyframes[startKeyframeIndex + 1].bones)
        {
            if (delta1.id == delta2.id)
            {
                bones[delta1.id].currentPosition = glm::lerp(delta1.position, delta2.position, lerpFactor);
                bones[delta1.id].currentRotation = glm::lerp(delta1.rotation, delta2.rotation, lerpFactor);
                break;
            }   
        }
    }
}

int skeleton::ParseBoneJson(nlohmann::json jObj, int parentIndex)
{
    bone newBone;
    std::string texturePath = jObj["texture"];
    std::string textureIdentifier = identifier + "_" + newBone.name;

    newBone.name = jObj["name"];
    newBone.textureId = Engine->GetTextureManager().GetTextureId(textureIdentifier.c_str());
    if (newBone.textureId == 0)
        newBone.textureId = Engine->GetTextureManager().RegisterTexture(Engine->GetRenderer(), textureIdentifier.c_str(), texturePath.c_str());
    newBone.textureRotation = jObj["textureRotation"];
    newBone.length = jObj["length"];
    newBone.scale = { jObj["scale"][0], jObj["scale"][1] };

    newBone.basePosition = { jObj["position"][0], jObj["position"][1] };
    newBone.baseRotation = jObj["rotation"];
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