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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <libnoise/noise.h>
#include <gsl/gsl_spline.h>

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
        NUM_THREADS = num_cores/2;
    }

    viewDist = 16;
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

const int numPoints = 5;
double ax[numPoints] = { -1, -0.8, 0.3, 0.5, 1.0 };
double ay[numPoints] = { 50, 30, 100, 100, 150 };

double interpolateY(double x)
{
    if(x < -1)
        x = -1;
    if(x > 1)
        x = 1;
    
    gsl_interp_accel* accel = gsl_interp_accel_alloc();
    gsl_interp* interp = gsl_interp_alloc(gsl_interp_linear, numPoints);

    gsl_interp_init(interp, ax, ay, numPoints);
    double interpolatedY = gsl_interp_eval(interp, ax, ay, x, accel);

    gsl_interp_free(interp);
    gsl_interp_accel_free(accel);

    return interpolatedY;
}

void Map::chunk_t::Generate(int chunkx, int chunkz, Map &map) {
    if(bGen || bIsCurrentlyGenerating)
        return;

    const int SEA_LEVEL = 63;

    bIsCurrentlyGenerating = true;
    bool bMount = (rand()%5 == 1) ? true : false;

    int brickType = 1;
    if(bMount) {
        brickType = 1;
    }

    std::cout << "generating " << chunkx << " , " << chunkz << std::endl;

    static int counter = 0;

    using namespace noise;

    module::Perlin normalPerlin;
    normalPerlin.SetFrequency(0.2);

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
            float unit = normalPerlin.GetValue((float)xindex / 100.0f,(float)zindex / 100.0f,0.5);
            int height = 0;//(((unit + 1) / 2) * MAX_HEIGHT);

            double xValue = unit;
//            double yValue = gsl_spline_eval(spline, xValue, accel);
//            height = yValue;
            height = interpolateY(xValue);

            if(height >= MAX_HEIGHT)
                height = MAX_HEIGHT - 1;
            
            for (int y = height; y > 0; y--) {
				map.SetBrick(xindex, zindex, y, brickType);
                if(y > 96)
                    map.SetBrick(xindex,zindex,y,map.IdFromName("otherstone"));
			}

            for(int y = 1;y < SEA_LEVEL + 5;y++) {
                if(map.GetBrick(xindex,zindex,y) == 1) {
                    map.SetBrick(xindex,zindex,y,map.IdFromName("sand"));
                }
            }
            for(int y = 1;y < SEA_LEVEL;y++) {
                if(map.GetBrick(xindex,zindex,y) == 0) {
                    map.SetBrick(xindex,zindex,y,map.IdFromName("water"));
                }
            }
		}
	}


    // Plant some trees

    if(rand() % 6 == 0) {
        int lX = rand() % 16;
        int lZ = rand() % 16;
        lX += chunkx * CHUNK_SIZE;
        lZ += chunkz * CHUNK_SIZE;
        int height = (rand() % 6) + 5;

        bool bCanPlant = false;

        int groundLvl = 1;
        for(groundLvl = 1;groundLvl < MAX_HEIGHT;groundLvl++) {
            if(map.GetBrick(lX,lZ,groundLvl) == 0) {
                if(map.GetBrick(lX,lZ,groundLvl-1) == map.IdFromName("grass_top")) { // Only grow ontop of grass
                    bCanPlant = true;
                }
                break;
            }
        }

        if(bCanPlant) {
            for(int y = 0;y < groundLvl+height;y++) {
                map.SetBrick(lX,lZ,y,map.IdFromName("tree"));
            }

            for(int y = 0;y < 4;y++) {
                int length = 4 - y;
                for(int x = lX - length;x <= lX + length;x++) {
                    for(int z = lZ - length;z <= lZ + length;z++) {
                        map.SetBrick(x,z,(groundLvl+height)+y,map.IdFromName("leaves"));
                    }
                }
            }
            for(int x = lX - 3;x <= lX + 3;x++) {
                for(int z = lZ - 3;z <= lZ + 3;z++) {
                    map.SetBrick(x,z,(groundLvl+height)-1,map.IdFromName("leaves"));
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

    width = myBmp.GetWidth();
    depth = myBmp.GetHeight();

    BrickTextureFilenames = TextureNamesFromFile("brick_textures.txt");
    myTexArray.Load(BrickTextureFilenames);
    LoadBrickMetaData();


    for(int x = -viewDist;x < viewDist;x++) {
        for(int z = -viewDist;z < viewDist;z++) {
            Chunks[std::make_pair(x,z)].Generate(x,z,*this);
        }
    }

    RebuildAllVisible();
}

void Map::RebuildAll() {
    // Build chunks
    for(int x = 0;x < width/CHUNK_SIZE;x++) {
        for(int z = 0;z < depth/CHUNK_SIZE;z++) {
            //BuildChunk(x,z);
        }
    }
}

void Map::BuildChunkAO(int chunkX, int chunkZ) {
    chunk_t &chunk = Chunks[std::make_pair(chunkX,chunkZ)];
    for (int y = 0; y < MAX_HEIGHT; y++) {
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

                uint8_t ambShade = 85;


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
    }
}

void Map::BuildChunk(int chunkX, int chunkZ) {
    //std::cout << "building chunk " << "(" << chunkX << " , " << chunkZ << ")" << std::endl;

//    std::cout << "building " << chunkX << ", " << chunkZ << std::endl;

//    BuildChunkAO(chunkX,chunkZ);

    int cube_count = 0;
    chunk_t &chunk = Chunks[std::make_pair(chunkX,chunkZ)];
    Mesh &mesh = chunk.mesh;

    chunk.bIniialBuild = true;

    mesh.Clean();
    for (int y = 0; y < MAX_HEIGHT; y++) {
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

                int len = 1;
                for(int i = 1;i < CHUNK_SIZE;i++) {
                    if(z < CHUNK_SIZE-i
                    && brickID == GetBrick(xindex,zindex+i,y)
                    && brickLightLevel == GetLightLevel(xindex,zindex+i,y)) {
                        for(int f = 0;f < 6;f++) {
                            for(int v = 0;v < 4;v++) {
                                if(chunk.ambientVecs[x][y][z+i][f][v] != chunk.ambientVecs[x][y][z][f][v]) {
                                    goto skip;  // This is just a crude way to break out of the loops
                                }
                            }
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
                // Draw top

                
                uint8_t (&topamb_a)[4] = chunk.ambientVecs[x][y][z][0];
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


                uint8_t (&botamb_a)[4] = chunk.ambientVecs[x][y][z][1];
                for(int k = 0;k < 4;k++) {
                    amb[k] = (float)botamb_a[k] / 100.0f;
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

                uint8_t (&leftamb_a)[4] = chunk.ambientVecs[x][y][z][2];
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

                uint8_t (&rightamb_a)[4] = chunk.ambientVecs[x][y][z][3];
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

                uint8_t (&backamb_a)[4] = chunk.ambientVecs[x][y][z][4];
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

                // Draw front
                uint8_t (&frontamb_a)[4] = chunk.ambientVecs[x][y][z][5];
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
    mesh.BindBufferData();

    // Transparent pass
//    BuildChunkTrans(chunkX,chunkZ);

//    std::cout << "built " << cube_count << std::endl;

/*    std::string str = std::to_string(dbgIterCount);
    if(str.length() > 3) {
        str.insert(str.begin()+3, ',');
    }
    std::cout << "iter_count: " << str << std::endl;*/
}

void Map::BuildChunkTrans(int chunkX, int chunkZ) {
    return;
    chunk_t &chunk = Chunks[std::make_pair(chunkX,chunkZ)];
    Mesh &mesh = chunk.mesh;

    chunk.bIniialBuild = true;

    mesh.Clean();
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
}

void Map::RebuildLights() {

}

void Map::Draw() {
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
    for(int x = sX - viewDist;x < sX+viewDist;x++) {
        for(int z = sZ-viewDist;z < sZ+viewDist;z++) {
            auto index = std::make_pair(x,z);
            auto &chunk = Chunks[index];
            if(chunk.bIniialBuild == false || chunk.mesh.IsEmpty() || chunk.bGen == false) {
                continue;
            }
            Mesh &mesh = chunk.mesh;
            mesh.Draw(Mesh::MODE_TRIANGLES);
        }
    }

/*    for(int x = sX - viewDist;x < sX+viewDist;x++) {
        for(int z = sZ-viewDist;z < sZ+viewDist;z++) {
            auto index = std::make_pair(x,z);
            auto &chunk = Chunks[index];
            if(chunk.bIniialBuild == false || chunk.mesh.IsEmpty()) {
                continue;
            }
            chunk.transMesh.Draw(Mesh::MODE_TRIANGLES);
        }
    }
*/

    if(SDL_GetKeyboardState(0)[SDL_SCANCODE_T]) {
        RebuildAllVisible();
    }
}

void Map::RebuildAllVisible() {
    int sX = ((int)camera.position.x / CHUNK_SIZE);
    int sZ = ((int)camera.position.z / CHUNK_SIZE);

    for(int x = sX - viewDist;x < sX+viewDist;x++) {
        for(int z = sZ-viewDist;z < sZ+viewDist;z++) {
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

void Map::RunBuilder() {
    int sX = ((int)camera.position.x / CHUNK_SIZE);
    int sZ = ((int)camera.position.z / CHUNK_SIZE);

    int build_count = 0;

    int endX = sX+viewDist;
    int endZ = sZ+viewDist;
/*    // First ensure every chunk within viewdist is both generated and built
    for(int x = sX - viewDist;x < sX+viewDist;x++) {
        for(int z = sZ-viewDist;z < endZ;z++) {
            auto index = std::make_pair(x,z);
            auto &chunk = Chunks[index];

            if(!chunk.bGen) {
                chunk.Generate(x,z,*this);
                goto exitLoop;
            }

            if(chunk.bIniialBuild == false || chunk.mesh.IsEmpty()) {
                //if(build_count < 1) {
                    BuildChunk(x,z);
                    build_count++;
                    goto exitLoop;
                //}
            }
        }
    }
    exitLoop:
````

    // Render one of the next chunks following the highest priority first
    for(int priority = Priority::ONE;priority < Priority::NUM_PRIORITIES;priority++) {
        for(size_t i = 0;i < ScheduledBuilds.size();i++) {
            if(ScheduledBuilds[i].priorityLevel == priority) {
                BuildChunk(ScheduledBuilds[i].x, ScheduledBuilds[i].z);
                ScheduledBuilds.erase(ScheduledBuilds.begin()+i);
                build_count++;

                // break out of the loop
                priority = Priority::NUM_PRIORITIES;
                break;
            }
        }
    }*/

    // Use the time to build a random one
    // TODO: Actually make a system for this
    int minX = sX - viewDist;
    int maxX = sX + viewDist;

    int minZ = sZ - viewDist;
    int maxZ = sZ + viewDist;

    int randX = (rand() % (maxX - minX) + 1) + minX;
    int randZ = (rand() % (maxZ - minZ) + 1) + minZ;

    static int rebuildX = randX;
    static int rebuildZ = randZ;

    chunk_t &rndChunk = *GetChunk(rebuildX, rebuildZ);
    if(rndChunk.pipleline_stage == 0) {
        BuildChunkAO(floor(camera.position.x / CHUNK_SIZE), floor(camera.position.z / CHUNK_SIZE));
        rndChunk.pipleline_stage++;
    }
    else {
        BuildChunk(floor(camera.position.x / CHUNK_SIZE), floor(camera.position.z / CHUNK_SIZE));
        rndChunk.pipleline_stage = 0;

        rebuildX = randX;
        rebuildZ = randZ;
    }

   static bool b = false;

   // Ensure immediate priority chunks are built on the same frame
    for(size_t i = 0;i < ScheduledBuilds.size();i++) {
        if(ScheduledBuilds[i].priorityLevel == Priority::IMMEDIATE) {

            auto &chunk = *GetChunk(ScheduledBuilds[i].x, ScheduledBuilds[i].z);
            if(chunk.pipleline_stage == 0) {
                BuildChunkAO(ScheduledBuilds[i].x, ScheduledBuilds[i].z);
                chunk.pipleline_stage++;
            }
            else {
                BuildChunk(ScheduledBuilds[i].x, ScheduledBuilds[i].z);
                chunk.pipleline_stage = 0;
                ScheduledBuilds.erase(ScheduledBuilds.begin()+i);
            }
            build_count++;
        }
    }
    b = !b;

//    std::cout << "built: " << build_count << std::endl;
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

