#include <DepthsOfPower/engine.h>
#include <iostream>
#include <thread>
#include <algorithm>
#include <DepthsOfPower/generation.h>
#include <DepthsOfPower/gui/widget.h>
#include <DepthsOfPower/gui/widget_text.h>

void engine::Initialize()
{
    renderer.Initialize(800, 600, "Depths of Power", "../../assets/default-texture.png");
    inputManager.Initialize();
    soundManager.Initialize();

    diamond_graphics_pipeline_create_info gpCreateInfo = {};
    gpCreateInfo.vertexShaderPath = "../../shaders/main.vert.spv";
    gpCreateInfo.fragmentShaderPath = "../../shaders/main.frag.spv";
    gpCreateInfo.maxVertexCount = 100000;
    gpCreateInfo.maxIndexCount = 100000;
    renderer.CreateGraphicsPipeline(gpCreateInfo);

    // initialize textures
    textureManager.RegisterTexture(renderer, "dirt", "../../assets/tiles/maps/dirt_map.png");
    textureManager.RegisterTexture(renderer, "stone", "../../assets/tiles/maps/stone_map.png");
    textureManager.RegisterTexture(renderer, "cold_stone", "../../assets/tiles/maps/cold_stone_map.png");
    textureManager.RegisterTexture(renderer, "hot_stone", "../../assets/tiles/maps/hot_stone_map.png");
    textureManager.RegisterTexture(renderer, "limestone", "../../assets/tiles/maps/limestone_map.png");
    textureManager.RegisterTexture(renderer, "granite", "../../assets/tiles/maps/granite_map.png");
    textureManager.RegisterTexture(renderer, "marble", "../../assets/tiles/maps/marble_map.png");
    textureManager.RegisterTexture(renderer, "basalt", "../../assets/tiles/maps/basalt_map.png");
    textureManager.RegisterTexture(renderer, "iron_ore", "../../assets/tiles/maps/iron_ore_map.png");
    textureManager.RegisterTexture(renderer, "copper_ore", "../../assets/tiles/maps/copper_ore_map.png");
    
    textureManager.RegisterTexture(renderer, "character", "../../assets/character.png");
    textureManager.RegisterTexture(renderer, "font_calibri", "../../assets/fonts/calibri.png");
    textureManager.RegisterTexture(renderer, "rounded_rectangle", "../../assets/rounded_rectangle.png");
    renderer.SyncTextureUpdates();

    soundManager.RegisterSound("block_break", "../../assets/block_break.wav");
    soundManager.RegisterSound("laser", "../../assets/laser.wav");
    soundManager.RegisterSound("music", "../../assets/music.wav");

    // initialize tilemap and position camera at the center of it
    int mapSizeX = 200;
    int mapSizeY = 500;
    f32 tileSizeMeters = 1.f;
    map = tilemap(mapSizeX, mapSizeY, tileSizeMeters);
    world_generator generator;
    generator.SetSeed(1337);
    generator.Generate(map, true);

    // init player entity
    transform_component playerTransform;
    physics_component physicsComp;
    animation_component animComp;
    entity player;
    componentManager.entityList.push_back(player); // entity 0 should be the main player

    playerTransform.transform.location = { MetersToPixels(tileSizeMeters) * mapSizeX * 0.5f, MetersToPixels(tileSizeMeters) * mapSizeY * 0.95f };
    playerTransform.transform.location.y = map.GetWorldLocationOfTile(map.RayTraceForTile(playerTransform.transform.location, { 0.f, -1.f }, 0)).y + MetersToPixels(3.f);
    componentManager.GetTransformComponent(0, true) = playerTransform;

    physicsComp.extent = { MetersToPixels(0.4f), MetersToPixels(0.9f) };
    componentManager.GetPhysicsComponent(0, true) = physicsComp;

    animComp.skeleton.Initialize("../../assets/character.skel");
    componentManager.GetAnimationComponent(0, true) = animComp;

    //animation anim = entityList[0].animationComponent.value().skeleton.LoadAnimation("../../assets/character_walk.anim");
    //entityList[0].animationComponent.value().skeleton.PlayAnimation(anim, true);

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
    renderer.BeginFrame();

    // call every frame just to cover any resize events
    renderer.UpdateCameraViewMode(diamond_camera_mode::OrthographicViewportIndependent, { 500.f, 500.f / renderer.GetAspectRatio() });

    renderer.SetGraphicsPipeline(0);

    // Draw the tilemap    
    map.Draw(mainCamera.GetPosition());
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

    // character movement
    f32 movementSpeed = MetersToPixels(30.f);
    physics_component& physicsComp = componentManager.GetPhysicsComponent(0);
    if (inputManager.IsKeyPressed("a"))
        physicsComp.velocity.x -= movementSpeed;
    if (inputManager.IsKeyPressed("d"))
        physicsComp.velocity.x += movementSpeed;
    if (inputManager.IsKeyPressed("w"))
        physicsComp.velocity.y += movementSpeed * 2.f;
    if (inputManager.IsKeyPressed("s"))
        physicsComp.velocity.y -= movementSpeed * 1.2f;

    if (inputManager.IsMouseDown(1)) // lmb
    {
        glm::vec2 mouseWorldPos = inputManager.GetMouseWorldPosition();
        glm::vec2 soundLocation = PixelsToMeters(mouseWorldPos) - PixelsToMeters(mainCamera.GetPosition());

        if (breakingSoundIndex == -1)
        {
            sound_settings settings;
            settings.looping = true;
            settings.gain = 0.5f;
            settings.rolloffFactor = 0.25f;
            settings.referenceDistance = 4.f;

            breakingSoundIndex = soundManager.PlaySoundAtLocation("laser", soundLocation, settings);
        }
        else
            soundManager.UpdateSoundLocation(breakingSoundIndex, soundLocation);

        u64 tileIndex = map.GetTileAtLocation(mouseWorldPos);
        if (tileIndex != -1 && map.GetTile(tileIndex).textureId != 0)
        {
            tile newTile;
            newTile.textureId = 0;
            map.UpdateTile(tileIndex, newTile);
        }
    }
    else if (breakingSoundIndex != -1)
    {
        soundManager.StopSound(breakingSoundIndex);
        breakingSoundIndex = -1;
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

void engine::TickComponents()
{
    soundManager.Tick();
    
    componentManager.PhysicsSystem(renderer.FrameDelta(), map);
    componentManager.AnimationSystem(renderer.FrameDelta());
    componentManager.SpriteSystem();
}

void engine::EndFrame()
{
    mainCamera.SetPosition(componentManager.GetTransformComponent(0).transform.location);
    renderer.SetCameraViewMatrix(mainCamera.GetViewMatrix());

    // Draw widgets
    widgetManager.DrawAllWidgets();

    inputManager.ClearJustPressedFlags();

    //std::this_thread::sleep_for(std::chrono::milliseconds(30));

    glm::vec4 clearColor = { 0.009, 0.009, 0.009, 1.f };
    renderer.EndFrame(clearColor);

    // update widget information
    std::string tempText = "Delta: " + std::to_string(renderer.FrameDelta());
    widgetManager.GetWidget<widget>("test_widget")->GetChild<widget_text>("text_delta")->SetText(tempText.c_str());

    glm::vec2 location = componentManager.GetTransformComponent(0).transform.location;
    tempText = "Pos: (" + std::to_string((int)location.x) + ", " + std::to_string((int)location.y) + ")";
    widgetManager.GetWidget<widget>("test_widget")->GetChild<widget_text>("text_pos")->SetText(tempText.c_str());

    //std::cout << "dt: " << deltaTime << std::endl;
}

void engine::Cleanup()
{
    renderer.Cleanup();
    soundManager.Cleanup();
}

bool engine::IsRunning()
{
    return running && renderer.IsRunning();
}