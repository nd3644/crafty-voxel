#include "map.h"

#include "ebmp.h"

#include <iostream>
#include <fstream>
#include "shader.h"
#include "cube.h"
#include "camera.h"
#include "globals.h"
#include <GL/glew.h>
#include <algorithm>

#include <immintrin.h>

#include <stack>

Map::Map(Camera &c) : camera(c) {
    height = MAX_HEIGHT;
    width = 128;
    depth = 128;

    unsigned int num_cores = std::thread::hardware_concurrency();
    if (num_cores == 0) {
        std::cerr << "Unable to determine the number of cores." << std::endl;
        NUM_THREADS = 2;
    } else {
        std::clog << "This PC has " << num_cores << " cores/threads." << std::endl;
        NUM_THREADS = num_cores;
    }

    std::cout << "Map::Map(): size: " << Chunks.size() << std::endl;

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

    width = myBmp.GetWidth();
    depth = myBmp.GetHeight();

    BrickTextureFilenames = TextureNamesFromFile("brick_textures.txt");
    myTexArray.Load(BrickTextureFilenames);
    LoadBrickMetaData();

//    RebuildAllVisible();
}

void Map::RebuildAll() {
    // Build chunks
    for(int x = 0;x < width/CHUNK_SIZE;x++) {
        for(int z = 0;z < depth/CHUNK_SIZE;z++) {
            BuildChunk(x,z);
        }
    }
}

