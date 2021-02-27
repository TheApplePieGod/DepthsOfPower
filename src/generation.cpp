#include <DepthsOfPower/generation.h>
#include <FastNoise/FastNoise.h>
#include <DepthsOfPower/util/basic.h>
#include <thread>

void Generate_Thread(tilemap& map, int startY, int lines, int seed)
{
    auto fnSimplex = FastNoise::New<FastNoise::Simplex>();
    auto fnFractal = FastNoise::New<FastNoise::FractalFBm>();
    auto fnCaveFractal = FastNoise::New<FastNoise::FractalRidged>();

    fnFractal->SetSource(fnSimplex);
    fnFractal->SetOctaveCount(3);

    std::vector<f32> basicNoiseOutput(map.GetWidth() * lines);
    std::vector<f32> temperatureNoiseOutput(map.GetWidth() * lines);
    std::vector<f32> caveNoiseOutput(map.GetWidth() * lines);

    fnFractal->GenUniformGrid2D(basicNoiseOutput.data(), 0, startY, map.GetWidth(), lines, 0.03f, seed);

    fnFractal->SetOctaveCount(2);
    fnFractal->GenUniformGrid2D(temperatureNoiseOutput.data(), 0, startY, map.GetWidth(), lines, 0.05f, seed + 1);

    fnCaveFractal->SetSource(fnSimplex);
    fnCaveFractal->SetOctaveCount(4);
    fnCaveFractal->GenUniformGrid2D(caveNoiseOutput.data(), 0, startY, map.GetWidth(), lines, 0.04f, seed + 2);

    u64 startPos = startY * map.GetWidth();
    u64 localIndex = 0;
    for (u64 i = startPos; i < startPos + lines * map.GetWidth(); i++)
    {
        tile newTile;

        // generation parameters
        f32 initialValuesRatio = 500.f; // inital map height that the gen params were tuned for
        f32 surfaceRigidness = 0.01f * (initialValuesRatio / map.GetHeight()); // affects the rigidness of the surface
        f32 groundStartThreshold = 0.8f; // affects when the sky ends and the ground starts generating
        f32 startingTemperatureOffset = 0.2f; // affects the starting temperature at the highest point of the world (higher = colder)
        f32 temperatureMapScale = 0.01f; // affects how much the temp map affects the final temperature
        f32 temperatureGradientScale = 1.f; // affects how much the depth affects the final temperature
        f32 biomeScale = 1.f; // weight of the biome map
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
                f32 temperatureGradient = (i / (f32)map.GetWidth()) / map.GetHeight() + startingTemperatureOffset;
                f32 temperatureMap = temperatureNoiseOutput[localIndex];
                f32 finalTemp = temperatureMap * temperatureMapScale + temperatureGradient * temperatureGradientScale;

                f32 biomeMap = basicNoiseOutput[localIndex] * biomeScale;

                // biome selection based on temp
                if (finalTemp > 0.9f)
                {
                    newTile.textureId = 1; // only dirt no biome
                }
                else if (finalTemp > 0.5f)
                {
                    if (biomeMap > -0.2f)
                        newTile.textureId = 2; // more commonly regular stone
                    else
                        newTile.textureId = 5; // sometimes limestone biome
                }
                else if (finalTemp > 0.2)
                {
                    if (biomeMap > -0.2f)
                        newTile.textureId = 4; // more commonly hot stone
                    else
                        newTile.textureId = 6; // sometimes granite biome
                }
                else
                {
                    newTile.textureId = 4; // otherwise just hot stone
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
        map.DebugSaveMapToFile();
}
