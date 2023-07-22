#ifndef CUBE_H
#define CUBE_H

#include "types.h"
#include <vector>

extern vec3_t cube_verts[8];
void CompileArr();

// These will store the data for the GL buffers
extern std::vector<vec3_t>vertList;
extern std::vector<vec2_t>uvList;

class Cube
{
	Cube();
	~Cube();
};

#endif
