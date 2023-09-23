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


    /*!
        @brief Returns a BrickID from a string name, e.g. "dirt"
        @param str the string name of a brick, e.g. "dirt"
    */
    int IdFromName(std::string str);

	int GetBrick(int x, int z, int y);
    void SetBrick(int x, int z, int y, int id);
    int GetLightLevel(int x, int z, int y) { return 0; }

    brick_ao_t GetBrickAO(int xindex, int zindex, int y);

    void BuildChunk(int chunkX, int chunkZ);

	void Initialize();
	void Draw(Camera &cam);

    void LoadBrickMetaData();

    std::vector<std::string> GetTextureFilenames();

    std::vector<std::array<int,6>>GetLookupArr() const;
    void FillWater(int x, int z, int y);

    chunk_t *GetChunk(int x, int z);

    float GetBrickTransparency(int id) const;

    /*! 
        @brief Rebuilds all chunks within the view distance
        This is mostly for debugging
    */
    void RebuildAllVisible();

    bool IsBrickSurroundedByOpaque(int x, int z, int y);
    /*!
        @brief Pre-generates the world given a radius around a chunk  

        Pre-generates the world for a given radius around a position.

        @param fromX X-Starting origin in chunks
        @param fromZ Z-Starting origin in chunks
    */
    void GenerateChunksFromOrigin(int fromX, int fromZ, int radius);

public: // public vars
    int NUM_THREADS;
    static constexpr int MAX_LIGHT_LEVEL = 16;
    
    std::map<std::string, int>BrickNameMap;
private: // private methods
    void MarkChunkVisibilities(Camera &cam);
    void SpawnGeneratorThreads(Camera &cam);
    void SpawnBuilderThreads(Camera &cam);
    
private:
    std::thread threads[32];
    std::vector<std::string> TextureNamesFromFile(std::string filename);
    std::vector<std::string>BrickTextureFilenames;

private:
    std::vector<std::thread>BuilderThreads;
    Camera &camera;
    TextureArray myTexArray;
    std::vector<std::array<int,6>>BrickLookup;
    std::vector<float>BrickTransparencies;

    std::map <std::pair<int,int>, chunk_t>Chunks;
};

#endif
