#include "map.h"

#include "ebmp.h"

#include <iostream>
#include <fstream>
#include "shader.h"
#include "cube.h"
#include "camera.h"
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <libnoise/noise.h>

#include <stack>

Map::Map(Camera &c) : camera(c) {
    height = 32;
    width = 128;
    depth = 128;

    unsigned int num_cores = std::thread::hardware_concurrency();
    if (num_cores == 0) {
        std::cerr << "Unable to determine the number of cores." << std::endl;
        NUM_THREADS = 2;
    } else {
        std::clog << "This PC has " << num_cores << " cores/threads." << std::endl;
        NUM_THREADS = num_cores/2;
    }
    fAmbient = 0.2f;

    viewDist = 3;
    bIsDay = true;
}

Map::~Map() {

}

void Map::FillWater(int fx, int fz, int fy) {
    const int water = IdFromName("water");

    std::stack<vec3i_t>stack;
    stack.push({ fx, fy, fz });

    const auto RECURSIVE_LIMIT = 2048;

    while(stack.size() > 0) {
        if(stack.size() > RECURSIVE_LIMIT) {
            break;
        }
        vec3i_t cur = stack.top();
        stack.pop();
        int x = cur.x;
        int y = cur.y;
        int z = cur.z;

        int initialPixel = GetBrick(x,z,y);
        if(initialPixel == water) {
            continue;
        }

        SetBrick(x,z,y,water);

        vec3_t next;
        if(GetBrick(x+1,z,y) == initialPixel) {
            stack.push({x+1,y,z});
        }

        if(GetBrick(x-1,z,y) == initialPixel) {
            stack.push({x-1,y,z});
        }

        if(GetBrick(x,z+1,y) == initialPixel) {
            stack.push({x,y,z+1});
        }

        if(GetBrick(x,z-1,y) == initialPixel) {
            stack.push({x,y,z-1});
        }
        if(GetBrick(x,z,y-1) == initialPixel) {
            stack.push({x,y-1,z});
        }
    }
}

void Map::chunk_t::Generate(int chunkx, int chunkz, Map &map) {
    if(bGen || bIsCurrentlyGenerating)
        return;

    bIsCurrentlyGenerating = true;
    bool bMount = (rand()%5 == 1) ? true : false;

    int brickType = 1;
    if(bMount) {
        brickType = 1;
    }

    static int counter = 0;
    std::cout << "generating " << chunkx << " , " << chunkz << " c = " << (++counter) << std::endl;

    using namespace noise;

    module::Perlin normalPerlin;
    normalPerlin.SetFrequency(0.1);

    module::Perlin baseFlatTerrain;

    module::ScaleBias flatTerrain;
    flatTerrain.SetSourceModule(0, baseFlatTerrain);
    flatTerrain.SetScale(0.125);

    module::RidgedMulti mountainTerrain;
    mountainTerrain.SetLacunarity(1.5);
    mountainTerrain.SetOctaveCount(1);

    module::Perlin terrainType;
    terrainType.SetFrequency(0.2);
    terrainType.SetPersistence(0.25);

    module::Select finalTerrain;
    finalTerrain.SetSourceModule(0, flatTerrain);
    finalTerrain.SetSourceModule(1, mountainTerrain);
    finalTerrain.SetControlModule(terrainType);
    finalTerrain.SetBounds(0.0, 1000.0);
    finalTerrain.SetEdgeFalloff(0.125);

    std::vector<vec3_t>toFill;
    for (int x = 0; x < CHUNK_SIZE;x++) {
        int xindex = (chunkx*CHUNK_SIZE)+x;
		for (int z = 0; z < CHUNK_SIZE; z++) {
            int zindex = (chunkz*CHUNK_SIZE)+z;
            int height = (((finalTerrain.GetValue((float)xindex / 100.0f,(float)zindex / 100.0f,0.5) + 1) / 2) * MAX_HEIGHT);

            if(height < 4)
                height = 4;
            if(height >= MAX_HEIGHT)
                height = MAX_HEIGHT - 1;
            
            for (int y = height; y > 0; y--) {
				map.SetBrick(xindex, zindex, y, brickType);
			}

            for(int y = 1;y < 8;y++) {
                if(map.GetBrick(xindex,zindex,y) == 0) {
                    map.SetBrick(xindex,zindex,y,map.IdFromName("water"));
                }
            }
		}
	}

    bGen = true;
}

