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

    const static int half_limit = std::numeric_limits<int>::max() / 2;

    struct chunk_t {
        chunk_t() {
            bGen = false;
            for(int i = 0;i < CHUNK_SIZE;i++) {
                for(int j = 0;j < CHUNK_SIZE;j++) {
                    for(int y = 0;y < 32;y++) {
                        iBricks[i][y][j] = 0;
                    }
                }
            }
        }
        Mesh mesh;
        int iBricks[CHUNK_SIZE][32][CHUNK_SIZE];
        bool bGen;

        void Generate(int chunkx, int chunkz, Map &map);
    };

	inline void SetBrick(int x, int z, int y, int id) {
        using namespace std;
        if(x < -half_limit || z < -half_limit || x > half_limit || z > half_limit) {
            return;
        }

        if(x < 0)
            x += half_limit;

        if(z < 0)
            z += half_limit;

/*        std::cout << "chunkx: " << floor(-64/CHUNK_SIZE) << std::endl;
        std::cout << "chunkz: " << floor(-10/CHUNK_SIZE) << std::endl;
        exit(0);*/
        int xchunk = x/CHUNK_SIZE;
        int zchunk = z/CHUNK_SIZE;
        Chunks[std::make_pair(xchunk,zchunk)].iBricks[abs(x%CHUNK_SIZE)][y][abs(z%CHUNK_SIZE)] = id;
	}


	inline int GetBrick(int x, int z, int y) {
        if(x < -half_limit || z < -half_limit || x > half_limit || z > half_limit) {
            return 0;
        }

        if(x < 0)
            x += half_limit;

        if(z < 0)
            z += half_limit;

        int xchunk = x/CHUNK_SIZE;
        int zchunk = z/CHUNK_SIZE;

        int xindex = abs(x%CHUNK_SIZE);
        int zindex = abs(z%CHUNK_SIZE);

		return Chunks[std::make_pair(xchunk,zchunk)].iBricks[xindex][y][zindex];
	}


    void GenerateDefaultChunk(int x, int y);
    
    void BuildChunk(int x, int z);
    void RebuildLights();

    void RebuildAll();

	void FromBMP(std::string sfile);
	void Draw();

    void LoadBrickMetaData();

    std::vector<vec3_t>lights;

private:
    void ProcessMap_Simple();
    
private:
    Camera &camera;
    float fAmbient;
//	int *iBricks;
    TextureArray myTexArray;
    std::vector<std::array<int,6>>BrickLookup;

    std::map <std::pair<int,int>, chunk_t>Chunks;

};

#endif
