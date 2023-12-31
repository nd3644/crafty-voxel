#include "map.h"

#include "ebmp.h"

#include <iostream>
#include <fstream>
#include "shader.h"
#include "cube.h"
#include "camera.h"
#include "helper.h"
#include "globals.h"
#include <GL/glew.h>
#include <algorithm>

#include <immintrin.h>

#include <stack>

Map::Map(Camera &c) : camera(c) {
    unsigned int num_cores = std::thread::hardware_concurrency();
    if (num_cores == 0) {
        std::cerr << "Unable to determine the number of cores." << std::endl;
        NUM_THREADS = 2;
    } else {
        std::clog << "This PC has " << num_cores << " cores/threads." << std::endl;
        NUM_THREADS = num_cores;
    }

    std::cout << "Map::Map(): size: " << Chunks.size() << std::endl;
}

Map::~Map() {

}

void Map::FillWater(int fx, int fz, int fy) {
/*     const int water = IdFromName("water");

    std::stack<vec3i_t>stack;
    stack.push({ fx, fy, fz });

    const auto RECURSIVE_LIMIT = 2048;

    while(stack.size() > 0) {
        if(stack.size() > RECURSIVE_LIMIT) {
            break;// Init the noise modules
    normalPerlin.SetSeed(321);
    secondPerlin.SetSeed(432);

    normalPerlin.SetFrequency(FREQ);
    secondPerlin.SetFrequency(FREQ);

    normalPerlin.SetOctaveCount(8);
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
    } */
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

void Map::Initialize() {
    BrickTextureFilenames = TextureNamesFromFile("brick_textures.txt");
    myTexArray.Load(BrickTextureFilenames);
    LoadBrickMetaData();

//    RebuildAllVisible();
}

brick_ao_t Map::GetBrickAO(int xindex, int zindex, int y) {
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

    const auto MAX_HEIGHT = chunk_t::MAX_HEIGHT;
    const auto CHUNK_SIZE = chunk_t::CHUNK_SIZE;

/*    std::cout << "building chunk " << "(" << chunkX << " , " << chunkZ << ")" << std::endl;
    std::cout << "size: " << Chunks.size() << std::endl;*/

    int cube_count = 0;

    auto it = Chunks.find(std::make_pair(chunkX,chunkZ));
    if(it == Chunks.end()) {
        return;     // Chunk doesn't exist
    }
    auto &chunk = it->second;
    
    chunk.bIniialBuild = true;

    chunk.mesh.Clean();
    chunk.TransMeshes.clear();
    for (int y = 2; y < MAX_HEIGHT; y++) {                              // Leaving a small margin at the base of the world helps with some batching issues. TODO: Address this.
        for (int x = 0; x < CHUNK_SIZE;x++) {
            int xindex = (chunkX*CHUNK_SIZE)+x;
            for (int z = 0; z < CHUNK_SIZE;z++) {
                int zindex = (chunkZ*CHUNK_SIZE)+z;                     // The actual "brick index coordinate" that applies to our world-view array. e.g. 17 instead of the local index 1

                // The first and most important thing to do to try and find an excuse not to draw the brick at all.
                int brickID = GetBrick(xindex,zindex,y);                // The actual "ID" of the brick
                if (brickID <= 0                                        // Continue if the brick is just empty air
                || IsBrickSurroundedByOpaque(xindex,zindex,y)) {        // If the brick is surrounded by opaque bricks then it is considered occluded and can be skipped
                    continue;
                }

                // We have to draw the brick. Let's collect some more information about the brick we're working with.
                float alpha = GetBrickTransparency(brickID-1);          // This is the conistent transparency level of the brick. e.g. water, glass etc

                if(alpha < 1.0f) {
                    chunk.TransMeshes.emplace_back();
                }

                Mesh &mesh = (alpha == 1.0f) ? chunk.mesh : chunk.TransMeshes.back().first;
                float HEIGHT = 0.5f;                                    // From the centre of the brick, how tall should it be? e.g. water bricks are a little shorter
                int brickLightLevel = GetLightLevel(xindex,zindex,y);   // This is for the lighting engine but is currently always just 1.0. TODO: Implement this.
                float posX = (chunkX*CHUNK_SIZE)+x;                     // This is the floating point "world position" for actual translating of the rendering.
                float posZ = (chunkZ*CHUNK_SIZE)+z;                     // as above, for Z.

                double dist = glm::distance(glm::vec3(posX, y, posZ), glm::vec3(camera.position.x, camera.position.y, camera.position.z)); // Get the average distance from the camera
                if(alpha < 1.0f) {
                    chunk.TransMeshes.back().second = dist;
                }

                mesh.SetTranslation(posX-0.5f,y,posZ-0.5f);             // Apply the translation. Note: -0.5 allow for the bricks to start centred at (0,0).

                brick_ao_t curBrickAO = GetBrickAO(xindex,zindex,y);    // This calculates the AO through the brick_ao_t struct. AO is a little complicated so this struct attempts to simplify things, esp comparison between brick AO values (which are many).

                /* The following routing is a little crude but will determine how many bricks along the Z-axis are of the same type, and also share the same AO.
                   These bricks can be "tiled" with a single polygon and a repeating texture. */
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
                z += len-1;                                             // Skip along the z-axis depending on how wide our polygon will be. This could be anything between 1 (just a block) and 16 (the entire chunk)

                /* BrickLookup stores which brick has which texture for each face. Index1 just tells the vertex shader which index into
                 the texture array to use for a given polygon. This loop will assign the correct information for the shaders. */
                for(int i = 0;i < 6*6;i++)
                    mesh.Index1(BrickLookup[brickID-1][i/6]);

                float lv = fAmbient + (float)GetLightLevel(xindex,zindex,y+1) / (float)MAX_LIGHT_LEVEL;

                float xlen = 1.0;

                if(brickID == BrickNameMap["water"] && GetBrick(xindex,zindex,y+1) <= 0)    // This is a hardcode to make water look a little better. TODO: Remove this and store this information somewhere sensible.
                    HEIGHT -= 0.15f;

                /*
                    NOTE: THE VERTICES ARE LABELD, FR, TR, BL, BR, ETC. THIS WAS DONE AFTER THE FACT FOR CONVENIENCE BUT SADLY, ARE LIKELY NOT CORRECT.
                    TODO: Label these properly.
                */

                // Draw top
                uint8_t (&topamb_a)[4] = curBrickAO.ambientVecs[0];
                float amb[4];
                for(int k = 0;k < 4;k++) {
                    amb[k] = ((float)topamb_a[k] / 100.0f) * lv;
                }
                
                if(amb[0] + amb[3] > amb[1] + amb[2]) {
                    mesh.Color4(amb[2], amb[2], amb[2], alpha); mesh.TexCoord2(0, 0);       mesh.Vert3(0, HEIGHT, 0); // left back 0
                    mesh.Color4(amb[1], amb[1], amb[1], alpha); mesh.TexCoord2(1, len);     mesh.Vert3(xlen, HEIGHT, len);  //  front right 2
                    mesh.Color4(amb[3], amb[3], amb[3], alpha); mesh.TexCoord2(1, 0);       mesh.Vert3(xlen, HEIGHT, 0);   // right back 1 
                    mesh.Color4(amb[1], amb[1], amb[1], alpha); mesh.TexCoord2(1, len);     mesh.Vert3(xlen, HEIGHT, len);  //  front right 2
                    mesh.Color4(amb[2], amb[2], amb[2], alpha); mesh.TexCoord2(0, 0);       mesh.Vert3(0, HEIGHT, 0); // left back 0
                    mesh.Color4(amb[0], amb[0], amb[0], alpha); mesh.TexCoord2(0, len);     mesh.Vert3(0, HEIGHT, len);  // front left 3
                }
                else {
                    mesh.Color4(amb[0], amb[0], amb[0], alpha); mesh.TexCoord2(0, len);     mesh.Vert3(0, HEIGHT, len); // front left
                    mesh.Color4(amb[1], amb[1], amb[1], alpha); mesh.TexCoord2(1, len);     mesh.Vert3(xlen, HEIGHT, len);  //  front right
                    mesh.Color4(amb[3], amb[3], amb[3], alpha); mesh.TexCoord2(1, 0);       mesh.Vert3(xlen, HEIGHT, -0); // right back
                    mesh.Color4(amb[0], amb[0], amb[0], alpha); mesh.TexCoord2(0, len);     mesh.Vert3(0, HEIGHT, len);  // front left
                    mesh.Color4(amb[3], amb[3], amb[3], alpha); mesh.TexCoord2(1, 0);       mesh.Vert3(xlen, HEIGHT, 0);   // right back
                    mesh.Color4(amb[2], amb[2], amb[2], alpha); mesh.TexCoord2(0, 0);       mesh.Vert3(0, HEIGHT, -0); // left back
                }

                lv = fAmbient + (float)GetLightLevel(xindex,zindex,y-1) / (float)MAX_LIGHT_LEVEL;

                uint8_t (&botamb_a)[4] = curBrickAO.ambientVecs[1];
                for(int k = 0;k < 4;k++) {
                    amb[k] = ((float)botamb_a[k] / 100.0f) * lv;
                }

                // TODO: add a/o here
                // Draw bottom
                mesh.Color4(lv, lv, lv, alpha); mesh.Color4(lv, lv, lv, alpha); mesh.Color4(lv, lv, lv, alpha); mesh.Color4(lv, lv, lv, alpha);mesh.Color4(lv, lv, lv, alpha); mesh.Color4(lv, lv, lv, alpha);
                mesh.TexCoord2(0, len);       mesh.Vert3(0, -0.5, len);
                mesh.TexCoord2(1, 0);         mesh.Vert3(xlen, -0.5, -0);
                mesh.TexCoord2(1, len);       mesh.Vert3(xlen, -0.5, len);

                mesh.TexCoord2(0, len);       mesh.Vert3(0, -0.5, len);
                mesh.TexCoord2(0, 0);         mesh.Vert3(0, -0.5, -0);
                mesh.TexCoord2(1, 0);         mesh.Vert3(xlen, -0.5, -0);

                lv = fAmbient + (float)GetLightLevel(xindex-1,zindex,y) / (float)MAX_LIGHT_LEVEL;

                uint8_t (&leftamb_a)[4] = curBrickAO.ambientVecs[2];
                for(int k = 0;k < 4;k++) {
                    amb[k] = ((float)leftamb_a[k] / 100.0f) * lv;
                }

                // Draw left
                mesh.Color4(amb[3], amb[3], amb[3], alpha);  mesh.TexCoord2(0, 1);         mesh.Vert3(0, -0.5, -0);
                mesh.Color4(amb[2], amb[2], amb[2], alpha);  mesh.TexCoord2(len, 1);       mesh.Vert3(0, -0.5, len); // br
                mesh.Color4(amb[1], amb[1], amb[1], alpha);  mesh.TexCoord2(len, 0);       mesh.Vert3(0, HEIGHT, len); // tr

                mesh.Color4(amb[1], amb[1], amb[1], alpha);  mesh.TexCoord2(len, 0);       mesh.Vert3(0, HEIGHT, len); // tr
                mesh.Color4(amb[0], amb[0], amb[0], alpha);  mesh.TexCoord2(0, 0);         mesh.Vert3(0, HEIGHT, -0); // tl
                mesh.Color4(amb[3], amb[3], amb[3], alpha);  mesh.TexCoord2(0, 1);         mesh.Vert3(0, -0.5, -0);

                lv = fAmbient + (float)GetLightLevel(xindex+1,zindex,y) / (float)MAX_LIGHT_LEVEL;

                uint8_t (&rightamb_a)[4] = curBrickAO.ambientVecs[3];
                for(int k = 0;k < 4;k++) {
                    amb[k] = ((float)rightamb_a[k] / 100.0f * lv);
                }
                
                // Draw right
                mesh.Color4(amb[1], amb[1], amb[1], alpha);  mesh.TexCoord2(len, 0);       mesh.Vert3(xlen, HEIGHT, len); // tr
                mesh.Color4(amb[2], amb[2], amb[2], alpha);  mesh.TexCoord2(len, 1);       mesh.Vert3(xlen, -0.5, len); // br
                mesh.Color4(amb[3], amb[3], amb[3], alpha);  mesh.TexCoord2(0, 1);         mesh.Vert3(xlen, -0.5, -0);
        
                mesh.Color4(amb[1], amb[1], amb[1], alpha);  mesh.TexCoord2(len, 0);       mesh.Vert3(xlen, HEIGHT, len); // tr   
                mesh.Color4(amb[3], amb[3], amb[3], alpha);  mesh.TexCoord2(0, 1);         mesh.Vert3(xlen, -0.5, -0);                             
                mesh.Color4(amb[0], amb[0], amb[0], alpha);  mesh.TexCoord2(0, 0);         mesh.Vert3(xlen, HEIGHT, -0); // tl

                lv = fAmbient + (float)GetLightLevel(xindex,zindex-1,y) / (float)MAX_LIGHT_LEVEL;

                uint8_t (&backamb_a)[4] = curBrickAO.ambientVecs[4];
                for(int k = 0;k < 4;k++) {
                    amb[k] = ((float)backamb_a[k] / 100.0f) * lv;
                }
                                
                // Draw back
                mesh.Color4(amb[3], amb[3], amb[3], alpha); mesh.TexCoord2(0, 1);         mesh.Vert3(0, -0.5, -0); // br
                mesh.Color4(amb[0], amb[0], amb[0], alpha); mesh.TexCoord2(1, 0);         mesh.Vert3(xlen, HEIGHT, -0); // tl
                mesh.Color4(amb[2], amb[2], amb[2], alpha); mesh.TexCoord2(1, 1);         mesh.Vert3(xlen, -0.5, -0); // bl

                mesh.Color4(amb[0], amb[0], amb[0], alpha); mesh.TexCoord2(1, 0);         mesh.Vert3(xlen, HEIGHT, -0);// tl
                mesh.Color4(amb[3], amb[3], amb[3], alpha); mesh.TexCoord2(0, 1);         mesh.Vert3(0, -0.5, -0); // br
                mesh.Color4(amb[1], amb[1], amb[1], alpha); mesh.TexCoord2(0, 0);         mesh.Vert3(0, HEIGHT, -0); // tr

                lv = fAmbient + (float)GetLightLevel(xindex,zindex+1,y) / (float)MAX_LIGHT_LEVEL;

                // Draw front
                uint8_t (&frontamb_a)[4] = curBrickAO.ambientVecs[5];
                for(int k = 0;k < 4;k++) {
                    amb[k] = ((float)frontamb_a[k] / 100.0f) * lv;
                }
                            
                mesh.Color4(amb[0], amb[0], amb[0], alpha); mesh.TexCoord2(1, 0);         mesh.Vert3(xlen, HEIGHT, len); // tl
                mesh.Color4(amb[3], amb[3], amb[3], alpha); mesh.TexCoord2(0, 1);         mesh.Vert3(0, -0.5, len); // br
                mesh.Color4(amb[2], amb[2], amb[2], alpha); mesh.TexCoord2(1, 1);         mesh.Vert3(xlen, -0.5, len); // bl
                mesh.Color4(amb[0], amb[0], amb[0], alpha); mesh.TexCoord2(1, 0);         mesh.Vert3(xlen, HEIGHT, len);// tl
                mesh.Color4(amb[1], amb[1], amb[1], alpha); mesh.TexCoord2(0, 0);         mesh.Vert3(0, HEIGHT, len); // tr
                mesh.Color4(amb[3], amb[3], amb[3], alpha); mesh.TexCoord2(0, 1);         mesh.Vert3(0, -0.5, len); // br
                cube_count++;
            }
        }
    }
//    mesh.BindBufferData();`
    chunk.curStage = chunk_t::UPLOAD_STAGE;
    chunk.iRebuildCounter++;
    if(chunk.iRebuildCounter > 5) // TODO: Review whatever this is. I don't actually remember?
        chunk.iRebuildCounter = 0;
    
    chunk.bRequiresRebuild = false;
}

void Map::MarkChunkVisibilities(Camera &cam) {
    gFrustumSkips = 0; // Reset counter

    bool bVisible = false;
    glm::vec3 newCamDir = cam.direction;
    newCamDir = glm::normalize(newCamDir);

    glm::vec3 chunk_edges[8];

    auto MAX_HEIGHT = chunk_t::MAX_HEIGHT;
    auto CHUNK_SIZE = chunk_t::CHUNK_SIZE;
    
    int sX = ((int)camera.position.x / chunk_t::CHUNK_SIZE);
    int sZ = ((int)camera.position.z / chunk_t::CHUNK_SIZE);

    // Loop through all chunks within the gViewDist
    for(int x = sX - gViewDist;x < sX+gViewDist;x++) {
        if(x < 0)
            continue;
        for(int z = sZ-gViewDist;z < sZ+gViewDist;z++) {
            if(z < 0)
                continue;
            
            // Define the bounding box for the chunk
            chunk_edges[0] = {(x * CHUNK_SIZE), 0.0f, (z * CHUNK_SIZE)};
            chunk_edges[1] = {(x * CHUNK_SIZE) + CHUNK_SIZE, 0.0f, (z * CHUNK_SIZE)};
            chunk_edges[2] = {(x * CHUNK_SIZE) + CHUNK_SIZE, 0.0f, (z * CHUNK_SIZE) + CHUNK_SIZE};
            chunk_edges[3] = {(x * CHUNK_SIZE), 0.0f, (z * CHUNK_SIZE) + CHUNK_SIZE};

            chunk_edges[4] = {(x * CHUNK_SIZE), MAX_HEIGHT, (z * CHUNK_SIZE)};
            chunk_edges[5] = {(x * CHUNK_SIZE) + CHUNK_SIZE, MAX_HEIGHT, (z * CHUNK_SIZE)};
            chunk_edges[6] = {(x * CHUNK_SIZE) + CHUNK_SIZE, MAX_HEIGHT, (z * CHUNK_SIZE) + CHUNK_SIZE};
            chunk_edges[7] = {(x * CHUNK_SIZE), MAX_HEIGHT, (z * CHUNK_SIZE) + CHUNK_SIZE};

            bVisible = false;

            int timerResult = 0;
            auto startTime = std::chrono::high_resolution_clock::now();
            for(int i = 0;i < 4;i++) {
                for(int j = 4;j < 8;j++) {
                    if(cam.DoesLineIntersectFrustum(chunk_edges[i],chunk_edges[j],0,4)) {
                        bVisible = true;
                        goto found;
                    }
                }
            }
            found:
            
            auto endTime = std::chrono::high_resolution_clock::now();
            timerResult += std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();

            auto index = std::make_pair(x,z);
            auto &chunk = Chunks[index];
            chunk.bVisible = bVisible;

            if(!chunk.bVisible) {
                gFrustumSkips++;
                continue;
            }
        }
    }
}

void Map::Draw(Camera &cam) {
    int sX = ((int)camera.position.x / chunk_t::CHUNK_SIZE);
    int sZ = ((int)camera.position.z / chunk_t::CHUNK_SIZE);

    if(gRenderMode == GlobalRenderModes::RENDER_MODE_DEFAULT) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        fAmbient = 1;
    }
    else if(gRenderMode == GlobalRenderModes::RENDER_MODE_WIRES) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Wait for threads from previous frame
    for(auto &t: threads) {
        if(t.joinable())
            t.join();
    }

    int curThread = 0;
    std::thread builder;

    int drawCount = 0;
    MarkChunkVisibilities(cam);

    // First run the generation pass
    for(int x = sX - gViewDist;x < sX+gViewDist;x++) {
        if(x < 0)
            continue;
        for(int z = sZ-gViewDist;z < sZ+gViewDist;z++) {
            if(z < 0)
                continue;

            auto index = std::make_pair(x,z);
            auto &chunk = Chunks[index];

            // Note that it's better not to discriminate on visibility here.
            // Generating the terrain should be done pretty freely as this information
            // is invaluable for rendering later
            if(chunk.curStage == chunk_t::DEFAULT_STAGE) {
                if(curThread < NUM_THREADS) {
                    threads[curThread] = std::thread([this, &chunk, x, z]() {
                        if(!chunk.bGen) {
                            chunk.Generate(x,z,*this);
                        }
                    });
                    curThread++;
                }
                continue;
            }
        }
    }

    // First, loop through all chunks within the gViewDist
    for(int x = sX - gViewDist;x < sX+gViewDist;x++) {
        if(x < 0)
            continue;
        for(int z = sZ-gViewDist;z < sZ+gViewDist;z++) {
            if(z < 0)
                continue;

            auto index = std::make_pair(x,z);
            auto &chunk = Chunks[index];
            
            if(!chunk.bVisible) {
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
            drawCount++;
        }
    }


    // Then, run the builder pass
    if(curThread == 0) {
        for(int x = sX - gViewDist;x < sX+gViewDist;x++) {
            if(x < 0)
                continue;
            for(int z = sZ-gViewDist;z < sZ+gViewDist;z++) {
                if(z < 0)
                    continue;

                auto index = std::make_pair(x,z);
                auto &chunk = Chunks[index];

                if(!chunk.bVisible) {
                    continue;
                }

                if(chunk.curStage == chunk_t::BUILD_STAGE) {

                    bool canbuild = true;
                    for(int lx = -2;lx < 2;lx++) {
                        for(int lz = -2;lz < 2;lz++) {
                            if(lx == 0 && lz == 0)
                                continue;
                            chunk_t &chunk = Chunks[std::make_pair(x+lx,z+lz)];
                            if(chunk.bGen == false){
                                lx = 2;
                                canbuild = false;
                                break;
                            }
                        }
                    }

                    if(canbuild) {
                        if(curThread < NUM_THREADS) {
                            threads[curThread] = std::thread([this, &chunk, x, z]() {
                                BuildChunk(x,z);
//                                std::cout << "BUILDING " << x << " , " << z << std::endl;
                            });
                            curThread++;
                        }
                        continue;
                    }
                }
            }
        }
    }

    if(SDL_GetKeyboardState(0)[SDL_SCANCODE_T]) {
        RebuildAllVisible();
    }

    
    //std::cout << "count: " << drawCount << std::endl;

    // (debug) Print out the total time spent on binary search
    //std::cout << timerRes << std::endl;
}

void Map::RebuildAllVisible() {
    int sX = ((int)camera.position.x / chunk_t::CHUNK_SIZE);
    int sZ = ((int)camera.position.z / chunk_t::CHUNK_SIZE);

    for(int x = sX - gViewDist;x < sX+gViewDist;x++) {
        for(int z = sZ-gViewDist;z < sZ+gViewDist;z++) {
            BuildChunk(x,z);
        }
    }
}

void Map::LoadBrickMetaData() {
    std::ifstream infile("bricks.txt");
    if(!infile.is_open()) {
        std::cout << "ERROR: Map::LoadBrickMetaDatA(): couldn't open bricks.txt" << std::endl;
        exit(1);
    }
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
        std::cout << "brick " << brickName << " has transparency level of " << transparency << std::endl;
        BrickTransparencies.push_back(transparency);
        BrickLookup.push_back(arr);
    }
}

