#include <DepthsOfPower/generation.h>
#include <FastNoise/FastNoise.h>
#include <DepthsOfPower/util/basic.h>
#include <thread>
#include <DepthsOfPower/engine.h>

#include <stb/stb_image_write.h>

extern engine* Engine;

void SetupMap_Thread(tilemap& map, std::vector<heat_level>& heatLevels, generation_parameters genParams, int startY, int lines, int seed)
{
    texture_manager& texManager = Engine->GetTextureManager();

    int xMax = (int)map.GetWidth();

    // noise setup
    std::vector<f32> basicNoise(xMax * lines);
    auto fnSimplex = FastNoise::New<FastNoise::Simplex>();
    auto fnFractal = FastNoise::New<FastNoise::FractalFBm>();
    fnFractal->SetSource(fnSimplex);
    fnFractal->SetOctaveCount(3);
    fnFractal->GenUniformGrid2D(basicNoise.data(), 0, startY, xMax, lines, 0.03f, seed);

    f32 initialValuesRatio = 500.f; // inital map height that the gen params were tuned for
    f32 surfaceRigidness = 0.01f * (initialValuesRatio / map.GetHeight()); // affects the rigidness of the surface

    f32 stoneStart = genParams.groundStartThreshold - (40.f / map.GetHeight());
    f32 heatInterval = stoneStart / heatLevels.size();
    tile newTile;
    for (int y = startY; y < startY + lines; y++)
    {
        f32 depthGradient = (f32)y / map.GetHeight();

        for (int x = 0; x < xMax; x++)
        {
            f32 depthPerturb = basicNoise[x] * surfaceRigidness;
            if (depthGradient + depthPerturb > genParams.groundStartThreshold)
                continue;

            u64 heatIndex = std::min((u64)(std::max(0.f, stoneStart - depthGradient + depthPerturb) / heatInterval), heatLevels.size() - 1);

            if (depthGradient + depthPerturb > stoneStart)
                newTile.textureId = texManager.GetTextureId("dirt");
            else
                newTile.textureId = heatLevels[heatIndex].baseTextureId;

            map.UpdateTile(x + y * xMax, newTile);
        }
    }
}

void GenerateCaves_Thread(tilemap& map, generation_parameters genParams, int startY, int lines, int seed)
{
    int xMax = (int)map.GetWidth();

    // noise setup
    std::vector<f32> caveNoise(xMax * lines);
    std::vector<f32> stringNoise(xMax * lines);
    auto fnSimplex = FastNoise::New<FastNoise::Simplex>();
    auto fnCaveFractal = FastNoise::New<FastNoise::FractalFBm>();
    auto fnStringFractal = FastNoise::New<FastNoise::FractalRidged>();
    fnCaveFractal->SetSource(fnSimplex);
    fnCaveFractal->SetOctaveCount(3);
    fnCaveFractal->GenUniformGrid2D(caveNoise.data(), 0, startY, xMax, lines, 0.02f, seed + 1);
    fnStringFractal->SetSource(fnSimplex);
    fnStringFractal->SetOctaveCount(2);
    fnStringFractal->GenUniformGrid2D(stringNoise.data(), 0, startY, xMax, lines, 0.03f, seed + 2);
    
    f32 depthOffset = genParams.groundStartThreshold - (80.f / map.GetHeight()); // 40 tiles below stone start
    tile newTile;
    for (int y = startY; y < startY + lines; y++)
    {
        f32 depthGradient = (f32)y / map.GetHeight();
        f32 depthFactor = std::min((f32)pow(depthGradient + depthOffset, 200) * genParams.caveDepthScale, genParams.caveMaxThreshold);

        for (int x = 0; x < xMax; x++)
        {
            f32 stringFinal = stringNoise[x + (y - startY) * xMax];
            f32 caveFinal = caveNoise[x + (y - startY) * xMax] + stringFinal + depthFactor;
            if (caveFinal > genParams.caveThreshold)
            {
                newTile.textureId = 0;
                map.UpdateTile(map.GetSize() - (x + y * xMax) - 1, newTile);
            }
        }
    }
}

void world_generator::Generate(tilemap& map, bool savePreview)
{
    InitializeGenerationData();

    int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0)
        numThreads = 1;

    int linesPerThread = (int)(map.GetHeight() / numThreads);
    int Extra = map.GetHeight() - (linesPerThread * numThreads);

    std::vector<std::thread> threads(numThreads);

    // Setup map
    int currentLine = 0;
    for (int i = 0; i < numThreads; i++) 
    {
        threads[i] = std::thread(SetupMap_Thread, std::ref(map), std::ref(heatLevels), genParams, currentLine, linesPerThread + (i == numThreads - 1 ? Extra : 0), seed);
        currentLine += linesPerThread;
    }
    for (auto& th : threads) 
    {
        th.join();
    }

    GenerateOresAndBiomes(map);

    // Generate caves
    currentLine = 0;
    for (int i = 0; i < numThreads; i++) 
    {
        threads[i] = std::thread(GenerateCaves_Thread, std::ref(map), genParams, currentLine, linesPerThread + (i == numThreads - 1 ? Extra : 0), seed);
        currentLine += linesPerThread;
    }
    for (auto& th : threads) 
    {
        th.join();
    }

    if (savePreview)
        map.DebugSaveMapToFile(false);
}

