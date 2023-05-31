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

#include <thread>

#include <libnoise/noise.h>

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
    fAmbient = 0.8f;
}

Map::~Map() {

}

void Map::chunk_t::Generate(int chunkx, int chunkz, Map &map) {
    if(bGen)
        return;

    std::cout << "generating " << chunkx << " , " << chunkz << std::endl;
    noise::module::Perlin myModule; 
    myModule.SetSeed(123);
    myModule.SetFrequency(0.02);
    myModule.SetPersistence(0.1);
    myModule.SetOctaveCount(3);

    for (int x = 0; x < CHUNK_SIZE;x++) {
		for (int z = 0; z < CHUNK_SIZE; z++) {
            int height = ((myModule.GetValue((chunkx*CHUNK_SIZE)+x,(chunkz*CHUNK_SIZE)+z,0.5) + 1) / 2) * 32;
            if(height < 4)
                height = 4;
            for (int y = 0; y < height/4; y++) {
				map.SetBrick((chunkx*CHUNK_SIZE)+x, (chunkz*CHUNK_SIZE)+z, y, 1);
			}
		}
	}
    bGen = true;
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


/*    for(int x = -16;x < 16;x++) {
        for(int z = -16;z < 16;z++) {
            Chunks[std::make_pair(x,z)].Generate(x, z, *this);
        }
    }
*/
/*	for (int x = 0; x < width*2;x++) {
		for (int z = 0; z < depth*2; z++) {
			//int height = myBmp.GetPixelRGBA(x%width, z%depth).R;
            int height = ((myModule.GetValue((float)x,(float)z,0.5) + 1) / 2) * 32;
            if(height < 4)
                height = 4;
            for (int y = 0; y < height/4; y++) {
				SetBrick(x, z, y, 1);
                
                // Set the mirror brick for negative X
                SetBrick(-x, z, y, 1);

                // Set the mirror brick for negative Z
                SetBrick(x, -z, y, 1);

                // Set the mirror brick for both negative X and Z
                SetBrick(-x, -z, y, 1);
			}
		}
	}
*/

    std::vector<std::string>images = { "textures/grass.png", "textures/grass_side.png","textures/dirt.png","textures/torch.png" };
    myTexArray.Load(images);
    LoadBrickMetaData();
    ProcessMap_Simple();

}

void Map::RebuildAll() {
    // Build chunks
    for(int x = 0;x < width/CHUNK_SIZE;x++) {
        for(int z = 0;z < depth/CHUNK_SIZE;z++) {
            BuildChunk(x,z);
        }
    }
}

void Map::BuildChunk(int chunkX, int chunkZ) {
    Mesh &mesh = Chunks[std::make_pair(chunkX,chunkZ)].mesh;

    if(!Chunks[std::make_pair(chunkX,chunkZ)].bGen) {
        Chunks[std::make_pair(chunkX,chunkZ)].Generate(chunkX, chunkZ, *this);
    }

    mesh.Clean();

    int skipped = 0;
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < CHUNK_SIZE;x++) {
            int xindex = (chunkX*CHUNK_SIZE)+x;
            for (int z = 0; z < CHUNK_SIZE;z++) {
                int zindex = (chunkZ*CHUNK_SIZE)+z;
                if (GetBrick(xindex, zindex, y) <= 0) {
                    continue;
                }
                if(GetBrick(xindex-1,zindex,y) != 0
                && GetBrick(xindex+1,zindex,y) != 0
                && GetBrick(xindex,zindex-1,y) != 0
                && GetBrick(xindex,zindex+1,y) != 0
                && GetBrick(xindex,zindex,y+1) != 0
                && GetBrick(xindex,zindex,y-1) != 0) {
                    skipped++;
                    continue;
                }
                
                float posX = (chunkX*CHUNK_SIZE)+x;
                float posZ = (chunkZ*CHUNK_SIZE)+z;
                mesh.SetTranslation(posX,y,posZ);
                float c = 1.0f;
                int brickID = GetBrick(xindex,zindex,y);

                int len = 1;

                for(int i = 1;i < CHUNK_SIZE-1;i++) {
                    if(z < CHUNK_SIZE-1-i
                    && GetBrick(xindex,zindex,y) == GetBrick(xindex,zindex+i,y)
                    ) { //&& GetLightLvl(xindex,zindex,y) == GetLightLvl(xindex,zindex+i,y)) {
                        len++;
                    }
                    else {
                        break;
                    }
                }
                if(zindex==0||zindex==CHUNK_SIZE-1){
                    len=1;
                }

                z += len-1;

                for(int i = 0;i < 6*6;i++)
                    mesh.Index1(BrickLookup[brickID-1][i/6]);

                float lv = 1;
                if(lv > 1)
                    lv = 1;
                if(lv < fAmbient)
                    lv = fAmbient;

                // Draw top
                mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1);mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1);
                mesh.TexCoord2(0, len);     mesh.Vert3(-0.5, 0.5, len);
                mesh.TexCoord2(1, len);     mesh.Vert3(0.5, 0.5, len);
                mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, -0);

                mesh.TexCoord2(0, len);     mesh.Vert3(-0.5, 0.5, len);
                mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, 0);
                mesh.TexCoord2(0, 0);         mesh.Vert3(-0.5, 0.5, -0);

                // Draw bottom
                mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1);mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1);
                mesh.TexCoord2(0, len);     mesh.Vert3(-0.5, -0.5, len);
                mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, -0.5, -0);
                mesh.TexCoord2(1, len);     mesh.Vert3(0.5, -0.5, len);

                mesh.TexCoord2(0, len);     mesh.Vert3(-0.5, -0.5, len);
                mesh.TexCoord2(0, 0);         mesh.Vert3(-0.5, -0.5, -0);
                mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, -0.5, -0);

                // Draw left
                mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1);mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1);
                mesh.TexCoord2(0, 1);         mesh.Vert3(0.5, -0.5, 0);
                mesh.TexCoord2(len, 0);     mesh.Vert3(0.5, 0.5, len);
                mesh.TexCoord2(len, 1);     mesh.Vert3(0.5, -0.5, len);

                mesh.TexCoord2(len, 0);     mesh.Vert3(0.5, 0.5, len);
                mesh.TexCoord2(0, 1);         mesh.Vert3(0.5, -0.5, -0);
                mesh.TexCoord2(0, 0);         mesh.Vert3(0.5, 0.5, -0);

                // Draw right
                mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1);mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1);
                mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, -0);
                mesh.TexCoord2(len, 1);     mesh.Vert3(-0.5, -0.5, len);
                mesh.TexCoord2(len, 0);     mesh.Vert3(-0.5, 0.5, len);

                mesh.TexCoord2(len, 0);     mesh.Vert3(-0.5, 0.5, len);
                mesh.TexCoord2(0, 0);         mesh.Vert3(-0.5, 0.5, -0);
                mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, -0);

                // Draw back
                mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1);mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1);
                mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, -0);
                mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, -0);
                mesh.TexCoord2(1, 1);         mesh.Vert3(0.5, -0.5, -0);

                mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, -0);
                mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, -0);
                mesh.TexCoord2(0, 0);         mesh.Vert3(-0.5, 0.5, -0);


                // Draw front
                mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1);mesh.Color4(lv, lv, lv, 1); mesh.Color4(lv, lv, lv, 1);
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

