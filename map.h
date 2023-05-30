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

    struct chunk_t {
        chunk_t() {
            bGen = false;
        }
        Mesh mesh;
        int iBricks[CHUNK_SIZE][32][CHUNK_SIZE];
        bool bGen;

        void Generate(int chunkx, int chunkz, Map &map);
    };

	inline void SetBrick(int x, int z, int y, int id) {
        Chunks[from_pair(x/CHUNK_SIZE,z/CHUNK_SIZE)].iBricks[abs(x%CHUNK_SIZE)][y][abs(z%CHUNK_SIZE)] = id;
	}


	inline int GetBrick(int x, int z, int y) {
		return Chunks[from_pair(x/CHUNK_SIZE,z/CHUNK_SIZE)].iBricks[abs(x%CHUNK_SIZE)][y][abs(z%CHUNK_SIZE)];
	}

    inline void SetLightLvl(int x, int z, int y, int id) {
        if(x < 0 || z < 0 || y < 0 || x >= width || z >= depth || y >= height) {
            return;
        } 
        if(id < 0)
            id = 0;
        
		LightLevels[((z * height * depth) + (y * width) + x)] = id;
	}

    inline int GetLightLvl(int x, int z, int y) {
        if(x < 0 || z < 0 || y < 0 || x >= width || z >= depth || y >= height) {
            return 16;
        } 
		return LightLevels[((z * height * depth) + (y * width) + x)];
    }

    void AddLight(int lx, int lz, int ly) {
        vec3_t vec = {lx,ly,lz};
        lights.push_back(vec);

        RGB rgb;
        rgb = { 1, 1, 1 };

        for(int x = vec.x-32;x < vec.x+32;x++) {
            for(int y = vec.y-32;y < vec.y+32;y++) {
                for(int z = vec.z-32;z < vec.z+32;z++) {
                    int dist = sqrtf(pow(x-lx,2) + pow(y-ly,2) + pow(z-lz,2));
                    float level = 28 - ((float)dist);
/*
                    if(level != 0)
                        std::cout << level << " ";
*/
                    if(level < 0)
                        level = 0;

                    if(level > 0) {
                        SetLightLvl(x,z,y,GetLightLvl(x,z,y)+(level));
                    }
                }
            }
        }
    }

    void GenerateDefaultChunk(int x, int y);
    
    void BuildChunk(int x, int z);
    void RebuildLights();

    void RebuildAll();

	void FromBMP(std::string sfile);
	void Draw();

    void LoadBrickMetaData();

    std::vector<vec3_t>lights;

    int from_pair(int x, int z) {
        return (x*CHUNK_SIZE)+z;
    }
private:
    void ProcessMap_Simple();
    
private:
    Camera &camera;
    float fAmbient;
//	int *iBricks;
    int *LightLevels;
    RGB *LightColors;
    TextureArray myTexArray;
    std::vector<std::array<int,6>>BrickLookup;

    std::unordered_map <int, chunk_t>Chunks;



};

#endif
