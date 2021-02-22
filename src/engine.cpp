#include <DepthsOfPower/engine.h>
#include <iostream>
#include <thread>
#include <algorithm>
#include <DepthsOfPower/generation.h>
#include <Box2D/b2_polygon_shape.h>
#include <Box2D/b2_fixture.h>
#include <Box2D/b2_circle_shape.h>

void engine::Initialize()
{
    renderer.Initialize(800, 600, "Depths of Power", "../shaders/main.vert.spv", "../shaders/main.frag.spv");
    inputManager.Initialize();

    // init box2d
    b2Vec2 gravity(0.f, -1.5f);
    physicsWorld = new b2World(gravity);
    physicsWorld->SetDebugDraw(&b2dDebugInstance);
    physicsWorld->SetAutoClearForces(false);
    b2dDebugInstance.SetFlags(b2Draw::e_shapeBit);

    // initialize tilemap and position camera at the center of it
    int mapSizeX = 200;
    int mapSizeY = 500;
    f32 tileSizeMeters = 1.f;
    map = tilemap(mapSizeX, mapSizeY, tileSizeMeters);
    world_generator generator;
    generator.Generate(map, true);

    // init player entity
    physics_component physicsComp;
    entity player;
    player.position = { MetersToPixels(tileSizeMeters) * mapSizeX * 0.5f, MetersToPixels(tileSizeMeters) * mapSizeY - MetersToPixels(100.f) };
    b2PolygonShape dynamicBox;
    f32 extentX = MetersToPixels(0.8f);
    f32 extentY = MetersToPixels(1.8f);
    dynamicBox.SetAsBox(extentX * 0.4f, extentY * 0.25f, b2Vec2(0, extentY*0.25f), 0);
    b2CircleShape circle;
    circle.m_p.Set(0, -(extentX * 0.22f));
    circle.m_radius = extentX * 0.5f;
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.fixedRotation = true;
    bodyDef.position.Set(player.position.x, player.position.y);
    physicsComp.body = physicsWorld->CreateBody(&bodyDef);
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &dynamicBox;
    fixtureDef.density = 1.f;
    fixtureDef.friction = 0.f;
    physicsComp.body->CreateFixture(&fixtureDef);
    fixtureDef.shape = &circle;
    fixtureDef.friction = 0.f;
    physicsComp.body->CreateFixture(&fixtureDef);
    physicsComp.body->SetBullet(true);
    physicsComp.body->SetLinearDamping(0.08f); 
    
    player.physicsComponent = physicsComp;
    entityList.push_back(player); // entity 0 should be the main player

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
    mainCamera.SetPosition(entityList[0].position);
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

    b2Body* body = entityList[0].physicsComponent.value().body;
    f32 movementSpeed = deltaTime * 10000.f;
    if (inputManager.IsKeyPressed("a"))
        //mainCamera.Move({ -movementSpeed, 0.f });
        body->ApplyForceToCenter(b2Vec2(-movementSpeed, 0), true);
    if (inputManager.IsKeyPressed("d"))
        //mainCamera.Move({ movementSpeed, 0.f });
        body->ApplyForceToCenter(b2Vec2(movementSpeed, 0), true);
    if (inputManager.IsKeyPressed("w"))
        //mainCamera.Move({ 0.f, movementSpeed * 1.2f });
        body->ApplyLinearImpulseToCenter(b2Vec2(0, movementSpeed * 1.f), true);
    if (inputManager.IsKeyPressed("s"))
        //mainCamera.Move({ 0.f, -movementSpeed * 1.2f });
        body->ApplyForceToCenter(b2Vec2(0, -movementSpeed * 1.f), true);

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
    //physicsWorld->DebugDraw();
}

void engine::TickPhysics()
{
    dtAccumulator += rawDeltaTime;

    int step = 0;
    while (dtAccumulator > physicsStep && step <= 5)
    {
        physicsWorld->Step(physicsStep, 6, 2);
        step++;
        dtAccumulator -= physicsStep;
    }

    // const int nSteps = static_cast<int> (
	// 	std::floor (dtAccumulator / physicsStep)
	// );

    // if (nSteps > 0)
	// {
	// 	dtAccumulator -= nSteps * physicsStep;
	// }

    // const int nStepsClamped = std::min(nSteps, maxSteps);
	// for (int i = 0; i < nStepsClamped; ++ i)
	// {
    //     physicsWorld->Step(physicsStep, 6, 2);
    // }

    //physicsWorld->Step(deltaTime, 6, 2);

    physicsWorld->ClearForces();
    map.UpdateColliders(mainCamera.GetPosition());
}

void engine::TickComponents()
{
    for (u32 i = 0; i < entityList.size(); i++)
    {
        physics_component::Tick(entityList[i]);
    }
}

void engine::EndFrame()
{
    inputManager.ClearJustPressedFlags();

    glm::vec4 clearColor = { 0.009, 0.009, 0.009, 1.f };
    renderer.EndFrame(clearColor);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // average delta time for smoother steps
    auto frameStop = std::chrono::high_resolution_clock::now();
    double dt = std::max((double)(std::chrono::duration_cast<std::chrono::milliseconds>(frameStop - frameStart)).count(), 1.0);
    deltaTimes[frameCount] = dt;
    rawDeltaTime = dt;
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