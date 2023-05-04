#include "cube.h"

vec3_t cube_verts[8];

// These will store the data for the GL buffers
std::vector<vec3_t>vertList;
std::vector<vec2_t>uvList;

void CompileArr() {
	cube_verts[0] = { -1, -1, -1 };
	cube_verts[1] = { 1, -1, -1 };
	cube_verts[2] = { 1, 1, -1 };
	cube_verts[3] = { -1, 1, -1 };

	cube_verts[4] = { -1, -1, 1 };
	cube_verts[5] = { 1, -1, 1 };
	cube_verts[7] = { -1, 1, 1 };
	cube_verts[6] = { 1, 1, 1 };

	uvList.clear();
	vertList.clear();

	auto tex2 = [](float x, float y) {
		vec2_t uv = { x, y };
		uvList.push_back(uv);
	};

	auto vert3 = [](float x, float y, float z) {
		vec3_t v = { x, y, z };
		vertList.push_back(v);
	};

	tex2(0, 0); vert3(cube_verts[0].x, cube_verts[0].y, cube_verts[0].z);
	tex2(1, 0); vert3(cube_verts[1].x, cube_verts[1].y, cube_verts[1].z);
	tex2(0, 1); vert3(cube_verts[3].x, cube_verts[3].y, cube_verts[3].z);

	tex2(0, 1); vert3(cube_verts[3].x, cube_verts[3].y, cube_verts[3].z);
	tex2(1, 0); vert3(cube_verts[1].x, cube_verts[1].y, cube_verts[1].z);
	tex2(1, 1); vert3(cube_verts[2].x, cube_verts[2].y, cube_verts[2].z);

	// back
	tex2(0, 0); vert3(cube_verts[0 + 4].x, cube_verts[0 + 4].y, cube_verts[0 + 4].z);
	tex2(1, 0); vert3(cube_verts[1 + 4].x, cube_verts[1 + 4].y, cube_verts[1 + 4].z);
	tex2(0, 1); vert3(cube_verts[3 + 4].x, cube_verts[3 + 4].y, cube_verts[3 + 4].z);

	tex2(0, 1); vert3(cube_verts[3 + 4].x, cube_verts[3 + 4].y, cube_verts[3 + 4].z);
	tex2(1, 0); vert3(cube_verts[1 + 4].x, cube_verts[1 + 4].y, cube_verts[1 + 4].z);
	tex2(1, 1); vert3(cube_verts[2 + 4].x, cube_verts[2 + 4].y, cube_verts[2 + 4].z);


	// top
	tex2(0, 0); vert3(cube_verts[0].x, cube_verts[0].y, cube_verts[0].z);
	tex2(0, 1); vert3(cube_verts[4].x, cube_verts[4].y, cube_verts[4].z);
	tex2(1, 1); vert3(cube_verts[5].x, cube_verts[5].y, cube_verts[5].z);

	tex2(1, 1); vert3(cube_verts[5].x, cube_verts[5].y, cube_verts[4].z);
	tex2(1, 0); vert3(cube_verts[1].x, cube_verts[1].y, cube_verts[1].z);
	tex2(0, 0); vert3(cube_verts[0].x, cube_verts[0].y, cube_verts[0].z);


	// bottom
	tex2(0, 0); vert3(cube_verts[3].x, cube_verts[3].y, cube_verts[3].z);
	tex2(0, 1); vert3(cube_verts[7].x, cube_verts[7].y, cube_verts[7].z);
	tex2(1, 1); vert3(cube_verts[6].x, cube_verts[6].y, cube_verts[6].z);

	tex2(1, 1); vert3(cube_verts[6].x, cube_verts[6].y, cube_verts[6].z);
	tex2(1, 0); vert3(cube_verts[2].x, cube_verts[2].y, cube_verts[2].z);
	tex2(0, 0); vert3(cube_verts[3].x, cube_verts[3].y, cube_verts[3].z);


	// left
	tex2(0, 0); vert3(cube_verts[0].x, cube_verts[0].y, cube_verts[0].z);
	tex2(1, 0); vert3(cube_verts[4].x, cube_verts[4].y, cube_verts[4].z);
	tex2(1, 1); vert3(cube_verts[7].x, cube_verts[7].y, cube_verts[7].z);

	tex2(0, 0); vert3(cube_verts[0].x, cube_verts[0].y, cube_verts[0].z);
	tex2(0, 1); vert3(cube_verts[3].x, cube_verts[3].y, cube_verts[3].z);
	tex2(1, 1); vert3(cube_verts[7].x, cube_verts[7].y, cube_verts[7].z);

	// right
	tex2(0, 0); vert3(cube_verts[1].x, cube_verts[1].y, cube_verts[1].z);
	tex2(1, 0); vert3(cube_verts[5].x, cube_verts[5].y, cube_verts[5].z);
	tex2(1, 1); vert3(cube_verts[6].x, cube_verts[6].y, cube_verts[6].z);

	tex2(0, 0); vert3(cube_verts[1].x, cube_verts[1].y, cube_verts[1].z);
	tex2(0, 1); vert3(cube_verts[2].x, cube_verts[2].y, cube_verts[2].z);
	tex2(1, 1); vert3(cube_verts[6].x, cube_verts[6].y, cube_verts[6].z);
}
