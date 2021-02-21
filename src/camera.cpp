#include <DepthsOfPower/camera.h>
#include <gtc/matrix_transform.hpp>

void camera::SetPosition(glm::vec2 newPos)
{
    position = newPos;
    UpdateViewMatrix();
}

void camera::Move(glm::vec2 offset)
{
    position += offset;
    UpdateViewMatrix();
}

void camera::UpdateViewMatrix()
{
    viewMatrix = glm::lookAt(glm::vec3(position.x, position.y, 5.f), glm::vec3(position.x, position.y, 0.f), glm::vec3(0.f, 1.f, 0.f));
}