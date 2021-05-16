#include <DepthsOfPower/engine.h>
#include <algorithm>

extern engine* Engine;

int component_manager::CreateEntity(entity data)
{
    entityList.push_back(data);
    return static_cast<int>(entityList.size() - 1);
}

int component_manager::FindEntity(const char* identifier)
{
    std::string name = identifier;
    for (u64 i = 0; i < entityList.size(); i++)
    {
        if (entityList[i].identifier == name)
            return i;
    }
    return -1;
}

void component_manager::FollowPlayerSystem()
{
    for (u32 i = 0; i < entityList.size(); i++)
    {
        if (entityList[i].transformComponentId != -1)
        {
            transform_component& transformComp = transformComponents[entityList[i].transformComponentId];

            if (transformComp.followPlayer)
            {
                // assumes entity zero is the player
                transform_component& playerTransformComp = transformComponents[entityList[0].transformComponentId];
                transformComp.transform.location = playerTransformComp.transform.location;
            }
        }
    }
}

void component_manager::PhysicsSystem(double frameDelta, tilemap& map)
{
    for (u32 i = 0; i < entityList.size(); i++)
    {
        if (entityList[i].transformComponentId != -1 && entityList[i].physicsComponentId != -1)
        {
            transform_component& transformComp = transformComponents[entityList[i].transformComponentId];
            physics_component& physicsComp = physicsComponents[entityList[i].physicsComponentId];
            int numSteps = 10;

            f32 gravity = (frameDelta / 1000) * physicsComp.gravitySpeed / (f32)numSteps;
            glm::vec2 velocity = (f32)(frameDelta / 1000) * physicsComp.velocity / (f32)numSteps;

            for (int i = 0; i < numSteps; i++)
            {
                // x movement
                f32 finalMovement = velocity.x;
                glm::vec2 oldPosition = transformComp.transform.location;
                transformComp.transform.location.x += finalMovement;
                if (map.IsColliding(transformComp.transform.location, physicsComp.extent))
                    transformComp.transform.location = oldPosition;

                // y movement
                finalMovement = velocity.y;
                finalMovement -= gravity;
                oldPosition = transformComp.transform.location;
                transformComp.transform.location.y += finalMovement;
                if (map.IsColliding(transformComp.transform.location, physicsComp.extent))
                    transformComp.transform.location = oldPosition;
            }

            physicsComp.velocity = { 0.f, 0.f };
        }
    }
}

void component_manager::AnimationSystem(double frameDelta)
{
    for (u32 i = 0; i < entityList.size(); i++)
    {
        if (entityList[i].transformComponentId != -1 && entityList[i].animationComponentId != -1)
        {
            transform_component& transformComp = transformComponents[entityList[i].transformComponentId];
            animation_component& animComp = animationComponents[entityList[i].animationComponentId];

            animComp.skeleton.TickAnimation(static_cast<f32>(frameDelta));
            animComp.skeleton.Draw(transformComp.transform);
        }
    }
}

void component_manager::SpriteSystem()
{
    diamond& renderer = Engine->GetRenderer();
    for (u32 i = 0; i < entityList.size(); i++)
    {
        if (entityList[i].transformComponentId != -1 && entityList[i].spriteComponentId != -1)
        {
            transform_component& transformComp = transformComponents[entityList[i].transformComponentId];
            sprite_component& spriteComp = spriteComponents[entityList[i].spriteComponentId];

            renderer.DrawQuad(spriteComp.textureId, spriteComp.texCoords, transformComp.transform);
        }
    }
}