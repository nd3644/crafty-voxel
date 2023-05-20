#ifndef MAP_H
#define MAP_H

#include "ebmp.h"
#include <array>
#include "mesh.h"
#include "texture_array.h"
#include <iostream>
#include <map>
#include <cmath>

class Map
{
public:
	Map();
	~Map();

	 int height;
	 int width;
	 int depth;

    int NUM_THREADS;

    static constexpr int CHUNK_SIZE = 128;

	inline void SetBrick(int x, int z, int y, int id) {
        if(x < 0 || z < 0 || y < 0 || x >= width || z >= depth || y >= height) {
            return;
        } 
		iBricks[((z * height * depth) + (y * width) + x)] = id;
	}


	inline int GetBrick(int x, int z, int y) {
        if(x < 0 || z < 0 || y < 0 || x >= width || z >= depth || y >= height) {
            return -1;
        } 
		return iBricks[((z * height * depth) + (y * width) + x)];
	}

    inline void SetLightLvl(int x, int z, int y, int id) {
        if(x < 0 || z < 0 || y < 0 || x >= width || z >= depth || y >= height) {
            return;
        } 
		LightLevels[((z * height * depth) + (y * width) + x)] = id;
	}

    inline int GetLightLvl(int x, int z, int y) {
        if(x < 0 || z < 0 || y < 0 || x >= width || z >= depth || y >= height) {
            return 16;
        } 
		return LightLevels[((z * height * depth) + (y * width) + x)];
    }

    Mesh *GetChunk(int x, int z) {
        return &myMeshes[x*(width/16)+z];
    }

    void AddLight(int lx, int lz, int ly) {
        vec3_t vec = {lx,ly,lz};
        lights.push_back(vec);

        float min = -1;
        float max = -1;
        for(int x = vec.x-16;x < vec.x+16;x++) {
            for(int y = vec.y-16;y < vec.y+16;y++) {
                for(int z = vec.z-16;z < vec.z+16;z++) {

                    int dist = sqrtf(pow(x-lx,2) + pow(y-ly,2) + pow(z-lz,2))*2;

                    if(dist < min || min == -1) {
                        min = dist;
                    }
                    if(dist > max || max == -1) {
                        max = dist;
                    }
                    if(dist < 28) {
                        SetLightLvl(x,z,y,GetLightLvl(x,z,y)+((float)28-dist));
                    }
                }
            }
        }
        std::cout << "min: " << min << std::endl;
        std::cout << "max: " << max << std::endl;
        ///exit(0);
    }
    
    void BuildChunk(int x, int z);
    void RebuildLights();

    void RebuildAll();

	void FromBMP(std::string sfile);
	void Draw();

    void LoadBrickMetaData();

    std::vector<vec3_t>lights;
private:
    void DrawSection(int i);
    void ProcessMap_Simple();
    
private:
	int *iBricks;
    int *LightLevels;
    Mesh ***Chunks;
    Mesh myMeshes[16];
    TextureArray myTexArray;
    std::vector<std::array<int,6>>BrickLookup;
};

#endif
