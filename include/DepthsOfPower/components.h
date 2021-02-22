#pragma once
#include <DepthsOfPower/util/basic.h>
#include <Box2D/b2_body.h>
#include <vec2.hpp>
#include <optional>

struct entity;
struct physics_component
{
    b2Body* body;

    static void Tick(entity& entity);
};

struct entity
{
    glm::vec2 position;
    std::optional<physics_component> physicsComponent;
};
