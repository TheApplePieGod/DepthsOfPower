#include <DepthsOfPower/engine.h>
#include <iostream>
#include <thread>
#include <algorithm>
#include <DepthsOfPower/generation.h>
#include <DepthsOfPower/gui/widget.h>
#include <DepthsOfPower/gui/widget_text.h>

void engine::Initialize()
{
    renderer.Initialize(800, 600, "Depths of Power", "../images/default-texture.png");
    inputManager.Initialize();

    diamond_graphics_pipeline_create_info gpCreateInfo = {};
    gpCreateInfo.vertexShaderPath = "../shaders/main.vert.spv";
    gpCreateInfo.fragmentShaderPath = "../shaders/main.frag.spv";
    gpCreateInfo.maxVertexCount = 100000;
    gpCreateInfo.maxIndexCount = 100000;
    renderer.CreateGraphicsPipeline(gpCreateInfo);

    // initialize textures
    textureManager.RegisterTexture(renderer, "dirt", "../images/tiles/maps/dirt_map.png");
    textureManager.RegisterTexture(renderer, "stone", "../images/tiles/maps/stone_map.png");
    textureManager.RegisterTexture(renderer, "cold_stone", "../images/tiles/maps/cold_stone_map.png");
    textureManager.RegisterTexture(renderer, "hot_stone", "../images/tiles/maps/hot_stone_map.png");
    textureManager.RegisterTexture(renderer, "limestone", "../images/tiles/maps/limestone_map.png");
    textureManager.RegisterTexture(renderer, "granite", "../images/tiles/maps/granite_map.png");
    textureManager.RegisterTexture(renderer, "marble", "../images/tiles/maps/marble_map.png");
    textureManager.RegisterTexture(renderer, "basalt", "../images/tiles/maps/basalt_map.png");
    textureManager.RegisterTexture(renderer, "iron_ore", "../images/tiles/maps/iron_ore_map.png");
    textureManager.RegisterTexture(renderer, "copper_ore", "../images/tiles/maps/copper_ore_map.png");
    
    textureManager.RegisterTexture(renderer, "font_calibri", "../images/fonts/calibri.png");
    textureManager.RegisterTexture(renderer, "rounded_rectangle", "../images/rounded_rectangle.png");
    renderer.SyncTextureUpdates();

    // initialize tilemap and position camera at the center of it
    int mapSizeX = 200;
    int mapSizeY = 500;
    f32 tileSizeMeters = 1.f;
    map = tilemap(mapSizeX, mapSizeY, tileSizeMeters);
    world_generator generator;
    generator.SetSeed(1337);
    //generator.Generate(map, true);
    generator.Test(map);

    // init player entity
    physics_component physicsComp;
    entity player;
    player.position = { MetersToPixels(tileSizeMeters) * mapSizeX * 0.5f, MetersToPixels(tileSizeMeters) * mapSizeY * 0.95f };
    player.position.y = map.GetWorldLocationOfTile(map.RayTraceForTile(player.position, { 0.f, -1.f }, 0)).y + MetersToPixels(3.f);
    physicsComp.extent = { MetersToPixels(0.4f), MetersToPixels(0.9f) };
    player.physicsComponent = physicsComp;
    entityList.push_back(player); // entity 0 should be the main player

    // create widgets
    widget* testWidget = new widget({ 0.0f, 0.f }, { 0.25f, 0.5f });
    widget* secondaryWidget = new widget({ 0.f, 0.f }, { 0.0f, 0.0f });

    widget_text* textDelta = new widget_text({ 0.f, 0.f }, { 0.015f, 0.03f }, "");
    textDelta->SetCharacterSpacing(-0.01f);
    textDelta->SetWordSpacing(-0.003f);
    textDelta->SetTextureId(textureManager.GetTextureId("font_calibri"));

    widget_text* textPosition = new widget_text({ 0.f, 0.f }, { 0.015f, 0.03f }, "");
    textPosition->SetCharacterSpacing(-0.01f);
    textPosition->SetWordSpacing(-0.003f);
    textPosition->SetTextureId(textureManager.GetTextureId("font_calibri"));

    testWidget->AddChild("text_delta", textDelta);
    testWidget->AddChild("text_pos", textPosition);
    testWidget->SetTextureId(textureManager.GetTextureId("rounded_rectangle"));

    testWidget->SetColor({ 1.f, 0.f, 0.f, 1.f });
    testWidget->SetPadding({ 0.01f, 0.01f * renderer.GetAspectRatio() });
    widgetManager.AddWidget("test_widget", testWidget);
}