std::vector<std::string> Map::TextureNamesFromFile(std::string filename) {
    std::vector<std::string>result;
    std::ifstream file(filename);

    if(!file.is_open()) {
        std::cerr << "ERROR! Couldn't open file " << filename << std::endl;
        exit(1);
    }

    std::string line;
    while(std::getline(file,line)) {
        result.push_back(line);
        std::cout << "adding texture " << line << std::endl;
    }
    
    return result;
}

void Map::FromBMP(std::string sfile) {
	Eternal::Bmp myBmp(sfile);
	if (myBmp.GetError().size() > 0) {
		std::cerr << "BMP Error: " << myBmp.GetError();
		exit(-1);
	}

    noise::module::Perlin myModule; 
    myModule.SetSeed(123);
    myModule.SetFrequency(0.02);
    myModule.SetPersistence(0.1);
    myModule.SetOctaveCount(3);

    width = myBmp.GetWidth();
    depth = myBmp.GetHeight();

    BrickTextureFilenames = TextureNamesFromFile("brick_textures.txt");
    myTexArray.Load(BrickTextureFilenames);
    LoadBrickMetaData();
}

void Map::RebuildAll() {
    // Build chunks
    for(int x = 0;x < width/CHUNK_SIZE;x++) {
        for(int z = 0;z < depth/CHUNK_SIZE;z++) {
            //BuildChunk(x,z);
        }
    }
}

