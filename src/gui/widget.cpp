#include <DepthsOfPower/gui/widget.h>
#include <DepthsOfPower/engine.h>

extern engine* Engine;

widget::widget(glm::vec2 pos, glm::vec2 _size)
{
    position = pos;
    size = _size;
}

void widget::SetPosition(glm::vec2 newPos)
{
    position = newPos;
}

void widget::SetSize(glm::vec2 newSize)
{
    size = newSize;
}

glm::vec2 widget::Draw(bool isParent, glm::vec2 currentOffset, glm::vec2 parentBounds)
{
    diamond_transform transform;
    glm::vec2 finalSize;

    if (isParent)
        finalSize = size;
    else
    {
        if (size.x != 0.f)
        {
            if (position.x + size.x > parentBounds.x)
                finalSize.x = parentBounds.x - position.x;
            else
                finalSize.x = size.x;
        }
        else
            finalSize.x = parentBounds.x;

        if (size.y != 0.f)
        {
            if (position.y + size.y > parentBounds.y)
                finalSize.y = parentBounds.y - position.y;
            else
                finalSize.y = size.y;
        }
        else
            finalSize.y = parentBounds.y;
    }

    transform.scale = finalSize;
    transform.location = { position.x + finalSize.x * 0.5f, position.y + finalSize.y * 0.5f };
    transform.location += currentOffset;

    glm::vec2 cameraPos = Engine->GetCamera().GetPosition();
    transform.location.y *= -1.f;
    transform.location = { transform.location.x * MetersToPixels(ScreenMetersX) + cameraPos.x, transform.location.y * MetersToPixels(ScreenMetersX) / Engine->GetRenderer().GetAspectRatio() + cameraPos.y }; // transform location
    transform.location += glm::vec2(-MetersToPixels(ScreenMetersX) * 0.5f, MetersToPixels(ScreenMetersX) * 0.5f / Engine->GetRenderer().GetAspectRatio()); // offset location
    transform.scale = { transform.scale.x * MetersToPixels(ScreenMetersX), transform.scale.y * MetersToPixels(ScreenMetersX) / Engine->GetRenderer().GetAspectRatio() };

    Engine->GetRenderer().DrawQuad(textureId, transform, color);
    glm::vec2 drawOffset = { 0.f, 0.f };
    for (const auto& p : children)
    {
        drawOffset.y += children[p.first]->Draw(false, position + padding + drawOffset, size - (padding + drawOffset) * 2.f).y;
    }

    return finalSize;
}

void widget::AddChild(const char* name, widget* child)
{
    children[name] = child;
}

void widget::RemoveChild(const char* name, bool deletePointer)
{
    if (deletePointer)
        delete children[name];
    children[name] = nullptr;
}