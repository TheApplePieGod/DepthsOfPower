#include <DepthsOfPower/engine.h>

void engine::Initialize()
{
    renderer.Initialize(800, 600, "Depths of Power", "../shaders/main.vert.spv", "../shaders/main.frag.spv");
}

void engine::BeginFrame()
{
    renderer.BeginFrame(diamond_camera_mode::Orthographic, { 0.f, 0.f });
}

void engine::EndFrame()
{
    renderer.EndFrame();
}

void engine::Cleanup()
{
    renderer.Cleanup();
}

bool engine::IsRunning()
{
    return running && renderer.IsRunning();
}