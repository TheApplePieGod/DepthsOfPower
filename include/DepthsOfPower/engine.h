#pragma once
#include <Diamond/diamond.h>
#include <DepthsOfPower/camera.h>
#include <DepthsOfPower/input.h>
#include <DepthsOfPower/sound.h>
#include <DepthsOfPower/tilemap.h>
#include <DepthsOfPower/util/basic.h>
#include <DepthsOfPower/util/texture_manager.h>
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
    void TickComponents();
    void EndFrame();
    void Cleanup();

    bool IsRunning();
    inline diamond& GetRenderer() { return renderer; };
    inline input_manager& GetInputManager() { return inputManager; }
    inline sound_manager& GetSoundManager() { return soundManager; }
    inline texture_manager& GetTextureManager() { return textureManager; }
    inline camera& GetCamera() { return mainCamera; };
    inline entity& GetEntity(int id) { return componentManager.entityList[id]; };

private:
    diamond renderer;
    camera mainCamera;
    input_manager inputManager;
    sound_manager soundManager;
    widget_manager widgetManager;
    texture_manager textureManager;
    component_manager componentManager;
    tilemap map = tilemap(1, 1, 1.f);

    bool running = true;
    int breakingSoundIndex = -1;
};