void Map::RebuildLights() {
    for(auto l: lights) {
    }
}

void Map::Draw() {
    glEnable(GL_CULL_FACE);

    //int sX = camera.position.x >= 0 ? camera.position.x / CHUNK_SIZE : (camera.position.x - (CHUNK_SIZE-1)) / CHUNK_SIZE;
    //int sZ = camera.position.z >= 0 ? camera.position.z / CHUNK_SIZE : (camera.position.z - (CHUNK_SIZE-1)) / CHUNK_SIZE;

    int sX = ((int)camera.position.x / CHUNK_SIZE);
    int sZ = ((int)camera.position.z / CHUNK_SIZE);
/*    std::cout << "sX/Y: " << sX << " , " << sZ << std::endl;
    auto index = std::make_pair(sX,sZ);
    BuildChunk(sX,sZ);
    Chunks[index].mesh.Draw(Mesh::MODE_TRIANGLES);*/

    int viewDist = 2;
    for(int x = sX - viewDist;x < sX+viewDist+1;x++) {
        for(int z = sZ-viewDist;z < sZ+viewDist+1;z++) {
            auto index = std::make_pair(x,z);
            if(Chunks[index].mesh.IsEmpty()) {
                BuildChunk(x,z);
            }
            Chunks[index].mesh.Draw(Mesh::MODE_TRIANGLES);
        }
    }

    if(SDL_GetKeyboardState(0)[SDL_SCANCODE_J]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else if(SDL_GetKeyboardState(0)[SDL_SCANCODE_K]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    int pCount = 0;
/*
    std::thread threads[NUM_THREADS];

    for(int i = 0;i < NUM_THREADS;i++) {
        threads[i] = std::thread(&Map::DrawSection, this, i);
    }

    for(int i = 0;i < NUM_THREADS;i++) {
        threads[i].join();
    }

 //   myTexArray.Bind();
    for(int i = 0;i < NUM_THREADS;i++) {
        myMeshes[i].Draw(Mesh::MODE_TRIANGLES);
    }*/
}

void Map::LoadBrickMetaData() {
    std::ifstream infile("bricks.txt");
    while(!infile.eof()) {
        std::array<int,6>arr;
        for(int i = 0;i < 6;i++) {
            infile >> arr[i];
        }
        BrickLookup.push_back(arr);
    }
}

void Map::ProcessMap_Simple() {
    for (int x = 0; x < width;x++) {
		for (int z = 0; z < depth; z++) {
			for (int y = height-1;y > 0; y--) {
                //
                if(rand()%8 == 1 && GetBrick(x,z,y) != 0) {
                    //SetBrick(x,z,y,2);
                }
            }
        }
    }
}

void Map::GenerateDefaultChunk(int x, int y) {

}

