#pragma once
#include <Diamond/diamond.h>
#include <DepthsOfPower/camera.h>
#include <DepthsOfPower/input.h>
#include <DepthsOfPower/tilemap.h>
#include <DepthsOfPower/util/basic.h>
#include <DepthsOfPower/util/box2d_util.h>
#include <DepthsOfPower/components.h>
#include <chrono>
#include <array>
#include <vector>
#include <Box2D/b2_world.h>

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
    inline b2World* GetPhysicsWorld() { return physicsWorld; };

private:
    diamond renderer;
    camera mainCamera;
    input_manager inputManager;
    tilemap map = tilemap(1, 1, 1.f);
    std::vector<entity> entityList;

    bool running = true;

    // delta time smoothing
    double rawDeltaTime = 0.f;
    double deltaTime = 0.f; // ms
    std::array<double, 11> deltaTimes;
    int frameCount = 0;
    std::chrono::steady_clock::time_point frameStart;

    // box2D
    b2World* physicsWorld = nullptr;
    box2d_debug_draw b2dDebugInstance;
    double dtAccumulator = 0.0;
    double physicsStep = 1.0 / 30.0;

};