#ifndef MAP_H
#define MAP_H

#include "ebmp.h"
#include <array>
#include "mesh.h"
#include "texture_array.h"
#include "brick_ao.h"
#include <iostream>
#include <map>
#include <unordered_map>
#include <cmath>
#include <limits>
#include <thread>
#include <stack>
#include <queue>
#include <glm/glm.hpp>

#include <SDL2/SDL.h>

class Camera;
class Map
{
public:
	Map(Camera &c);
	~Map();

	 int height;
	 int width;
	 int depth;

    int NUM_THREADS;

    static constexpr int CHUNK_SIZE = 16;
    static constexpr int MAX_HEIGHT = 256;
    static constexpr int MAX_LIGHT_LEVEL = 16;

    const static int half_limit = std::numeric_limits<int>::max() / 2;

    struct light_t {
        int x, y, z;
    };

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

    enum Priority {
        IMMEDIATE = 0,
        ONE,
        TWO,
        THREE,
        NUM_PRIORITIES            
    };
    struct build_schedule_info_t {
        int x, z;
        Priority priorityLevel;
    };

    int IdFromName(std::string str);

    /* This function makes a terrible amount of effort to prevent negative indices
      because they were causing a lot of trouble. */
	int GetBrick(int x, int z, int y);
    void SetBrick(int x, int z, int y, int id);
    int GetLightLevel(int x, int z, int y) { return 0; }

    brick_ao_t GetBrickAO(int xindex, int zindex, int y);

    void BuildChunk(int chunkX, int chunkZ);
    void BuildChunkAO(int chunkX, int chunkZ);

    void RebuildLights();

    void RebuildAll();

	void FromBMP(std::string sfile);
	void Draw(Camera &cam);

    void RunBuilder();

    void LoadBrickMetaData();

    // Take a chunk position and buiilds the 9 chunks within and around that point
    void ScheduleAdjacentChunkBuilds(int startx, int startz, Priority level);

    std::vector<vec3_t>lights;

    std::vector<std::tuple<float, float, RGB>>toDraw;

    std::vector<std::string> GetTextureFilenames();
    bool IsDay() const ;
    void SetDay(bool b);

    std::vector<std::array<int,6>>GetLookupArr() const;
    void FillWater(int x, int z, int y);

    void ScheduleMeshBuild(build_schedule_info_t info);

    chunk_t *GetChunk(int x, int z);

    float GetBrickTransparency(int id);

    // Makes all chunks within the view distance rebuild their meshes.
    // This is more for debugging than anything.
    void RebuildAllVisible();


    bool IsBrickSurroundedByOpaque(int x, int z, int y);

    // Pre-generates the world given a radius around a chunk
    void GenerateChunksFromOrigin(int fromX, int fromZ, int radius);
private:
    std::thread threads[16];
    std::vector<std::string> TextureNamesFromFile(std::string filename);
    std::vector<std::string>BrickTextureFilenames;
    std::vector<build_schedule_info_t>ScheduledBuilds;

private:
    bool bIsDay;
    std::vector<std::thread>BuilderThreads;
    Camera &camera;
    TextureArray myTexArray;
    std::vector<std::array<int,6>>BrickLookup;
    std::vector<float>BrickTransparencies;

    std::map<std::string, int>BrickNameMap;
    std::map <std::pair<int,int>, chunk_t>Chunks;

};

#endif
