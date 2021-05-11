#include <iostream>
#include <DepthsOfPower/engine.h>

engine* Engine;

int main(int argc, char** argv)
{
    Engine = new engine();
    Engine->Initialize();
    
    while (Engine->IsRunning())
    {
        Engine->HandleInput();
        Engine->TickPhysics();

        Engine->BeginFrame();

        Engine->TickComponents();

        Engine->EndFrame();
    }

    Engine->Cleanup();

    return 0;
}