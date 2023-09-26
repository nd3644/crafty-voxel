#ifndef CHUNK_H
#define CHUNK_H

#include "mesh.h"
#include "sprite.h"
#include <cstdint>
#include <libnoise/noise.h>

class Map;
struct chunk_t {
    enum ChunkState {
        DEFAULT_STAGE = 0,
        GEN_STAGE,
        BUILD_STAGE,
        UPLOAD_STAGE,
        READY_STAGE,
        NUM_STAGES
    };

    constexpr static int DEFAULT_CONTINENTALNESS_SEED = 321;
    constexpr static int DEFAULT_EROSION_SEED = 432;
    constexpr static double DEFAULT_FREQUENCY = 0.0005*2;
    
    constexpr static int DEFAULT_CONTINENTALNESS_OCTAVES = 8;
    constexpr static int SEA_LEVEL = 45;

    static void ConfigureContinentalness(noise::module::Perlin &perlin);
    static void ConfigureErosion(noise::module::Perlin &perlin);

    ChunkState curStage;

    chunk_t();
    ~chunk_t();

    static constexpr int CHUNK_SIZE = 16;
    static constexpr int MAX_HEIGHT = 256;

    Mesh mesh;
    uint8_t iBricks[CHUNK_SIZE][MAX_HEIGHT][CHUNK_SIZE];
    uint8_t iLightLevels[CHUNK_SIZE][MAX_HEIGHT][CHUNK_SIZE];
    
//        uint8_t ambientVecs[CHUNK_SIZE][MAX_HEIGHT][CHUNK_SIZE][6][4];
    
    bool bVisible;

    bool bGen;
    // This is for preventing Generate recursively calling itself. This can probably be done better
    bool bIsCurrentlyGenerating;
    bool bIniialBuild, bInitialAOBuild;

    void Generate(int chunkx, int chunkz, Map &map);

    float heatShift;



    int iRebuildCounter; // Reflects the number of times this chunk has been rebuilt
    bool bRequiresRebuild;
};

#endif