void Map::BuildChunk(int chunkX, int chunkZ) {
    //std::cout << "building chunk " << "(" << chunkX << " , " << chunkZ << ")" << std::endl;

    int cube_count = 0;
    chunk_t &chunk = Chunks[std::make_pair(chunkX,chunkZ)];
    Mesh &mesh = chunk.mesh;

    Chunks[std::make_pair(chunkX,chunkZ)].bIniialBuild = true;

    mesh.Clean();

    int skipped = 0;
    for (int y = 0; y < MAX_HEIGHT; y++) {
        for (int x = 0; x < CHUNK_SIZE;x++) {
            int xindex = (chunkX*CHUNK_SIZE)+x;
            for (int z = 0; z < CHUNK_SIZE;z++) {
                int zindex = (chunkZ*CHUNK_SIZE)+z;

                // Check the current brick is not empty
                if (GetBrick(xindex, zindex, y) <= 0) {
                    continue;
                }

                // Is the brick surrounding by non-transparent bricks?
                if(GetBrick(xindex-1,zindex,y) != 0 && BrickTransparencies[GetBrick(xindex-1,zindex,y)] == 1
                && GetBrick(xindex+1,zindex,y) != 0 && BrickTransparencies[GetBrick(xindex+1,zindex,y)] == 1
                && GetBrick(xindex,zindex-1,y) != 0 && BrickTransparencies[GetBrick(xindex,zindex-1,y)] == 1
                && GetBrick(xindex,zindex+1,y) != 0 && BrickTransparencies[GetBrick(xindex,zindex+1,y)] == 1
                && GetBrick(xindex,zindex,y+1) != 0 && BrickTransparencies[GetBrick(xindex,zindex,y+1)] == 1
                && GetBrick(xindex,zindex,y-1) != 0 && BrickTransparencies[GetBrick(xindex,zindex,y-1)] == 1) {
                    skipped++;
                    continue;
                }

                float posX = (chunkX*CHUNK_SIZE)+x;
                float posZ = (chunkZ*CHUNK_SIZE)+z;
                mesh.SetTranslation(posX,y,posZ);
                float c = 1.0f;
                int brickID = GetBrick(xindex,zindex,y);

                float trans = BrickTransparencies[brickID-1];

                if(trans != 1) {
                    continue;
                }
                int len = 1;
/*
                for(int i = 1;i < CHUNK_SIZE-1;i++) {
                    if(z < CHUNK_SIZE-1-i
                    && GetBrick(xindex,zindex,y) == GetBrick(xindex,zindex+i,y)
                    && GetLightLevel(xindex,zindex,y) == GetLightLevel(xindex,zindex+i,y)) {
                        len++;
                    }
                    else {
                        break;
                    }
                }*/
                z += len-1;

                for(int i = 0;i < 6*6;i++)
                    mesh.Index1(BrickLookup[brickID-1][i/6]);
                fAmbient = 0.05f;
                float lv = fAmbient + (float)GetLightLevel(xindex,zindex,y+1) / (float)MAX_LIGHT_LEVEL;
                if(lv > 1)
                    lv = 1;
                // Draw top
                mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans);mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans);
                mesh.TexCoord2(0, len);     mesh.Vert3(-0.5, 0.5, len);
                mesh.TexCoord2(1, len);     mesh.Vert3(0.5, 0.5, len);
                mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, -0);

                mesh.TexCoord2(0, len);     mesh.Vert3(-0.5, 0.5, len);
                mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, 0);
                mesh.TexCoord2(0, 0);         mesh.Vert3(-0.5, 0.5, -0);

                lv = fAmbient + (float)GetLightLevel(xindex,zindex,y-1) / (float)MAX_LIGHT_LEVEL;
                if(lv > 1)
                    lv = 1;
                // Draw bottom
                mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans);mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans);
                mesh.TexCoord2(0, len);     mesh.Vert3(-0.5, -0.5, len);
                mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, -0.5, -0);
                mesh.TexCoord2(1, len);     mesh.Vert3(0.5, -0.5, len);

                mesh.TexCoord2(0, len);     mesh.Vert3(-0.5, -0.5, len);
                mesh.TexCoord2(0, 0);         mesh.Vert3(-0.5, -0.5, -0);
                mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, -0.5, -0);


                lv = fAmbient + (float)GetLightLevel(xindex+1,zindex,y) / (float)MAX_LIGHT_LEVEL;
                if(lv > 1)
                    lv = 1;
                // Draw left
                mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans);mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans);
                mesh.TexCoord2(0, 1);         mesh.Vert3(0.5, -0.5, 0);
                mesh.TexCoord2(len, 0);     mesh.Vert3(0.5, 0.5, len);
                mesh.TexCoord2(len, 1);     mesh.Vert3(0.5, -0.5, len);

                mesh.TexCoord2(len, 0);     mesh.Vert3(0.5, 0.5, len);
                mesh.TexCoord2(0, 1);         mesh.Vert3(0.5, -0.5, -0);
                mesh.TexCoord2(0, 0);         mesh.Vert3(0.5, 0.5, -0);

                lv = fAmbient + (float)GetLightLevel(xindex-1,zindex,y) / (float)MAX_LIGHT_LEVEL;
                if(lv > 1)
                    lv = 1;
                // Draw right
                mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans);mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans);
                mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, -0);
                mesh.TexCoord2(len, 1);     mesh.Vert3(-0.5, -0.5, len);
                mesh.TexCoord2(len, 0);     mesh.Vert3(-0.5, 0.5, len);

                mesh.TexCoord2(len, 0);     mesh.Vert3(-0.5, 0.5, len);
                mesh.TexCoord2(0, 0);         mesh.Vert3(-0.5, 0.5, -0);
                mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, -0);

                lv = fAmbient + (float)GetLightLevel(xindex,zindex-1,y) / (float)MAX_LIGHT_LEVEL;
                if(lv > 1)
                    lv = 1;
                // Draw back
                mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans);mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans);
                mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, -0);
                mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, -0);
                mesh.TexCoord2(1, 1);         mesh.Vert3(0.5, -0.5, -0);

                mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, -0);
                mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, -0);
                mesh.TexCoord2(0, 0);         mesh.Vert3(-0.5, 0.5, -0);

                lv = fAmbient + (float)GetLightLevel(xindex,zindex+1,y) / (float)MAX_LIGHT_LEVEL;
                if(lv > 1)
                    lv = 1;
                // Draw front
                mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans);mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans);
                mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, len);
                mesh.TexCoord2(1, 1);         mesh.Vert3(0.5, -0.5, len);
                mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, len);

                mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, len);
                mesh.TexCoord2(0, 0);         mesh.Vert3(-0.5, 0.5, len);
                mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, len);
                cube_count++;
            }
        }
    }
    mesh.BindBufferData();
    glFinish();

    // Transparent pass
