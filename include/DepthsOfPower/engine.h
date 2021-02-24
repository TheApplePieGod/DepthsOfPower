#pragma once
#include <Diamond/diamond.h>
#include <DepthsOfPower/camera.h>
#include <DepthsOfPower/input.h>
#include <DepthsOfPower/tilemap.h>
#include <DepthsOfPower/util/basic.h>
#include <DepthsOfPower/components.h>
#include <DepthsOfPower/gui/widget_manager.h>
#include <chrono>
#include <array>
#include <vector>

class engine
{
public:
    void Initialize();
    void BeginFrame();
    void HandleInput();
    void TickPhysics();
    void TickComponents();
    void RenderTestScene();
    void EndFrame();
    void Cleanup();

    bool IsRunning();
    inline diamond& GetRenderer() { return renderer; };
    inline input_manager& GetInputManager() { return inputManager; }
    inline camera& GetCamera() { return mainCamera; };
    inline entity& GetEntity(int id) { return entityList[id]; };

private:
    diamond renderer;
    camera mainCamera;
    input_manager inputManager;
    widget_manager widgetManager;
    tilemap map = tilemap(1, 1, 1.f);
    std::vector<entity> entityList;

    bool running = true;

    // delta time smoothing
    double deltaTime = 0.f; // ms
    std::array<double, 11> deltaTimes;
    int frameCount = 0;
    std::chrono::steady_clock::time_point frameStart;
};