#pragma once
#include <DepthsOfPower/util/basic.h>
#include <DepthsOfPower/animation.h>
#include <glm/vec2.hpp>
#include <optional>

struct entity;
struct physics_component
{
    glm::vec2 extent;
};

struct animation_component
{
    skeleton skeleton;
};

struct entity
{
    glm::vec2 position;
    std::optional<physics_component> physicsComponent;
    std::optional<animation_component> animationComponent;
};
