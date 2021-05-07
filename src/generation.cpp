#include <DepthsOfPower/generation.h>
#include <FastNoise/FastNoise.h>
#include <DepthsOfPower/util/basic.h>
#include <thread>
#include <DepthsOfPower/engine.h>

#include <stb/stb_image_write.h>

extern engine* Engine;

void Generate_Thread(tilemap& map, int startY, int lines, int seed)
{
    texture_manager& texManager = Engine->GetTextureManager();

    auto fnSimplex = FastNoise::New<FastNoise::Simplex>();
    auto fnFractal = FastNoise::New<FastNoise::FractalFBm>();
    auto fnCaveFractal = FastNoise::New<FastNoise::FractalRidged>();

    fnFractal->SetSource(fnSimplex);
    fnFractal->SetOctaveCount(3);

    std::vector<f32> basicNoiseOutput(map.GetWidth() * lines);
    std::vector<f32> temperatureNoiseOutput(map.GetWidth() * lines);
    std::vector<f32> biomeNoiseOutput(map.GetWidth() * lines);
    std::vector<f32> caveNoiseOutput(map.GetWidth() * lines);
    //std::vector<f32> highFrequencyNoise(map.GetWidth() * lines);

    fnFractal->GenUniformGrid2D(basicNoiseOutput.data(), 0, startY, map.GetWidth(), lines, 0.03f, seed);

    fnFractal->SetOctaveCount(2);
    fnFractal->GenUniformGrid2D(temperatureNoiseOutput.data(), 0, startY, map.GetWidth(), lines, 0.05f, seed + 1);

    fnFractal->GenUniformGrid2D(biomeNoiseOutput.data(), 0, startY, map.GetWidth(), lines, 0.02f, seed + 2);

    //fnFractal->GenUniformGrid2D(highFrequencyNoise.data(), 0, startY, map.GetWidth(), lines, 0.1f, seed + 3);

    fnCaveFractal->SetSource(fnSimplex);
    fnCaveFractal->SetOctaveCount(4);
    fnCaveFractal->GenUniformGrid2D(caveNoiseOutput.data(), 0, startY, map.GetWidth(), lines, 0.04f, seed + 4);

    u64 startPos = startY * map.GetWidth();
    u64 localIndex = 0;
    for (u64 i = startPos; i < startPos + lines * map.GetWidth(); i++)
    {
        tile newTile;

        // generation parameters
        f32 initialValuesRatio = 500.f; // inital map height that the gen params were tuned for
        f32 surfaceRigidness = 0.01f * (initialValuesRatio / map.GetHeight()); // affects the rigidness of the surface
        f32 groundStartThreshold = 0.8f; // affects when the sky ends and the ground starts generating
        f32 temperatureMapScale = 0.01f; // affects how much the temp map affects the final temperature (higher = colder)
        f32 temperatureGradientScale = 1.f; // affects how much the depth affects the final temperature
        f32 biomeScale = 1.f; // weight of the biome map
        f32 biomePerturbScale = 0.5f; // affects how jagged biomes are
        f32 cavePerturbScale = 0.25f; // affects how jagged cave generation is
        f32 caveThreshold = -0.1f; // affects how big caves are (-1 to 1)
        f32 caveMaxThreshold = 0.9f; // affects the maximum size of caves
        f32 caveDepthScale = 0.5f; // how much depth affects cave size

        f32 depthGradient = (i / (f32)map.GetWidth()) / map.GetHeight();
        f32 depthPerturb = basicNoiseOutput[localIndex % map.GetWidth()] * surfaceRigidness;
        depthGradient += depthPerturb;

        if (depthGradient < groundStartThreshold)
        {
            f32 cavePerturb = basicNoiseOutput[localIndex] * cavePerturbScale;
            f32 caveNoise = caveNoiseOutput[localIndex] + depthGradient * caveDepthScale + cavePerturb;
            if (std::min(caveNoise, caveMaxThreshold) > caveThreshold)
            {
                f32 temperatureGradient = (i / (f32)map.GetWidth()) / map.GetHeight() + (1 - groundStartThreshold);
                f32 temperatureMap = temperatureNoiseOutput[localIndex];
                f32 finalTemp = temperatureMap * temperatureMapScale + temperatureGradient * temperatureGradientScale;

                f32 biomePerturb = basicNoiseOutput[localIndex] * biomePerturbScale;
                f32 biomeMap = biomeNoiseOutput[localIndex] * biomeScale + biomePerturb;

                f32 specialBlockMap = temperatureNoiseOutput[localIndex];

                // biome selection based on temp
                if (finalTemp > 0.95f) // coldest, near surface
                {
                    newTile.textureId = texManager.GetTextureId("dirt"); // only dirt no biome
                }
                else if (finalTemp > 0.7f) // warm
                {
                    if (biomeMap > -0.5f) // more commonly cold stone
                    {
                        if (specialBlockMap > -0.62f && specialBlockMap < 0.62) // mostly not a special block
                            newTile.textureId = texManager.GetTextureId("cold_stone");
                        else
                        {
                            if (specialBlockMap >= 0.62) // sometimes copper ore
                                newTile.textureId = texManager.GetTextureId("copper_ore");
                            else  // sometimes iron ore
                                newTile.textureId = texManager.GetTextureId("iron_ore"); 
                        }
                    }
                    else // sometimes limestone biome
                    {
                        newTile.textureId = texManager.GetTextureId("limestone");
                    }
                }
                else if (finalTemp > 0.4) // hotter
                {
                    if (biomeMap > -0.5f && biomeMap < 0.5f) // more commonly regular stone
                    {
                        newTile.textureId = texManager.GetTextureId("stone");
                    }
                    else
                    {
                        if (biomeMap >= 0.5f) // sometimes limestone biome
                            newTile.textureId = texManager.GetTextureId("limestone");
                        else // sometimes marble biome
                            newTile.textureId = texManager.GetTextureId("marble");
                    }
                }
                else // hottest
                {
                    if (biomeMap > -0.5f && biomeMap < 0.5f) // more commonly hot stone
                    {
                        newTile.textureId = texManager.GetTextureId("hot_stone");
                    }
                    else
                    {
                        if (biomeMap >= 0.5f) // sometimes marble biome
                            newTile.textureId = texManager.GetTextureId("marble");
                        else // sometimes basalt biome
                            newTile.textureId = texManager.GetTextureId("basalt");
                    }
                }
            }
            else
                newTile.textureId = 0;
        }
        else
            newTile.textureId = 0;
        
        map.UpdateTile(i, newTile);
        localIndex++;
    }
}

