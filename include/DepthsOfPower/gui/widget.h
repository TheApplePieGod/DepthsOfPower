#pragma once
#include <vec2.hpp>
#include <vec4.hpp>
#include <vector>
#include <DepthsOfPower/util/basic.h>

// a widget is the base gui element and can act as both a parent and a child
class widget
{
public:
    widget(glm::vec2 pos, glm::vec2 size);

    void SetPosition(glm::vec2 newPos);
    void SetSize(glm::vec2 newSize);
    inline void SetPadding(glm::vec2 newValue) { padding = newValue; };
    inline void SetColor(glm::vec4 newColor) { color = newColor; }; 
    inline void SetTextureId(int newId) { textureId = newId; }; 
    inline void SetTransparency(float newValue) { color.a = newValue; };

    virtual void Draw(bool isParent, glm::vec2 currentOffset = { 0.f, 0.f }, glm::vec2 parentBounds = { 0.f, 0.f }); // passed down by parent, new draw origin of this element

    u32 AddChild(widget* child);
    void RemoveChild(u32 index);
    inline void SetChild(u32 index, widget* child) { children[index] = child; };

    inline glm::vec2 GetPosition() { return position; };
    inline glm::vec2 GetSize() { return size; };
    inline glm::vec2 GetPadding() { return padding; };
    inline glm::vec4 GetColor() { return color; };
    inline int GetTextureId() { return textureId; };

protected:
    glm::vec2 position = { 0.f, 0.f }; // position of the top left corner in screen space ([0,0] is top left and [1,1] is bottom right) (position is treated as an offset when this element is a child)
    glm::vec2 size = { 0.2f, 0.2f, }; // size of the widget extending out of the origin (top left corner) (if size is zero and widget is child, it will fill to bounds)
    glm::vec2 padding = { 0.f, 0.f }; // space between the edges of parent element and child elements
    glm::vec4 color = { 0.f, 0.f, 0.f, 1.f };
    int textureId = -1;
    std::vector<widget*> children;
};