void engine::BeginFrame()
{
    frameStart = std::chrono::high_resolution_clock::now();
    mainCamera.SetPosition(entityList[0].position);

    glm::vec2 camDimensions = { 500.f, 500.f / renderer.GetAspectRatio() };
    renderer.BeginFrame(diamond_camera_mode::OrthographicViewportIndependent, camDimensions, mainCamera.GetViewMatrix());

    renderer.SetGraphicsPipeline(0);
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
    transform.scale = entityList[0].physicsComponent.value().extent * 2.f;
    transform.location = entityList[0].position;
    
    map.Draw(mainCamera.GetPosition());
    renderer.DrawQuad(0, transform, { 0.f, 0.f, 0.f, 1.f });
    widgetManager.DrawAllWidgets();
}

void engine::TickPhysics()
{
    // once we get more collision, move all of this into a collision manager
    int numSteps = 10; // todo: scale num steps by dt
    f32 movementSpeed = (deltaTime / 1000) * MetersToPixels(30.f) / numSteps;
    f32 gravity = (deltaTime / 1000) * MetersToPixels(25.f) / numSteps;

    for (int i = 0; i < numSteps; i++)
    {
        // x movement
        f32 finalMovement = 0.f;
        if (inputManager.IsKeyPressed("a"))
            finalMovement -= movementSpeed;
        if (inputManager.IsKeyPressed("d"))
            finalMovement += movementSpeed;
        glm::vec2 oldPosition = entityList[0].position;
        entityList[0].position.x += finalMovement;
        if (map.IsColliding(entityList[0].position, entityList[0].physicsComponent.value().extent))
            entityList[0].position = oldPosition;

        // y movement
        finalMovement = 0.f;
        if (inputManager.IsKeyPressed("w"))
            finalMovement += movementSpeed * 2.f;
        if (inputManager.IsKeyPressed("s"))
            finalMovement -= movementSpeed * 1.2f;
        finalMovement -= gravity;
        oldPosition = entityList[0].position;
        entityList[0].position.y += finalMovement;
        if (map.IsColliding(entityList[0].position, entityList[0].physicsComponent.value().extent))
            entityList[0].position = oldPosition;
    }
}

void engine::TickComponents()
{
    for (u32 i = 0; i < entityList.size(); i++)
    {
        
    }
}

void engine::EndFrame()
{
    inputManager.ClearJustPressedFlags();

    glm::vec4 clearColor = { 0.009, 0.009, 0.009, 1.f };
    renderer.EndFrame(clearColor);

    //std::this_thread::sleep_for(std::chrono::milliseconds(30));

    // average delta time for smoother steps
    auto frameStop = std::chrono::high_resolution_clock::now();
    double dt = std::max((double)(std::chrono::duration_cast<std::chrono::milliseconds>(frameStop - frameStart)).count(), 1.0);
    deltaTimes[frameCount] = dt;
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

    // update widget information
    std::string tempText = "Delta: " + std::to_string(deltaTime);
    widgetManager.GetWidget<widget>("test_widget")->GetChild<widget_text>("text_delta")->SetText(tempText.c_str());

    tempText = "Pos: (" + std::to_string((int)entityList[0].position.x) + ", " + std::to_string((int)entityList[0].position.y) + ")";
    widgetManager.GetWidget<widget>("test_widget")->GetChild<widget_text>("text_pos")->SetText(tempText.c_str());

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