void world_generator::Generate(tilemap& map, bool savePreview)
{
    int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0)
        numThreads = 1;

    int currentLine = 0;
    int linesPerThread = (int)(map.GetHeight() / numThreads);
    int Extra = map.GetHeight() - (linesPerThread * numThreads);

    std::vector<std::thread> threads(numThreads);
    for (int i = 0; i < numThreads; i++) 
    {
        threads[i] = std::thread(Generate_Thread, std::ref(map), currentLine, linesPerThread + (i == numThreads - 1 ? Extra : 0), seed);
        currentLine += linesPerThread;
    }

    for (auto& th : threads) 
    {
        th.join();
    }

    if (savePreview)
        map.DebugSaveMapToFile(false);
}

void world_generator::Test(tilemap& map)
{
    int xMax = (int)map.GetWidth();
    int yMax = (int)map.GetHeight();

    std::vector<f32> basicNoiseOutput(xMax * yMax);
    std::vector<f32> biomeNoiseOutput(xMax * yMax);
    std::vector<f32> caveNoiseOutput(xMax * yMax);

    auto fnSimplex = FastNoise::New<FastNoise::Simplex>();
    auto fnFractal = FastNoise::New<FastNoise::FractalFBm>();
    auto fnCaveFractal = FastNoise::New<FastNoise::FractalRidged>();

    fnFractal->SetSource(fnSimplex);
    fnFractal->SetOctaveCount(3);
    fnFractal->GenUniformGrid2D(basicNoiseOutput.data(), 0, 0, xMax, yMax, 0.03f, seed);
    fnFractal->GenUniformGrid2D(biomeNoiseOutput.data(), 0, 0, xMax, yMax, 0.02f, seed + 1);

    fnCaveFractal->SetSource(fnSimplex);
    fnCaveFractal->SetOctaveCount(2);
    fnCaveFractal->GenUniformGrid2D(caveNoiseOutput.data(), 0, 0, xMax, yMax, 0.04f, seed + 2);

    srand(seed);

    SetupMap(map, basicNoiseOutput);
    GenerateBiomes(map, biomeNoiseOutput);
    CreateCaves(map, basicNoiseOutput, caveNoiseOutput);

    map.DebugSaveMapToFile(false);
}

