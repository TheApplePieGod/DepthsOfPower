#include <DepthsOfPower/util/box2d_util.h>
#include <Diamond/structures.h>
#include <vec2.hpp>
#include <DepthsOfPower/engine.h>

extern engine* Engine;

void box2d_debug_draw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
    diamond_transform transform;
    transform.location = { (vertices[3].x + vertices[2].x) * 0.5f, (vertices[3].y + vertices[1].y) * 0.5f };
    transform.scale = { abs(vertices[2].x - vertices[3].x), abs(vertices[3].y - vertices[1].y) };

    glm::vec4 quadColor = { 1.f, 0.f, 0.f, 0.8f };

    Engine->GetRenderer().DrawQuad(-1, transform, quadColor);
}