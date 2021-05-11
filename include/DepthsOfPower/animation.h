#pragma once
#include <DepthsOfPower/util/basic.h>
#include <glm/vec2.hpp>
#include <vector>
#include <string>
#include <json/json.hpp>

struct bone
{
    glm::vec2 basePosition;
    f32 baseRotation;
    glm::vec2 currentPosition;
    f32 currentRotation;
    f32 textureRotation;
    f32 length;
    glm::vec2 scale;
    std::string name;

    std::vector<int> children;
    int textureId;
};

struct bone_delta
{
    int id;
    glm::vec2 position;
    f32 rotation;
};

struct keyframe
{
    f32 time;
    std::vector<bone_delta> bones;
};

struct animation
{
    std::string name;
    std::string skeleton;
    std::vector<keyframe> keyframes;
};

struct diamond_transform;
class skeleton
{
public:
    void Initialize(const char* path);
    void Draw(diamond_transform baseTransform);
    animation LoadAnimation(const char* path);
    void PlayAnimation(animation animation, bool loop);
    void TickAnimation(f32 deltaTime);

    int GetBoneIdFromName(std::string name);

    std::vector<bone> bones;

private:
    int ParseBoneJson(nlohmann::json jObj, int parentIndex);
    void DrawBone(int boneIndex, glm::vec2 bonePos);

    animation currentAnimation;
    f32 currentAnimationTime;
    bool looping;

    bool initialized = false;
    std::string identifier;
};