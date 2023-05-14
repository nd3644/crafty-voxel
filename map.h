#ifndef MAP_H
#define MAP_H

#include "ebmp.h"
#include <array>
#include "mesh.h"
#include "texture_array.h"


class Map
{
public:
	Map();
	~Map();

	 int height;
	 int width;
	 int depth;

    int NUM_THREADS;

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

	void FromBMP(std::string sfile);
	void Draw();

    void LoadBrickMetaData();

private:
    void DrawSection(int i);
    void ProcessMap_Simple();
    
private:
	int *iBricks;
    Mesh myMeshes[16];
    TextureArray myTexArray;
    std::vector<std::array<int,6>>BrickLookup;
};

#endif
