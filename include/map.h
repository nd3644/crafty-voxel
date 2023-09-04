#ifndef MAP_H
#define MAP_H

#include "ebmp.h"
#include <array>
#include "mesh.h"
#include "texture_array.h"
#include "brick_ao.h"
#include "chunk.h"
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
public: // public methods
	Map(Camera &c);
	~Map();

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

    std::vector<vec3_t>lights;

    std::vector<std::tuple<float, float, RGB>>toDraw;

    std::vector<std::string> GetTextureFilenames();
    bool IsDay() const ;
    void SetDay(bool b);

    std::vector<std::array<int,6>>GetLookupArr() const;
    void FillWater(int x, int z, int y);

    chunk_t *GetChunk(int x, int z);

    float GetBrickTransparency(int id);

    // Makes all chunks within the view distance rebuild their meshes.
    // This is more for debugging than anything.
    void RebuildAllVisible();

    bool IsBrickSurroundedByOpaque(int x, int z, int y);

    /*!

        @brief Pre-generates the world given a radius around a chunk  
        Pre-generates the world for a given radius around a position.   
        @param fromX X-Starting origin in chunk indices
        @param fromZ Z-Starting origin in chunk indices
    */
    void GenerateChunksFromOrigin(int fromX, int fromZ, int radius);

public: // public vars
    int NUM_THREADS;
    static constexpr int MAX_LIGHT_LEVEL = 16;

    struct light_t {
        int x, y, z;
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

    std::map<std::string, int>BrickNameMap;
private: // private methods

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

    std::map <std::pair<int,int>, chunk_t>Chunks;

};

#endif