//    BuildChunkTrans(chunkX,chunkZ);

    std::cout << "built " << cube_count << std::endl;
}

void Map::BuildChunkTrans(int chunkX, int chunkZ) {
    Mesh &mesh = Chunks[std::make_pair(chunkX,chunkZ)].transMesh;

    Chunks[std::make_pair(chunkX,chunkZ)].bIniialBuild = true;

    mesh.Clean();

    int skipped = 0;
    for (int y = 0; y < MAX_HEIGHT; y++) {
        for (int x = 0; x < CHUNK_SIZE;x++) {
            int xindex = (chunkX*CHUNK_SIZE)+x;
            for (int z = 0; z < CHUNK_SIZE;z++) {
                int zindex = (chunkZ*CHUNK_SIZE)+z;

                // Check the current brick is not empty
                if (GetBrick(xindex, zindex, y) <= 0) {
                    continue;
                }
                float posX = (chunkX*CHUNK_SIZE)+x;
                float posZ = (chunkZ*CHUNK_SIZE)+z;
                mesh.SetTranslation(posX,y,posZ);
                float c = 1.0f;
                int brickID = GetBrick(xindex,zindex,y);

                float trans = BrickTransparencies[brickID-1];

                if(trans == 1) {
                    continue;
                }
                int len = 1;

                for(int i = 0;i < 6*6;i++)
                    mesh.Index1(BrickLookup[brickID-1][i/6]);

                float lv = fAmbient;
                if(lv > 1)
                    lv = 1;
                if(lv < fAmbient)
                    lv = fAmbient;

                // Draw top
                if(GetBrick(xindex,zindex,y+1) == 0) {
                    mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans);mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans);
                    mesh.TexCoord2(0, len);     mesh.Vert3(-0.5, 0.5, len);
                    mesh.TexCoord2(1, len);     mesh.Vert3(0.5, 0.5, len);
                    mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, -0);

                    mesh.TexCoord2(0, len);     mesh.Vert3(-0.5, 0.5, len);
                    mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, 0);
                    mesh.TexCoord2(0, 0);         mesh.Vert3(-0.5, 0.5, -0);
                }
            
                if(GetBrick(xindex,zindex,y-1) == 0) {
                // Draw bottom
                mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans);mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans);
                mesh.TexCoord2(0, len);     mesh.Vert3(-0.5, -0.5, len);
                mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, -0.5, -0);
                mesh.TexCoord2(1, len);     mesh.Vert3(0.5, -0.5, len);

                mesh.TexCoord2(0, len);     mesh.Vert3(-0.5, -0.5, len);
                mesh.TexCoord2(0, 0);         mesh.Vert3(-0.5, -0.5, -0);
                mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, -0.5, -0);
                }

                lv -= 0.2f;
                // Draw left
                if(GetBrick(xindex+1,zindex,y) == 0) {
                mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans);mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans);
                mesh.TexCoord2(0, 1);         mesh.Vert3(0.5, -0.5, 0);
                mesh.TexCoord2(len, 0);     mesh.Vert3(0.5, 0.5, len);
                mesh.TexCoord2(len, 1);     mesh.Vert3(0.5, -0.5, len);

                mesh.TexCoord2(len, 0);     mesh.Vert3(0.5, 0.5, len);
                mesh.TexCoord2(0, 1);         mesh.Vert3(0.5, -0.5, -0);
                mesh.TexCoord2(0, 0);         mesh.Vert3(0.5, 0.5, -0);
                }

                if(GetBrick(xindex-1,zindex,y) == 0) {
                // Draw right
                mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans);mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans);
                mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, -0);
                mesh.TexCoord2(len, 1);     mesh.Vert3(-0.5, -0.5, len);
                mesh.TexCoord2(len, 0);     mesh.Vert3(-0.5, 0.5, len);

                mesh.TexCoord2(len, 0);     mesh.Vert3(-0.5, 0.5, len);
                mesh.TexCoord2(0, 0);         mesh.Vert3(-0.5, 0.5, -0);
                mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, -0);
                }

                if(GetBrick(xindex,zindex-1,y) == 0) {
                // Draw back
                mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans);mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans);
                mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, -0);
                mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, -0);
                mesh.TexCoord2(1, 1);         mesh.Vert3(0.5, -0.5, -0);

                mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, -0);
                mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, -0);
                mesh.TexCoord2(0, 0);         mesh.Vert3(-0.5, 0.5, -0);
                }

                if(GetBrick(xindex,zindex+1,y) == 0) {
                // Draw front
                mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans);mesh.Color4(lv, lv, lv, trans); mesh.Color4(lv, lv, lv, trans);
                mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, len);
                mesh.TexCoord2(1, 1);         mesh.Vert3(0.5, -0.5, len);
                mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, len);

                mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, len);
                mesh.TexCoord2(0, 0);         mesh.Vert3(-0.5, 0.5, len);
                mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, len);
                }
            }
        }
    }
    mesh.BindBufferData();
    glFinish();
}

