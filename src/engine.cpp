#include <DepthsOfPower/engine.h>
#include <iostream>
#include <thread>
#include <algorithm>
#include <DepthsOfPower/generation.h>

void engine::Initialize()
{
    renderer.Initialize(800, 600, "Depths of Power", "../shaders/main.vert.spv", "../shaders/main.frag.spv");
    inputManager.Initialize();

    // init box2d
    b2Vec2 gravity(0.f, -MetersToPixels(1.f));
    physicsWorld = new b2World(gravity);
    physicsWorld->SetDebugDraw(&b2dDebugInstance);
    b2dDebugInstance.SetFlags(b2Draw::e_shapeBit);

    // initialize tilemap and position camera at the center of it
    int mapSizeX = 200;
    int mapSizeY = 500;
    f32 tileSizeMeters = 1.f;
    map = tilemap(mapSizeX, mapSizeY, tileSizeMeters);
    world_generator generator;
    generator.Generate(map, true);
    mainCamera.SetPosition({ MetersToPixels(tileSizeMeters) * mapSizeX * 0.5f, MetersToPixels(tileSizeMeters) * mapSizeY - MetersToPixels(100.f) });

    renderer.RegisterTexture("../images/dirt_map.png");
    renderer.RegisterTexture("../images/stone_map.png");
    renderer.RegisterTexture("../images/cold_stone_map.png");
    renderer.RegisterTexture("../images/hot_stone_map.png");
    renderer.RegisterTexture("../images/limestone_map.png");
    renderer.RegisterTexture("../images/granite_map.png");
    renderer.SyncTextureUpdates();
}

void engine::BeginFrame()
{
    frameStart = std::chrono::high_resolution_clock::now();
    renderer.BeginFrame(diamond_camera_mode::OrthographicViewportIndependent, mainCamera.GetViewMatrix());
}

void engine::HandleInput()
{
    inputManager.Tick();
    
    if (inputManager.WasKeyJustPressed("ESCAPE"))
        running = false;
    if (inputManager.WasKeyJustPressed("F1"))
        renderer.SetFullscreen(true);
    if (inputManager.WasKeyJustPressed("F2"))
        renderer.SetFullscreen(false);

    f32 movementSpeed = (deltaTime / 1000.f) * MetersToPixels(20.f);
    if (inputManager.IsKeyPressed("a"))
        mainCamera.Move({ -movementSpeed, 0.f });
    if (inputManager.IsKeyPressed("d"))
        mainCamera.Move({ movementSpeed, 0.f });
    if (inputManager.IsKeyPressed("w"))
        mainCamera.Move({ 0.f, movementSpeed * 1.2f });
    if (inputManager.IsKeyPressed("s"))
        mainCamera.Move({ 0.f, -movementSpeed * 1.2f });

    if (inputManager.IsMouseDown(1)) // lmb
    {
        glm::vec2 mouseWorldPos = inputManager.GetMouseWorldPosition();
        u64 tileIndex = map.GetTileAtLocation(mouseWorldPos);
        if (tileIndex != -1)
        {
            tile newTile;
            newTile.textureId = 0;
            map.UpdateTile(tileIndex, newTile);
        }
    }

    if (inputManager.IsMouseDown(2)) // rmb
    {
        glm::vec2 mouseWorldPos = inputManager.GetMouseWorldPosition();
        u64 tileIndex = map.GetTileAtLocation(mouseWorldPos);
        if (tileIndex != -1)
        {
            tile newTile;
            newTile.textureId = 2;
            map.UpdateTile(tileIndex, newTile);
        }
    }
}

void engine::RenderTestScene()
{
    diamond_transform transform{};
    transform.scale = { MetersToPixels(0.25f), MetersToPixels(0.25f) };
    transform.location = mainCamera.GetPosition();
    
    map.Draw(mainCamera.GetPosition());
    renderer.DrawQuad(0, transform, { 0.f, 0.f, 0.f, 1.f });
    physicsWorld->DebugDraw();
}

void engine::TickPhysics()
{
    dtAccumulator += deltaTime;

    while (dtAccumulator >= physicsStep)
    {
        physicsWorld->Step(physicsStep, 6, 2);
        dtAccumulator -= physicsStep;
    }

    map.UpdateColliders(mainCamera.GetPosition());
}

void engine::EndFrame()
{
    inputManager.ClearJustPressedFlags();

    glm::vec4 clearColor = { 0.009, 0.009, 0.009, 1.f };
    renderer.EndFrame(clearColor);

    //std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // average delta time for smoother steps
    auto frameStop = std::chrono::high_resolution_clock::now();
    deltaTimes[frameCount] = std::max((double)(std::chrono::duration_cast<std::chrono::milliseconds>(frameStop - frameStart)).count(), 1.0);
    frameCount++;

    if (frameCount == deltaTimes.size())
    {
        std::sort(deltaTimes.begin(), deltaTimes.end());
        double avg = 0.f;
        for (int i = 2; i < deltaTimes.size() - 3; i++)
        {
            avg += deltaTimes[i];
        }
        avg /= deltaTimes.size() - 4;

        deltaTime = avg;

        frameCount = 0;
    }

    //f32 fps = 1.f / (deltaTime / 1000.f);
    //std::cout << "dt: " << deltaTime << std::endl;
}

void engine::Cleanup()
{
    renderer.Cleanup();
}

bool engine::IsRunning()
{
    return running && renderer.IsRunning();
}