void Map::BuildChunkAO(int chunkX, int chunkZ) {
    if(chunkX < 0 || chunkZ < 0) {
        return;
    }
    chunk_t &chunk = Chunks[std::make_pair(chunkX,chunkZ)];
    goto finishAO;
/*    for (int y = 0; y < MAX_HEIGHT; y++) {
        for (int x = 0; x < CHUNK_SIZE;x++) {
            int xindex = (chunkX*CHUNK_SIZE)+x;
            for (int z = 0; z < CHUNK_SIZE;z++) {
                int zindex = (chunkZ*CHUNK_SIZE)+z;

                int brickID = GetBrick(xindex,zindex,y);
                if (GetBrick(xindex, zindex, y) <= 0                // Is the brick air
                || IsBrickSurroundedByOpaque(xindex,zindex,y)              // Is it visible or occluded by bricks?
                ) {            // Is it opaque? Transparencies are handled in a seperate pass
                    continue;
                }

                // Set default AO values to 1.0f (no AO at all)
                for(int f = 0;f < 6;f++)
                    for(int v = 0;v < 4;v++)
                        chunk.ambientVecs[x][y][z][f][v] = 100;

                if(!gEnableAO) {
                    continue;
                }

                uint8_t ambShade = gAOLevel;

                // Top
                uint8_t (&topamb)[4] = chunk.ambientVecs[x][y][z][0];
                if(GetBrick(xindex-1,zindex,y+1) != 0 || GetBrick(xindex,zindex+1,y+1) != 0 || GetBrick(xindex-1,zindex+1,y+1) != 0) {
                    topamb[0] = ambShade;
                }
                if(GetBrick(xindex+1,zindex,y+1) != 0 || GetBrick(xindex,zindex+1,y+1) != 0 || GetBrick(xindex+1,zindex+1,y+1) != 0) {
                    topamb[1] = ambShade;
                }
                if(GetBrick(xindex-1,zindex,y+1) != 0 || GetBrick(xindex,zindex-1,y+1) != 0 || GetBrick(xindex-1,zindex-1,y+1) != 0) {
                    topamb[2] = ambShade;
                }
                if(GetBrick(xindex+1,zindex,y+1) != 0 || GetBrick(xindex,zindex-1,y+1) != 0 || GetBrick(xindex+1,zindex-1,y+1) != 0) {
                    topamb[3] = ambShade;
                }
                
                // Left
                uint8_t (&leftamb)[4] = chunk.ambientVecs[x][y][z][2];
                //float topamb[0] = lv, topamb[1] = lv, topamb[2] = lv, topamb[3] = lv;
                if(GetBrick(xindex-1,zindex-1,y) != 0 || GetBrick(xindex-1,zindex-1,y+1) != 0 || GetBrick(xindex-1,zindex,y+1) != 0) {
                    leftamb[0] = ambShade;
                }
                if(GetBrick(xindex-1,zindex+1,y) != 0 || GetBrick(xindex-1,zindex+1,y+1) != 0 || GetBrick(xindex-1,zindex,y+1) != 0) {
                    leftamb[1] = ambShade;
                }
                if(GetBrick(xindex-1,zindex+1,y) != 0 || GetBrick(xindex-1,zindex+1,y-1) != 0 || GetBrick(xindex-1,zindex,y-1) != 0) {
                    leftamb[2] = ambShade;
                }
                if(GetBrick(xindex-1,zindex-1,y) != 0 || GetBrick(xindex-1,zindex-1,y-1) != 0 || GetBrick(xindex-1,zindex,y-1) != 0) {
                    leftamb[3] = ambShade;
                }

                // ...
                // Right
                uint8_t (&rightamb)[4] = chunk.ambientVecs[x][y][z][3];
                //float topamb[0] = lv, topamb[1] = lv, topamb[2] = lv, topamb[3] = lv;
                if(GetBrick(xindex+1,zindex-1,y) != 0 || GetBrick(xindex+1,zindex-1,y+1) != 0 || GetBrick(xindex+1,zindex,y+1) != 0) {
                    rightamb[0] = ambShade;
                }
                if(GetBrick(xindex+1,zindex+1,y) != 0 || GetBrick(xindex+1,zindex+1,y+1) != 0 || GetBrick(xindex+1,zindex,y+1) != 0) {
                    rightamb[1] = ambShade;
                }
                if(GetBrick(xindex+1,zindex+1,y) != 0 || GetBrick(xindex+1,zindex+1,y-1) != 0 || GetBrick(xindex+1,zindex,y-1) != 0) {
                    rightamb[2] = ambShade;
                }
                if(GetBrick(xindex+1,zindex-1,y) != 0 || GetBrick(xindex+1,zindex-1,y-1) != 0 || GetBrick(xindex+1,zindex,y-1) != 0) {
                    rightamb[3] = ambShade;
                }

                // back
                uint8_t (&backamb)[4] = chunk.ambientVecs[x][y][z][4];
                //float topamb[0] = lv, topamb[1] = lv, topamb[2] = lv, topamb[3] = lv;
                if(GetBrick(xindex,zindex-1,y+1) != 0 || GetBrick(xindex+1,zindex-1,y+1) != 0 || GetBrick(xindex+1,zindex-1,y) != 0) {
                    backamb[0] = ambShade;
                }
                if(GetBrick(xindex,zindex-1,y+1) != 0 || GetBrick(xindex-1,zindex-1,y+1) != 0 || GetBrick(xindex-1,zindex-1,y) != 0) {
                    backamb[1] = ambShade;
                }
                if(GetBrick(xindex,zindex-1,y-1) != 0 || GetBrick(xindex+1,zindex-1,y-1) != 0 || GetBrick(xindex+1,zindex-1,y) != 0) {
                    backamb[2] = ambShade;
                }
                if(GetBrick(xindex,zindex-1,y-1) != 0 || GetBrick(xindex-1,zindex-1,y-1) != 0 || GetBrick(xindex-1,zindex-1,y) != 0) {
                    backamb[3] = ambShade;
                }

                // Front
                uint8_t (&frontamb)[4] = chunk.ambientVecs[x][y][z][5];
                if(GetBrick(xindex,zindex+1,y+1) != 0 || GetBrick(xindex+1,zindex+1,y+1) != 0 || GetBrick(xindex+1,zindex+1,y) != 0) {
                    frontamb[0] = ambShade;
                }
                if(GetBrick(xindex,zindex+1,y+1) != 0 || GetBrick(xindex-1,zindex+1,y+1) != 0 || GetBrick(xindex-1,zindex+1,y) != 0) {
                    frontamb[1] = ambShade;
                }
                if(GetBrick(xindex,zindex+1,y-1) != 0 || GetBrick(xindex+1,zindex+1,y-1) != 0 || GetBrick(xindex+1,zindex+1,y) != 0) {
                    frontamb[2] = ambShade;
                }
                if(GetBrick(xindex,zindex+1,y-1) != 0 || GetBrick(xindex-1,zindex+1,y-1) != 0 || GetBrick(xindex-1,zindex+1,y) != 0) {
                    frontamb[3] = ambShade;
                }
            }
        }
    }*/

    finishAO:
    chunk.bInitialAOBuild = true;
    chunk.curStage = chunk_t::BUILD_STAGE;
}