std::vector<std::array<int,6>> Map::GetLookupArr() const {
    return BrickLookup;
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

 
int Map::Map::GetBrick(int x, int z, int y) {
    int xchunk = x / chunk_t::CHUNK_SIZE;
    int zchunk = z / chunk_t::CHUNK_SIZE;

    int xindex = x % chunk_t::CHUNK_SIZE;
    int zindex = z % chunk_t::CHUNK_SIZE;

    int val = 255;
    auto key = std::make_pair(xchunk,zchunk);
    auto it = Chunks.find(key);
    if(it != Chunks.end()) {
        auto &chunk = it->second;
        val = chunk.iBricks[xindex][y][zindex];
    }

    return val;
}

void Map::Map::SetBrick(int x, int z, int y, int id) {
    using namespace std;

    int xchunk = x / chunk_t::CHUNK_SIZE;
    int zchunk = z / chunk_t::CHUNK_SIZE;

    int xindex = x % chunk_t::CHUNK_SIZE;
    int zindex = z % chunk_t::CHUNK_SIZE;

    auto key = std::make_pair(xchunk,zchunk);
    auto it = Chunks.find(key);
    if(it != Chunks.end()) {
        auto &chunk = it->second;
        chunk.iBricks[xindex][y][zindex] = id;
    }
}

chunk_t *Map::GetChunk(int x, int z) {
    x *= chunk_t::CHUNK_SIZE;
    z *= chunk_t::CHUNK_SIZE;

    int xchunk = x / chunk_t::CHUNK_SIZE;
    int zchunk = z / chunk_t::CHUNK_SIZE;

    auto chunk_index = std::make_pair(xchunk,zchunk);

    return &Chunks[chunk_index];
}

float Map::GetBrickTransparency(int id) const {
    if(id < 0 || id >= static_cast<int>(BrickTransparencies.size()))
        return 1;
    return BrickTransparencies[id];
}

bool Map::IsBrickSurroundedByOpaque(int x, int z, int y) {
    // These are checked in a funny order because they are in order of most-to-least likely
    if(GetBrick(x,z,y+1) <= 0 
    || GetBrick(x-1,z,y) <= 0
    || GetBrick(x+1,z,y) <= 0
    || GetBrick(x,z-1,y) <= 0
    || GetBrick(x,z+1,y) <= 0
    || GetBrick(x,z,y-1) <= 0){
        return false;
    }
    return true;
}

int Map::IdFromName(std::string str) {
    return BrickNameMap[str];
}

std::vector<std::string> Map::GetTextureFilenames() {
    return BrickTextureFilenames;
}

double Map::GetContinentalness(float x, float z) const {
    noise::module::Perlin module;
    chunk_t::ConfigureContinentalness(module);
    return module.GetValue(x,z, 0.5);
}
double Map::GetErosion(float x, float z) const {
    noise::module::Perlin module;
    chunk_t::ConfigureErosion(module);
    return module.GetValue(x,z, 0.5);
}
