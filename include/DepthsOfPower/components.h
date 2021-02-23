#pragma once
#include <DepthsOfPower/util/basic.h>
#include <vec2.hpp>
#include <optional>

struct entity;
struct physics_component
{
    glm::vec2 extent;
};

struct entity
{
    glm::vec2 position;
    std::optional<physics_component> physicsComponent;
};
