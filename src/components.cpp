#include <DepthsOfPower/components.h>

void physics_component::Tick(entity& entity)
{
    if (entity.physicsComponent.has_value())
    {
        const b2Transform& transform = entity.physicsComponent.value().body->GetTransform();
        entity.physicsComponent.value().body->SetAwake(true);
        entity.position = { transform.p.x, transform.p.y };
    }
}