void Map::RebuildLights() {
    for(auto l: lights) {
    }
}

void Map::Draw() {
    int sX = ((int)camera.position.x / CHUNK_SIZE);
    int sZ = ((int)camera.position.z / CHUNK_SIZE);

    if(SDL_GetKeyboardState(0)[SDL_SCANCODE_J]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else if(SDL_GetKeyboardState(0)[SDL_SCANCODE_K]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    int ChunksDrawn = 0;

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    for(int x = sX - viewDist;x < sX+viewDist;x++) {
        for(int z = sZ-viewDist;z < sZ+viewDist;z++) {
            auto index = std::make_pair(x,z);
            auto &chunk = Chunks[index];
            if(chunk.bIniialBuild == false || chunk.mesh.IsEmpty()) {
                continue;
            }
            Mesh &mesh = chunk.mesh;
            mesh.Draw(Mesh::MODE_TRIANGLES);
        }
    }

    for(int x = sX - viewDist;x < sX+viewDist;x++) {
        for(int z = sZ-viewDist;z < sZ+viewDist;z++) {
            auto index = std::make_pair(x,z);
            auto &chunk = Chunks[index];
            if(chunk.bIniialBuild == false || chunk.mesh.IsEmpty()) {
                continue;
            }
            chunk.transMesh.Draw(Mesh::MODE_TRIANGLES);
        }
    }


    if(SDL_GetKeyboardState(0)[SDL_SCANCODE_T]) {
        for(int x = sX - viewDist;x < sX+viewDist;x++) {
            for(int z = sZ-viewDist;z < sZ+viewDist;z++) {
                BuildChunk(x,z);
            }
        }
    }

    glFinish();
}

void Map::RunBuilder() {
    int sX = ((int)camera.position.x / CHUNK_SIZE);
    int sZ = ((int)camera.position.z / CHUNK_SIZE);

    int build_count = 0;
    for(int x = sX - viewDist;x < sX+viewDist;x++) {
        for(int z = sZ-viewDist;z < sZ+viewDist;z++) {
            auto index = std::make_pair(x,z);
            auto &chunk = Chunks[index];

            if(!chunk.bGen) {
                chunk.Generate(x,z,*this);
            }

            if(chunk.bIniialBuild == false || chunk.mesh.IsEmpty()) {
                if(build_count < 1) {
                    BuildChunk(x,z);
                    build_count++;
                }
                continue;
            }
        }
    }
}

void Map::LoadBrickMetaData() {
    std::ifstream infile("bricks.txt");
    float transparency = 0;
    while(!infile.eof()) {

        // Load the string name
        std::string brickName;
        infile >> brickName;
        BrickNameMap[brickName] = BrickNameMap.size() + 1;

        // Face information
        std::array<int,6>arr;
        for(int i = 0;i < 6;i++) {
            infile >> arr[i];
        }
        infile >> transparency;
        BrickTransparencies.push_back(transparency);
        BrickLookup.push_back(arr);
    }
}

std::vector<std::array<int,6>> Map::GetLookupArr() const {
    return BrickLookup;
}

bool Map::IsDay() const {
    return bIsDay;
}

void Map::SetDay(bool b) {
    bIsDay = b;
}

