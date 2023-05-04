#ifndef MAP_H
#define MAP_H

#include "ebmp.h"
#include <array>

class Map
{
public:
	Map();
	~Map();

	static const int height = 64;
	static const int width = 128;
	static const int depth = 128;

	inline void SetBrick(int x, int z, int y, int id) {
		iBricks[((z * height * depth) + (y * width) + x)] = id;
	}


	inline int GetBrick(int x, int z, int y) {
		return iBricks[((z * height * depth) + (y * width) + x)];
	}

	void FromBMP(std::string sfile);
	void Draw();
private:	
	int *iBricks;

};

#endif