Map::brick_ao_t Map::GetBrickAO(int xindex, int zindex, int y) {

    brick_ao_t ao;

    //int brickID = GetBrick(xindex,zindex,y);
    if (GetBrick(xindex, zindex, y) <= 0                // Is the brick air
    || IsBrickSurroundedByOpaque(xindex,zindex,y)              // Is it visible or occluded by bricks?
    ) {            // Is it opaque? Transparencies are handled in a seperate pass
        return ao;
    }

    // Set default AO values to 1.0f (no AO at all)
    for(int f = 0;f < 6;f++)
        for(int v = 0;v < 4;v++)
            ao.ambientVecs[f][v] = 100;

    if(!gEnableAO) {
        return ao;
    }

    uint8_t ambShade = gAOLevel;

    // Top
    uint8_t (&topamb)[4] = ao.ambientVecs[0];
    if(GetBrick(xindex-1,zindex,y+1) != 0 || GetBrick(xindex,zindex+1,y+1) != 0 || GetBrick(xindex-1,zindex+1,y+1) != 0) {
        topamb[0] = ambShade;
    }
    if(GetBrick(xindex+1,zindex,y+1) != 0 || GetBrick(xindex,zindex+1,y+1) != 0 || GetBrick(xindex+1,zindex+1,y+1) != 0) {
        topamb[1] = ambShade;
    }
    if(GetBrick(xindex-1,zindex,y+1) != 0 || GetBrick(xindex,zindex-1,y+1) != 0 || GetBrick(xindex-1,zindex-1,y+1) != 0) {
        topamb[2] = ambShade;
    }
    if(GetBrick(xindex+1,zindex,y+1) != 0 || GetBrick(xindex,zindex-1,y+1) != 0 || GetBrick(xindex+1,zindex-1,y+1) != 0) {
        topamb[3] = ambShade;
    }
    
    // Left
    uint8_t (&leftamb)[4] = ao.ambientVecs[2];
    //float topamb[0] = lv, topamb[1] = lv, topamb[2] = lv, topamb[3] = lv;
    if(GetBrick(xindex-1,zindex-1,y) != 0 || GetBrick(xindex-1,zindex-1,y+1) != 0 || GetBrick(xindex-1,zindex,y+1) != 0) {
        leftamb[0] = ambShade;
    }
    if(GetBrick(xindex-1,zindex+1,y) != 0 || GetBrick(xindex-1,zindex+1,y+1) != 0 || GetBrick(xindex-1,zindex,y+1) != 0) {
        leftamb[1] = ambShade;
    }
    if(GetBrick(xindex-1,zindex+1,y) != 0 || GetBrick(xindex-1,zindex+1,y-1) != 0 || GetBrick(xindex-1,zindex,y-1) != 0) {
        leftamb[2] = ambShade;
    }
    if(GetBrick(xindex-1,zindex-1,y) != 0 || GetBrick(xindex-1,zindex-1,y-1) != 0 || GetBrick(xindex-1,zindex,y-1) != 0) {
        leftamb[3] = ambShade;
    }

    // ...
    // Right
    uint8_t (&rightamb)[4] = ao.ambientVecs[3];
    //float topamb[0] = lv, topamb[1] = lv, topamb[2] = lv, topamb[3] = lv;
    if(GetBrick(xindex+1,zindex-1,y) != 0 || GetBrick(xindex+1,zindex-1,y+1) != 0 || GetBrick(xindex+1,zindex,y+1) != 0) {
        rightamb[0] = ambShade;
    }
    if(GetBrick(xindex+1,zindex+1,y) != 0 || GetBrick(xindex+1,zindex+1,y+1) != 0 || GetBrick(xindex+1,zindex,y+1) != 0) {
        rightamb[1] = ambShade;
    }
    if(GetBrick(xindex+1,zindex+1,y) != 0 || GetBrick(xindex+1,zindex+1,y-1) != 0 || GetBrick(xindex+1,zindex,y-1) != 0) {
        rightamb[2] = ambShade;
    }
    if(GetBrick(xindex+1,zindex-1,y) != 0 || GetBrick(xindex+1,zindex-1,y-1) != 0 || GetBrick(xindex+1,zindex,y-1) != 0) {
        rightamb[3] = ambShade;
    }

    // back
    uint8_t (&backamb)[4] = ao.ambientVecs[4];
    //float topamb[0] = lv, topamb[1] = lv, topamb[2] = lv, topamb[3] = lv;
    if(GetBrick(xindex,zindex-1,y+1) != 0 || GetBrick(xindex+1,zindex-1,y+1) != 0 || GetBrick(xindex+1,zindex-1,y) != 0) {
        backamb[0] = ambShade;
    }
    if(GetBrick(xindex,zindex-1,y+1) != 0 || GetBrick(xindex-1,zindex-1,y+1) != 0 || GetBrick(xindex-1,zindex-1,y) != 0) {
        backamb[1] = ambShade;
    }
    if(GetBrick(xindex,zindex-1,y-1) != 0 || GetBrick(xindex+1,zindex-1,y-1) != 0 || GetBrick(xindex+1,zindex-1,y) != 0) {
        backamb[2] = ambShade;
    }
    if(GetBrick(xindex,zindex-1,y-1) != 0 || GetBrick(xindex-1,zindex-1,y-1) != 0 || GetBrick(xindex-1,zindex-1,y) != 0) {
        backamb[3] = ambShade;
    }

    // Front
    uint8_t (&frontamb)[4] = ao.ambientVecs[5];
    if(GetBrick(xindex,zindex+1,y+1) != 0 || GetBrick(xindex+1,zindex+1,y+1) != 0 || GetBrick(xindex+1,zindex+1,y) != 0) {
        frontamb[0] = ambShade;
    }
    if(GetBrick(xindex,zindex+1,y+1) != 0 || GetBrick(xindex-1,zindex+1,y+1) != 0 || GetBrick(xindex-1,zindex+1,y) != 0) {
        frontamb[1] = ambShade;
    }
    if(GetBrick(xindex,zindex+1,y-1) != 0 || GetBrick(xindex+1,zindex+1,y-1) != 0 || GetBrick(xindex+1,zindex+1,y) != 0) {
        frontamb[2] = ambShade;
    }
    if(GetBrick(xindex,zindex+1,y-1) != 0 || GetBrick(xindex-1,zindex+1,y-1) != 0 || GetBrick(xindex-1,zindex+1,y) != 0) {
        frontamb[3] = ambShade;
    }

    return ao;
}

