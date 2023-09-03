#ifndef CHUNK_H
#define CHUNK_H

#include "mesh.h"
#include <cstdint>

class Map;
struct chunk_t {
    enum ChunkState {
        DEFAULT_STAGE = 0,
        GEN_STAGE,
        AO_STAGE,
        BUILD_STAGE,
        UPLOAD_STAGE,
        READY_STAGE,
        NUM_STAGES
    };

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
};

#endif
