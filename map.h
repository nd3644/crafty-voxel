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

    inline RGB GetLightColor(int x, int z, int y) {
        if(x < 0 || z < 0 || y < 0 || x >= width || z >= depth || y >= height) {
            return RGB(1,1,1);
        } 
		return LightColors[((z * height * depth) + (y * width) + x)];
    }

    inline void SetLightColor(int x, int z, int y, RGB rgb) {
        if(x < 0 || z < 0 || y < 0 || x >= width || z >= depth || y >= height) {
            return;
        } 
		LightColors[((z * height * depth) + (y * width) + x)] = rgb;
    }

    Mesh *GetChunk(int x, int z) {
        return &myMeshes[x*(width/16)+z];
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
    float fAmbient;
	int *iBricks;
    int *LightLevels;
    RGB *LightColors;
    Mesh ***Chunks;
    Mesh myMeshes[16];
    TextureArray myTexArray;
    std::vector<std::array<int,6>>BrickLookup;
};

#endif