void Map::BuildChunk(int chunkX, int chunkZ) {
    if(chunkX < 0 || chunkZ < 0) {
        return;
    }

/*    std::cout << "building chunk " << "(" << chunkX << " , " << chunkZ << ")" << std::endl;
    std::cout << "size: " << Chunks.size() << std::endl;*/

    int cube_count = 0;
    chunk_t &chunk = Chunks[std::make_pair(chunkX,chunkZ)];
    Mesh &mesh = chunk.mesh;

    chunk.bIniialBuild = true;

    mesh.Clean();
    for (int y = 2; y < MAX_HEIGHT; y++) {
        for (int x = 0; x < CHUNK_SIZE;x++) {
            int xindex = (chunkX*CHUNK_SIZE)+x;
            for (int z = 0; z < CHUNK_SIZE;z++) {
                int zindex = (chunkZ*CHUNK_SIZE)+z;

                int brickID = GetBrick(xindex,zindex,y);
                if (brickID <= 0                                    // Is the brick air
                || IsBrickSurroundedByOpaque(xindex,zindex,y)                       // Is it visible or occluded by bricks?
                ) {            // Is it opaque? Transparencies are handled in a seperate pass
                    continue;
                }

                int brickLightLevel = GetLightLevel(xindex,zindex,y);

                float posX = (chunkX*CHUNK_SIZE)+x;
                float posZ = (chunkZ*CHUNK_SIZE)+z;
                mesh.SetTranslation(posX,y,posZ);

                brick_ao_t curBrickAO = GetBrickAO(xindex,zindex,y);


                int len = 1;
                for(int i = 1;i < CHUNK_SIZE;i++) {
                    if(z < CHUNK_SIZE-i
                    && brickID == GetBrick(xindex,zindex+i,y)
                    && brickLightLevel == GetLightLevel(xindex,zindex+i,y)) {

                        if(GetBrickAO(xindex,zindex+i,y) != curBrickAO) {
                            goto skip;
                        }
                        len++;
                    }
                    else {
                        break;
                    }
                }
                skip:

                z += len-1;

                for(int i = 0;i < 6*6;i++)
                    mesh.Index1(BrickLookup[brickID-1][i/6]);

                float lv = fAmbient + (float)GetLightLevel(xindex,zindex,y+1) / (float)MAX_LIGHT_LEVEL;
                lv -= chunk.heatShift;
                // Draw top

                
                uint8_t (&topamb_a)[4] = curBrickAO.ambientVecs[0];
                float amb[4];
                for(int k = 0;k < 4;k++) {
                    amb[k] = ((float)topamb_a[k] / 100.0f) * lv;
                }
                
                if(amb[0] + amb[3] > amb[1] + amb[2]) {
                    mesh.Color4(amb[2], amb[2], amb[2], 1); mesh.TexCoord2(0, 0);       mesh.Vert3(-0.5, 0.5, 0); // left back 0
                    mesh.Color4(amb[1], amb[1], amb[1], 1); mesh.TexCoord2(1, len);     mesh.Vert3(0.5, 0.5, len);  //  front right 2
                    mesh.Color4(amb[3], amb[3], amb[3], 1); mesh.TexCoord2(1, 0);       mesh.Vert3(0.5, 0.5, 0);   // right back 1 
                    mesh.Color4(amb[1], amb[1], amb[1], 1); mesh.TexCoord2(1, len);     mesh.Vert3(0.5, 0.5, len);  //  front right 2
                    mesh.Color4(amb[2], amb[2], amb[2], 1); mesh.TexCoord2(0, 0);       mesh.Vert3(-0.5, 0.5, 0); // left back 0
                    mesh.Color4(amb[0], amb[0], amb[0], 1); mesh.TexCoord2(0, len);     mesh.Vert3(-0.5, 0.5, len);  // front left 3
                }
                else {
                    mesh.Color4(amb[0], amb[0], amb[0], 1); mesh.TexCoord2(0, len);     mesh.Vert3(-0.5, 0.5, len); // front left
                    mesh.Color4(amb[1], amb[1], amb[1], 1); mesh.TexCoord2(1, len);     mesh.Vert3(0.5, 0.5, len);  //  front right
                    mesh.Color4(amb[3], amb[3], amb[3], 1); mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, -0); // right back
                    mesh.Color4(amb[0], amb[0], amb[0], 1); mesh.TexCoord2(0, len);     mesh.Vert3(-0.5, 0.5, len);  // front left
                    mesh.Color4(amb[3], amb[3], amb[3], 1); mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, 0);   // right back
                    mesh.Color4(amb[2], amb[2], amb[2], 1); mesh.TexCoord2(0, 0);         mesh.Vert3(-0.5, 0.5, -0); // left back
                }

                lv = fAmbient + (float)GetLightLevel(xindex,zindex,y-1) / (float)MAX_LIGHT_LEVEL;
                lv -= chunk.heatShift;

                uint8_t (&botamb_a)[4] = curBrickAO.ambientVecs[1];
                for(int k = 0;k < 4;k++) {
                    amb[k] = ((float)botamb_a[k] / 100.0f) * lv;
                }


                // TODO: add a/o here
                // Draw bottom
                mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1);mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1);
                mesh.TexCoord2(0, len);     mesh.Vert3(-0.5, -0.5, len);
                mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, -0.5, -0);
                mesh.TexCoord2(1, len);     mesh.Vert3(0.5, -0.5, len);

                mesh.TexCoord2(0, len);     mesh.Vert3(-0.5, -0.5, len);
                mesh.TexCoord2(0, 0);         mesh.Vert3(-0.5, -0.5, -0);
                mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, -0.5, -0);

                lv = fAmbient + (float)GetLightLevel(xindex-1,zindex,y) / (float)MAX_LIGHT_LEVEL;
                lv -= chunk.heatShift;

                uint8_t (&leftamb_a)[4] = curBrickAO.ambientVecs[2];
                for(int k = 0;k < 4;k++) {
                    amb[k] = ((float)leftamb_a[k] / 100.0f) * lv;
                }

                // Draw left
                mesh.Color4(amb[3], amb[3], amb[3], 1);  mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, -0);
                mesh.Color4(amb[2], amb[2], amb[2], 1);  mesh.TexCoord2(len, 1);       mesh.Vert3(-0.5, -0.5, len); // br
                mesh.Color4(amb[1], amb[1], amb[1], 1);  mesh.TexCoord2(len, 0);       mesh.Vert3(-0.5, 0.5, len); // tr

                mesh.Color4(amb[1], amb[1], amb[1], 1);  mesh.TexCoord2(len, 0);       mesh.Vert3(-0.5, 0.5, len); // tr
                mesh.Color4(amb[0], amb[0], amb[0], 1);  mesh.TexCoord2(0, 0);         mesh.Vert3(-0.5, 0.5, -0); // tl
                mesh.Color4(amb[3], amb[3], amb[3], 1);  mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, -0);

                lv = fAmbient + (float)GetLightLevel(xindex+1,zindex,y) / (float)MAX_LIGHT_LEVEL;
                lv -= chunk.heatShift;

                uint8_t (&rightamb_a)[4] = curBrickAO.ambientVecs[3];
                for(int k = 0;k < 4;k++) {
                    amb[k] = ((float)rightamb_a[k] / 100.0f * lv);
                }
                
                // Draw right
                mesh.Color4(amb[1], amb[1], amb[1], 1);  mesh.TexCoord2(len, 0);       mesh.Vert3(0.5, 0.5, len); // tr
                mesh.Color4(amb[2], amb[2], amb[2], 1);  mesh.TexCoord2(len, 1);       mesh.Vert3(0.5, -0.5, len); // br
                mesh.Color4(amb[3], amb[3], amb[3], 1);  mesh.TexCoord2(0, 1);         mesh.Vert3(0.5, -0.5, -0);
        
                mesh.Color4(amb[1], amb[1], amb[1], 1);  mesh.TexCoord2(len, 0);       mesh.Vert3(0.5, 0.5, len); // tr   
                mesh.Color4(amb[3], amb[3], amb[3], 1);  mesh.TexCoord2(0, 1);         mesh.Vert3(0.5, -0.5, -0);                             
                mesh.Color4(amb[0], amb[0], amb[0], 1);  mesh.TexCoord2(0, 0);         mesh.Vert3(0.5, 0.5, -0); // tl


                lv = fAmbient + (float)GetLightLevel(xindex,zindex-1,y) / (float)MAX_LIGHT_LEVEL;
                lv -= chunk.heatShift;

                uint8_t (&backamb_a)[4] = curBrickAO.ambientVecs[4];
                for(int k = 0;k < 4;k++) {
                    amb[k] = ((float)backamb_a[k] / 100.0f) * lv;
                }
                                
                // Draw back
                mesh.Color4(amb[3], amb[3], amb[3], 1); mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, -0); // br
                mesh.Color4(amb[0], amb[0], amb[0], 1); mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, -0); // tl
                mesh.Color4(amb[2], amb[2], amb[2], 1); mesh.TexCoord2(1, 1);         mesh.Vert3(0.5, -0.5, -0); // bl

                mesh.Color4(amb[0], amb[0], amb[0], 1); mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, -0);// tl
                mesh.Color4(amb[3], amb[3], amb[3], 1); mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, -0); // br
                mesh.Color4(amb[1], amb[1], amb[1], 1); mesh.TexCoord2(0, 0);         mesh.Vert3(-0.5, 0.5, -0); // tr

                lv = fAmbient + (float)GetLightLevel(xindex,zindex+1,y) / (float)MAX_LIGHT_LEVEL;
                lv -= chunk.heatShift;

                // Draw front
                uint8_t (&frontamb_a)[4] = curBrickAO.ambientVecs[5];
                for(int k = 0;k < 4;k++) {
                    amb[k] = ((float)frontamb_a[k] / 100.0f) * lv;
                }
                            
                mesh.Color4(amb[0], amb[0], amb[0], 1); mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, len); // tl
                mesh.Color4(amb[3], amb[3], amb[3], 1); mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, len); // br
                mesh.Color4(amb[2], amb[2], amb[2], 1); mesh.TexCoord2(1, 1);         mesh.Vert3(0.5, -0.5, len); // bl
                mesh.Color4(amb[0], amb[0], amb[0], 1); mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, len);// tl
                mesh.Color4(amb[1], amb[1], amb[1], 1); mesh.TexCoord2(0, 0);         mesh.Vert3(-0.5, 0.5, len); // tr
                mesh.Color4(amb[3], amb[3], amb[3], 1); mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, len); // br
                cube_count++;
            }
        }
    }
