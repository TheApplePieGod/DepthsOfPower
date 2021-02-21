#pragma once
#include <Diamond/diamond.h>
#include <DepthsOfPower/camera.h>
#include <DepthsOfPower/input.h>
#include <DepthsOfPower/tilemap.h>
#include <DepthsOfPower/util/basic.h>
#include <chrono>
#include <array>

class engine
{
public:
    void Initialize();
    void BeginFrame();
    void HandleInput();
    void RenderTestScene();
    void EndFrame();
    void Cleanup();

    bool IsRunning();
    inline diamond& GetRenderer() { return renderer; };
    inline input_manager& GetInputManager() { return inputManager; }
    inline camera& GetCamera() { return mainCamera; };

private:
    diamond renderer;
    camera mainCamera;
    input_manager inputManager;
    tilemap map = tilemap(1, 1, 1.f);

    bool running = true;

    // delta time smoothing
    double deltaTime = 0.f; // ms
    std::array<double, 11> deltaTimes;
    int frameCount = 0;

    std::chrono::steady_clock::time_point frameStart;

};