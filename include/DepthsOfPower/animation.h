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
    f32 length;
    glm::vec2 size;
    std::string name;

    std::vector<int> children;
    int textureId;
};

struct diamond_transform;
class skeleton
{
public:
    void Initialize(const char* path);
    void Draw(diamond_transform baseTransform);

    std::vector<bone> bones;

private:
    int ParseBoneJson(nlohmann::json jObj, int parentIndex);
    void DrawBone(int boneIndex, glm::vec2 bonePos);

    std::string identifier;
};