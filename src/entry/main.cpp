#include <iostream>
#include <DepthsOfPower/engine.h>

engine* Engine;

int main(int argc, char** argv)
{
    Engine = new engine();
    Engine->Initialize();
    
    while (Engine->IsRunning())
    {
        Engine->BeginFrame();
        Engine->HandleInput();

        Engine->TickPhysics();
        Engine->TickComponents();
        Engine->RenderTestScene();

        Engine->EndFrame();
    }

    Engine->Cleanup();

    return 0;
}