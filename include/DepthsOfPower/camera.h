#pragma once
#include <vec2.hpp>
#include <mat4x4.hpp>

class camera
{
public:
    inline glm::vec2 GetPosition() { return position; };
    void SetPosition(glm::vec2 newPos);
    void Move(glm::vec2 offset);
    inline glm::mat4 GetViewMatrix() { return viewMatrix; };

private:
    glm::vec2 position = { 0.f, 0.f };
    glm::mat4 viewMatrix;
    void UpdateViewMatrix();

};