//    mesh.BindBufferData();
    chunk.curStage = chunk_t::UPLOAD_STAGE;
}

void Map::RebuildLights() { }
void Map::RunBuilder() { }

void Map::Draw(Camera &cam) {
    
    int sX = ((int)camera.position.x / CHUNK_SIZE);
    int sZ = ((int)camera.position.z / CHUNK_SIZE);

    if(SDL_GetKeyboardState(0)[SDL_SCANCODE_J]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        fAmbient = 1;
    }
    else if(SDL_GetKeyboardState(0)[SDL_SCANCODE_K]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    for(auto &t: threads) {
        if(t.joinable())
            t.join();
    }


    int curThread = 0;
    std::thread builder;

    int skip = 0;
    int len = 0;
    for(int x = sX - gViewDist;x < sX+gViewDist;x++) {
        for(int z = sZ-gViewDist;z < sZ+gViewDist;z++) {
            if(x < 0 || z < 0) {
                continue;
            }
            len++;

//            std::cout << "drawing " << x << " , " << z << std::endl;

            bool bVisible = false;
            glm::vec3 newCamDir = cam.direction;
            newCamDir = glm::normalize(newCamDir);

            glm::vec3 chunk_edges[8];
            
            chunk_edges[0] = {(x * CHUNK_SIZE), 0.0f, (z * CHUNK_SIZE)};
            chunk_edges[1] = {(x * CHUNK_SIZE) + CHUNK_SIZE, 0.0f, (z * CHUNK_SIZE)};
            chunk_edges[2] = {(x * CHUNK_SIZE) + CHUNK_SIZE, 0.0f, (z * CHUNK_SIZE) + CHUNK_SIZE};
            chunk_edges[3] = {(x * CHUNK_SIZE), 0.0f, (z * CHUNK_SIZE) + CHUNK_SIZE};

            chunk_edges[4] = {(x * CHUNK_SIZE), MAX_HEIGHT, (z * CHUNK_SIZE)};
            chunk_edges[5] = {(x * CHUNK_SIZE) + CHUNK_SIZE, MAX_HEIGHT, (z * CHUNK_SIZE)};
            chunk_edges[6] = {(x * CHUNK_SIZE) + CHUNK_SIZE, MAX_HEIGHT, (z * CHUNK_SIZE) + CHUNK_SIZE};
            chunk_edges[7] = {(x * CHUNK_SIZE), MAX_HEIGHT, (z * CHUNK_SIZE) + CHUNK_SIZE};

            for(float px = (x * CHUNK_SIZE);px < (x * CHUNK_SIZE) + CHUNK_SIZE;px += 2) {
                for(float pz = (z * CHUNK_SIZE);pz < (z * CHUNK_SIZE) + CHUNK_SIZE;pz += 2) {
                    for(float py = 0;py < MAX_HEIGHT;py += 32) {
                        glm::vec3 point(px,py,pz);
                        if(glm::dot(cam.myFrustumPlanes[Camera::PLANE_LEFT].position - point,cam.myFrustumPlanes[Camera::PLANE_LEFT].normal) <= 10
                        && glm::dot(cam.myFrustumPlanes[Camera::PLANE_RIGHT].position - point,cam.myFrustumPlanes[Camera::PLANE_RIGHT].normal) <= 10
                        && glm::dot(cam.myFrustumPlanes[Camera::PLANE_NEAR].position - point,cam.myFrustumPlanes[Camera::PLANE_NEAR].normal)
                        && glm::dot(cam.myFrustumPlanes[Camera::PLANE_TOP].position - point,cam.myFrustumPlanes[Camera::PLANE_TOP].normal) <= 10
                        && glm::dot(cam.myFrustumPlanes[Camera::PLANE_BOTTOM].position - point,cam.myFrustumPlanes[Camera::PLANE_BOTTOM].normal) <= 10) {
                            bVisible = true;
                            goto found;
                        }
                    }
                }
            }
            found:
            auto index = std::make_pair(x,z);
            auto &chunk = Chunks[index];
            chunk.bVisible = bVisible;
            if(!bVisible) {
                skip++;
                continue;
            }
            
            if(chunk.curStage < chunk_t::UPLOAD_STAGE) {
                if(curThread < NUM_THREADS) {
                    threads[curThread] = std::thread([this, &chunk, x, z]() {
                        if(!chunk.bGen) {
                            chunk.Generate(x,z,*this);
                        }
                        else if(!chunk.bInitialAOBuild) {
                            BuildChunkAO(x,z);
                        }
                        else if(!chunk.bIniialBuild) {
                            BuildChunk(x,z);
                        }
                    });
                    curThread++;
                    continue;
                }
                continue;
            }
            if(chunk.curStage != chunk.READY_STAGE) {
                if(chunk.curStage == chunk.UPLOAD_STAGE) {
                    chunk.mesh.BindBufferData();
                    chunk.curStage = chunk.READY_STAGE;
                }
                else {
                    continue;
                }
            }
            Mesh &mesh = chunk.mesh;
            mesh.Draw(Mesh::MODE_TRIANGLES);
        }
    }

    if(SDL_GetKeyboardState(0)[SDL_SCANCODE_T]) {
        RebuildAllVisible();
    }
}

void Map::RebuildAllVisible() {
    int sX = ((int)camera.position.x / CHUNK_SIZE);
    int sZ = ((int)camera.position.z / CHUNK_SIZE);

    for(int x = sX - gViewDist;x < sX+gViewDist;x++) {
        for(int z = sZ-gViewDist;z < sZ+gViewDist;z++) {
            BuildChunkAO(x,z);
            BuildChunk(x,z);
        }
    }
}

void Map::ScheduleMeshBuild(build_schedule_info_t info) {
    // Check if the chunk is already scheduled
    for(size_t i = 0;i < ScheduledBuilds.size();i++) {
        build_schedule_info_t &s = ScheduledBuilds[i];
        if(s.x == info.x && s.z == info.z) {
            if(info.priorityLevel > s.priorityLevel) {
                s.priorityLevel = info.priorityLevel;
            }
            return; // Already scheduled ...
        }
    }
    // Otherwise add it
    ScheduledBuilds.push_back(info);
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

void Map::ScheduleAdjacentChunkBuilds(int startx, int startz, Priority level) {
    for(int x = startx - 1;x < startx + 2;x++) {
        for(int z = startz - 1;z < startz + 2;z++) {
            build_schedule_info_t info;
            info.priorityLevel = level;
            info.x = x;
            info.z = z;

            ScheduleMeshBuild(info);
        }
    }
}

void Map::GenerateChunksFromOrigin(int fromX, int fromZ, int radius) {
    std::vector<std::thread>threadList;
    for(int x = fromX - radius;x < fromX + radius;x++) {
        for(int z = fromZ - radius;z < fromZ + radius;z++) {
            auto &chunk = Chunks[std::make_pair(x,z)];
            threadList.emplace_back(std::thread([this, &chunk, x, z]() {
                chunk.Generate(x,z,*this);
            }));

            if(static_cast<int>(threadList.size()) >= NUM_THREADS) { // Don't spawn too many
                for(auto &t: threadList) {
                    t.join();
                }
                threadList.clear();
            }
        }
    }

    for(auto &t: threadList) { // Finish the threads
        t.join();
    }
}

 