void world_generator::InitializeGenerationData()
{
    texture_manager& texManager = Engine->GetTextureManager();

    // Heat levels
    heatLevels.clear();

    // Level 0
    {
        // Biomes
        biome limestoneBiome;
        limestoneBiome.textureId = texManager.GetTextureId("limestone");
        limestoneBiome.frequency = 0.001f;
        limestoneBiome.maxRadius = 5;
        limestoneBiome.edgeDistortionFactor = 10.f;

        // Ores
        biome copperOre;
        copperOre.textureId = texManager.GetTextureId("copper_ore");
        copperOre.frequency = 0.003f;
        copperOre.maxRadius = 2;
        copperOre.edgeDistortionFactor = 2.f;

        heat_level heatLevel0;
        heatLevel0.baseTextureId = texManager.GetTextureId("cold_stone");
        heatLevel0.biomes.push_back(limestoneBiome);
        heatLevel0.biomes.push_back(copperOre);
        heatLevels.push_back(heatLevel0);
    }

    // Level 1
    {
        // Biomes
        biome marbleBiome;
        marbleBiome.textureId = texManager.GetTextureId("marble");
        marbleBiome.frequency = 0.001f;
        marbleBiome.maxRadius = 5;
        marbleBiome.edgeDistortionFactor = 10.f;

        // Ores
        biome copperOre;
        copperOre.textureId = texManager.GetTextureId("copper_ore");
        copperOre.frequency = 0.001f;
        copperOre.maxRadius = 2;
        copperOre.edgeDistortionFactor = 2.f;

        biome ironOre;
        ironOre.textureId = texManager.GetTextureId("iron_ore");
        ironOre.frequency = 0.001f;
        ironOre.maxRadius = 2;
        ironOre.edgeDistortionFactor = 2.f;

        heat_level heatLevel1;
        heatLevel1.baseTextureId = texManager.GetTextureId("stone");
        heatLevel1.biomes.push_back(marbleBiome);
        heatLevel1.biomes.push_back(copperOre);
        heatLevel1.biomes.push_back(ironOre);
        heatLevels.push_back(heatLevel1);
    }

    // Level 2
    {
        // Biomes
        biome basaltBiome;
        basaltBiome.textureId = texManager.GetTextureId("basalt");
        basaltBiome.frequency = 0.001f;
        basaltBiome.maxRadius = 5;
        basaltBiome.edgeDistortionFactor = 10.f;

        // Ores
        biome ironOre;
        ironOre.textureId = texManager.GetTextureId("iron_ore");
        ironOre.frequency = 0.003f;
        ironOre.maxRadius = 2;
        ironOre.edgeDistortionFactor = 2.f;

        biome graniteOre;
        graniteOre.textureId = texManager.GetTextureId("granite");
        graniteOre.frequency = 0.001f;
        graniteOre.maxRadius = 2;
        graniteOre.edgeDistortionFactor = 2.f;

        heat_level heatLevel2;
        heatLevel2.baseTextureId = texManager.GetTextureId("hot_stone");
        heatLevel2.biomes.push_back(basaltBiome);
        heatLevel2.biomes.push_back(ironOre);
        heatLevel2.biomes.push_back(graniteOre);
        heatLevels.push_back(heatLevel2);
    }
}

void world_generator::GenerateOresAndBiomes(tilemap& map)
{
    texture_manager& texManager = Engine->GetTextureManager();

    int xMax = (int)map.GetWidth();
    int yMax = (int)map.GetHeight();

    // noise setup
    std::vector<f32> biomeNoise(xMax * yMax);
    auto fnSimplex = FastNoise::New<FastNoise::Simplex>();
    auto fnFractal = FastNoise::New<FastNoise::FractalFBm>();
    fnFractal->SetSource(fnSimplex);
    fnFractal->SetOctaveCount(2);
    fnFractal->GenUniformGrid2D(biomeNoise.data(), 0, 0, xMax, yMax, 0.02f, seed + 3);

    f32 stoneStart = genParams.groundStartThreshold - (40.f / map.GetHeight());
    f32 heatInterval = stoneStart / heatLevels.size();
    tile newTile;
    for (u64 i = 0; i < heatLevels.size(); i++)
    {
        int heatMax = (int)((stoneStart - heatInterval * i) * yMax);
        int heatMin = heatMax - (int)(heatInterval * yMax);

        for (const biome& biomeObj : heatLevels[i].biomes)
        {
            int biomeCount = (int)(xMax * (heatMax - heatMin) * biomeObj.frequency);
            for (int j = 0; j < biomeCount; j++)
            {
                // select a random point within this heat level
                int xPos = rand() % xMax;
                int yPos = heatMax - (rand() % (heatMax - heatMin));

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
}