#ifndef MAP_H
#define MAP_H

#include "ebmp.h"
#include <array>
#include "mesh.h"
#include "texture_array.h"
#include <iostream>
#include <map>
#include <unordered_map>
#include <cmath>
#include <limits>
#include <thread>

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

    static constexpr int CHUNK_SIZE = 64;
    static constexpr int MAX_HEIGHT = 64;

    const static int half_limit = std::numeric_limits<int>::max() / 2;

    struct chunk_t {
        chunk_t() {
            bGen = false;
            bIniialBuild = false;
            for(int i = 0;i < CHUNK_SIZE;i++) {
                for(int j = 0;j < CHUNK_SIZE;j++) {
                    for(int y = 0;y < MAX_HEIGHT;y++) {
                        iBricks[i][y][j] = 0;
                    }
                }
            }
        }

        ~chunk_t() {
        }

        Mesh mesh;
        int iBricks[CHUNK_SIZE][MAX_HEIGHT][CHUNK_SIZE];
        bool bGen;
        bool bIniialBuild;

        void Generate(int chunkx, int chunkz, Map &map);
    };

	inline void SetBrick(int x, int z, int y, int id) {
        using namespace std;
        if(x < -half_limit || z < -half_limit || x > half_limit || z > half_limit || y < 0 || y >= MAX_HEIGHT) {
            return;
        }

        if(x < 0)
            x += half_limit;

        if(z < 0)
            z += half_limit;

        int xchunk = x / CHUNK_SIZE;
        int zchunk = z / CHUNK_SIZE;

        int xindex = x % CHUNK_SIZE;
        int zindex = z % CHUNK_SIZE;

        auto chunk_index = std::make_pair(xchunk,zchunk);

        Chunks[std::make_pair(xchunk,zchunk)].iBricks[xindex][y][zindex] = id;
	}



    /* This function makes a terrible amount of effort to prevent negative indices
      because they were causing a lot of trouble. */
	inline int GetBrick(int x, int z, int y) {
        if(x < -half_limit || z < -half_limit || x > half_limit || z > half_limit || y < 0 || y >= 60) {
            return 0;
        }

        int origxchunk = x / CHUNK_SIZE;
        int origzchunk = z / CHUNK_SIZE;

        if(x < 0)
            x += half_limit;

        if(z < 0)
            z += half_limit;

        int xchunk = x / CHUNK_SIZE;
        int zchunk = z / CHUNK_SIZE;

        int xindex = x % CHUNK_SIZE;
        int zindex = z % CHUNK_SIZE;

        auto chunk_index = std::make_pair(xchunk,zchunk);

        // It's important to make sure the chunk was generated at this point because
        // adjacent chunks may be trying to access data here.
        if(!Chunks[chunk_index].bGen)
            Chunks[chunk_index].Generate(origxchunk,origzchunk,*this);

		return Chunks[chunk_index].iBricks[xindex][y][zindex];
	}

    void BuildChunk(int x, int z);
    void RebuildLights();

    void RebuildAll();

	void FromBMP(std::string sfile);
	void Draw();

    void RunBuilder();

    void LoadBrickMetaData();

    std::vector<vec3_t>lights;

    std::vector<std::tuple<float, float, RGB>>toDraw;

    int int_pair(int x, int z) {
        std::string str = std::to_string(x) + std::to_string(z);
        return std::stoi(str);
    }
private:
    std::vector<std::string> TextureNamesFromFile(std::string filename);

private:
    int viewDist;
    std::vector<std::thread>BuilderThreads;
    Camera &camera;
    float fAmbient;
    TextureArray myTexArray;
    std::vector<std::array<int,6>>BrickLookup;
    std::vector<float>BrickTransparencies;

    std::map <std::pair<int,int>, chunk_t>Chunks;

};

#endif
