#pragma once
#include <Diamond/diamond.h>

class engine
{
public:
    void Initialize();
    void BeginFrame();
    void EndFrame();
    void Cleanup();

    bool IsRunning();

private:
    diamond renderer;

    bool running = true;
};