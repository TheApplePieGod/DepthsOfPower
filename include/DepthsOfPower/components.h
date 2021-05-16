#pragma once
#include <DepthsOfPower/util/basic.h>
#include <DepthsOfPower/animation.h>
#include <Diamond/diamond.h>
#include <glm/vec2.hpp>
#include <optional>

struct transform_component
{
    diamond_transform transform;
    bool followPlayer = false;
};

struct physics_component
{
    glm::vec2 extent;
    glm::vec2 velocity = { 0.f, 0.f }; // in meters/second
    f32 gravitySpeed = MetersToPixels(25.f); // in meters/second
};

struct animation_component
{
    skeleton skeleton;
};

struct sprite_component
{
    int textureId = 0;
    glm::vec4 texCoords = { 0.f, 0.f, 1.f, 1.f };
};

struct entity
{
    std::string identifier;
    int transformComponentId = -1;
    int physicsComponentId = -1;
    int animationComponentId = -1;
    int spriteComponentId = -1;
};

enum component_type
{
    Transform,
    Physics,
    Animation,
    Sprite
};

#define ENTITY_COMPONENT(struct, arrayName, getFunctionName, createFunctionName, idName) \
    std::vector<struct> arrayName; \
    inline struct& getFunctionName(int entityId, bool create = false) \
    { \
        Assert(entityId != -1) \
        if (entityList[entityId].idName == -1) \
        { \
            if (create) \
                entityList[entityId].idName = createFunctionName(); \
            else \
                Assert(1==2); \
        } \
        return arrayName[entityList[entityId].idName]; \
    } \
    inline int createFunctionName() \
    { \
        arrayName.push_back(struct()); \
        return static_cast<int>(arrayName.size() - 1); \
    }

class tilemap;
class component_manager
{
public:
    std::vector<entity> entityList;

    ENTITY_COMPONENT(transform_component, transformComponents, GetTransformComponent, CreateTransformComponent, transformComponentId)
    ENTITY_COMPONENT(physics_component, physicsComponents, GetPhysicsComponent, CreatePhysicsComponent, physicsComponentId)
    ENTITY_COMPONENT(animation_component, animationComponents, GetAnimationComponent, CreateAnimationComponent, animationComponentId)
    ENTITY_COMPONENT(sprite_component, spriteComponents, GetSpriteComponent, CreateSpriteComponent, spriteComponentId)

    void FollowPlayerSystem();
    void PhysicsSystem(double frameDelta, tilemap& map);
    void AnimationSystem(double frameDelta);
    void SpriteSystem();

    int CreateEntity(entity data);
    int FindEntity(const char* identifier);

private:

};