void world_generator::SetupMap(tilemap& map, const std::vector<f32>& basicNoise)
{
    texture_manager& texManager = Engine->GetTextureManager();

    int xMax = (int)map.GetWidth();
    int yMax = (int)map.GetHeight();

    f32 initialValuesRatio = 500.f; // inital map height that the gen params were tuned for
    f32 surfaceRigidness = 0.01f * (initialValuesRatio / map.GetHeight()); // affects the rigidness of the surface
    f32 groundStartThreshold = 0.8f; // affects when the sky ends and the ground starts generating

    f32 stoneStart = groundStartThreshold - (40.f / yMax);
    tile newTile;
    for (int y = 0; y < yMax; y++)
    {
        f32 depthGradient = (f32)y / yMax;

        for (int x = 0; x < xMax; x++)
        {
            f32 depthPerturb = basicNoise[x] * surfaceRigidness;
            if (depthGradient + depthPerturb > groundStartThreshold)
                continue;

            if (depthGradient + depthPerturb > stoneStart)
                newTile.textureId = texManager.GetTextureId("dirt");
            else
                newTile.textureId = texManager.GetTextureId("stone");

            map.UpdateTile(x + y * xMax, newTile);
        }
    }
}

void world_generator::CreateCaves(tilemap& map, const std::vector<f32>& basicNoise, const std::vector<f32>& caveNoise)
{
    int xMax = (int)map.GetWidth();
    int yMax = (int)map.GetHeight();

    f32 cavePerturbScale = 0.25f; // affects how jagged cave generation is
    f32 caveThreshold = 0.9f; // affects how big caves are (-1 to 1)
    f32 caveMaxThreshold = 0.5f; // affects the maximum size of caves regardless of depth
    f32 caveDepthScale = 1.f; // how much depth affects cave size

    tile newTile;
    for (int y = 0; y < yMax; y++)
    {
        f32 depthGradient = (f32)y / yMax;

        for (int x = 0; x < xMax; x++)
        {
            f32 cavePerturb = basicNoise[x + y * xMax] * cavePerturbScale;
            f32 caveNoiseFinal = caveNoise[x + y * xMax] + std::min(depthGradient * caveDepthScale, caveMaxThreshold) + cavePerturb;
            if (caveNoiseFinal > caveThreshold)
            {
                newTile.textureId = 0;
                map.UpdateTile(xMax * yMax - (x + y * xMax) - 1, newTile);
            }
        }
    }
}

void world_generator::GenerateBiomes(tilemap& map, const std::vector<f32>& biomeNoise)
{
    texture_manager& texManager = Engine->GetTextureManager();

    int xMax = (int)map.GetWidth();
    int yMax = (int)map.GetHeight();

    std::vector<biome> biomeList;
    biome biomeInfo;

    biomeInfo.textureId = texManager.GetTextureId("marble");
    biomeInfo.frequency = 0.0001f;
    biomeInfo.maxRadius = 10;
    biomeInfo.edgeDistortionFactor = 10.f;
    biomeInfo.depthRange = glm::vec2(0.75f, 0.5f);
    biomeList.push_back(biomeInfo);

    biomeInfo.textureId = texManager.GetTextureId("limestone");
    biomeInfo.frequency = 0.0001f;
    biomeInfo.maxRadius = 10;
    biomeInfo.edgeDistortionFactor = 10.f;
    biomeInfo.depthRange = glm::vec2(0.5f, 0.3f);
    biomeList.push_back(biomeInfo);

    biomeInfo.textureId = texManager.GetTextureId("basalt");
    biomeInfo.frequency = 0.0005f;
    biomeInfo.maxRadius = 10;
    biomeInfo.edgeDistortionFactor = 10.f;
    biomeInfo.depthRange = glm::vec2(0.35f, 0.0f);
    biomeList.push_back(biomeInfo);

    tile newTile;
    for (const biome& biomeObj : biomeList)
    {
        int biomeCount = (int)(xMax * yMax * biomeObj.frequency);
        for (int i = 0; i < biomeCount; i++)
        {
            // select a random point on the map
            int xPos = rand() % xMax;
            int yPos = (int)(biomeObj.depthRange.x * yMax) - (rand() % (int)(yMax * (biomeObj.depthRange.y - biomeObj.depthRange.x)));

            f32 baselineNoise = biomeNoise[xPos + yPos * xMax];

            // set the pixels based on the radius of the biome
            for (int y = yPos - biomeObj.maxRadius * 2; y <= yPos + biomeObj.maxRadius * 2; y++)
            {
                if (y < 0 || y >= yMax)
                    continue;

                for (int x = xPos - biomeObj.maxRadius * 2; x <= xPos + biomeObj.maxRadius * 2; x++)
                {
                    if (x < 0 || x >= xMax)
                        continue;

                    f32 noiseVal = biomeNoise[x + y * xMax];
                    int modifiedRadius = (int)(biomeObj.maxRadius + noiseVal * biomeObj.edgeDistortionFactor);

                    if (sqrt(pow(x - xPos, 2) + pow(y - yPos, 2)) > modifiedRadius)
                       continue;

                    u64 tileIndex = x + y * xMax;
                    newTile.textureId = biomeObj.textureId;
                    map.UpdateTile(tileIndex, newTile);
                }
            }
        }
    }
}