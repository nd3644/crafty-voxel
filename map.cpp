#include "map.h"

#include "ebmp.h"

#include <iostream>
#include <fstream>
#include "shader.h"
#include "cube.h"
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <thread>

Map::Map() : iBricks { 0 } {
    height = 32;
    width = 128;
    depth = 128;
    iBricks = nullptr;

    unsigned int num_cores = std::thread::hardware_concurrency();
    if (num_cores == 0) {
        std::cerr << "Unable to determine the number of cores." << std::endl;
        NUM_THREADS = 2;
    } else {
        std::clog << "This PC has " << num_cores << " cores/threads." << std::endl;
        NUM_THREADS = num_cores/2;
    }
    fAmbient = 0.3f;
}

Map::~Map() {
    if(iBricks != nullptr)
        delete[] iBricks;

    if(LightLevels != nullptr)
        delete[] LightLevels;

    if(LightColors != nullptr)
        delete[] LightColors;


        for (int i = 0; i < width / CHUNK_SIZE; i++) {
            for (int j = 0; j < depth / CHUNK_SIZE; j++) {
                delete Chunks[i][j];
            }
            delete[] Chunks[i];
        }
        delete[] Chunks;

}

void Map::FromBMP(std::string sfile) {
	Eternal::Bmp myBmp(sfile);
	if (myBmp.GetError().size() > 0) {
		std::cerr << "BMP Error: " << myBmp.GetError();
		exit(-1);
	}

    width = myBmp.GetWidth();
    depth = myBmp.GetHeight();
    iBricks = new int[width*height*depth];
    for(int i = 0;i < width*height*depth;i++) {
        iBricks[i] = 0;
    }
	for (int x = 0; x < width;x++) {
		for (int z = 0; z < depth; z++) {
			int height = myBmp.GetPixelRGBA(x, z).R;
			for (int y = 0; y < height/4; y++) {
				SetBrick(x, z, y, 1);
			}
		}
	}

    LightLevels = new int[width*height*depth];
    for(int i = 0;i < width*height*depth;i++) {
        LightLevels[i] = 0;
    }

    LightColors = new RGB[width*height*depth];
    for(int i = 0;i < width*height*depth;i++) {
        LightColors[i] = {0.1,0.1,0.1};
        RGB c;
    }

    std::vector<std::string>images = { "textures/grass.png", "textures/grass_side.png","textures/dirt.png","textures/torch.png" };
    myTexArray.Load(images);
    LoadBrickMetaData();
    ProcessMap_Simple();


    Chunks = new Mesh**[width / CHUNK_SIZE];
    for (int i = 0; i < width / CHUNK_SIZE; i++) {
        Chunks[i] = new Mesh*[depth / CHUNK_SIZE];
        for (int j = 0; j < depth / CHUNK_SIZE; j++) {
            Chunks[i][j] = new Mesh();
        }
    }

    RebuildAll();
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
    Mesh &mesh = *Chunks[chunkX][chunkZ];
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
                float c = (float)GetLightLvl(xindex,zindex,y) / 16.0f;
                int brickID = GetBrick(xindex,zindex,y);

                int len = 1;

                for(int i = 1;i < CHUNK_SIZE-1;i++) {
                    if(z < CHUNK_SIZE-1-i 
                    && GetBrick(xindex,zindex,y) == GetBrick(xindex,zindex+i,y)
                    && GetLightLvl(xindex,zindex,y) == GetLightLvl(xindex,zindex+i,y)) {
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

                float lv = (float)GetLightLvl(xindex,zindex,y) / 16.0f;
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

    for(int x = 0;x < width/CHUNK_SIZE;x++) {
        for(int z = 0;z < depth/CHUNK_SIZE;z++) {
            Chunks[x][z]->Draw(Mesh::MODE_TRIANGLES);
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

void Map::DrawSection(int which) {
    myMeshes[which].Clean();
    // Second half
    for (int y = 0; y < height; y++) {
        for (int x = (width/NUM_THREADS)*which; x < ((width/NUM_THREADS)*which)+(width/NUM_THREADS); x++) {
		    for (int z = 0; z < depth; z++) {
				if (GetBrick(x, z, y) > 0) {
                    if(GetBrick(x-1,z,y) != 0
                    && GetBrick(x+1,z,y) != 0
                    && GetBrick(x,z-1,y) != 0
                    && GetBrick(x,z+1,y) != 0
                    && GetBrick(x,z,y+1) != 0
                    && GetBrick(x,z,y-1) != 0) {
        //                    std::cout << "yes" << std::endl;
                        continue;
                    }

                    myMeshes[which].SetTranslation(x,y,z);
                    float c = 0.1f;
                    int len = 0;
                    for(int i = z+1;i < depth;i++) {
                        if(GetBrick(x,z,y) != GetBrick(x,i,y)) {
                            //std::cout << "stopping: " << z << " , " << i << " : " << GetBrick(x,z,y) << " , " << GetBrick(x,i,y) << std::endl;
                            len = i-z;
                            break;
                        }
                    }
                    if(len == 0) {
                        len = depth-z;
                    }

                    int brickID = GetBrick(x,z,y);
                    for(int i = 0;i < 6*6;i++)
                        myMeshes[which].Index1(BrickLookup[brickID-1][i/6]);

                    // Draw top
                    myMeshes[which].Color4(c, c, c, c);      myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c);
                    myMeshes[which].TexCoord2(0, 1+len);     myMeshes[which].Vert3(-0.5, 0.5, 0.5*len);
                    myMeshes[which].TexCoord2(1, 1+len);     myMeshes[which].Vert3(0.5, 0.5, 0.5*len);
                    myMeshes[which].TexCoord2(1, 0);         myMeshes[which].Vert3(0.5, 0.5, -0.5);

                    myMeshes[which].TexCoord2(0, 1+len);     myMeshes[which].Vert3(-0.5, 0.5, 0.5*len);
                    myMeshes[which].TexCoord2(1, 0);         myMeshes[which].Vert3(0.5, 0.5, -0.5);
                    myMeshes[which].TexCoord2(0, 0);         myMeshes[which].Vert3(-0.5, 0.5, -0.5);

                    // Draw bottom
                    myMeshes[which].Color4(c, c, c, c);      myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c);
                    myMeshes[which].TexCoord2(0, 1+len);     myMeshes[which].Vert3(-0.5, -0.5, 0.5*len);
                    myMeshes[which].TexCoord2(1, 1+len);     myMeshes[which].Vert3(0.5, -0.5, 0.5*len);
                    myMeshes[which].TexCoord2(1, 0);         myMeshes[which].Vert3(0.5, -0.5, -0.5);

                    myMeshes[which].TexCoord2(0, 1+len);     myMeshes[which].Vert3(-0.5, -0.5, 0.5*len);
                    myMeshes[which].TexCoord2(1, 0);         myMeshes[which].Vert3(0.5, -0.5, -0.5);
                    myMeshes[which].TexCoord2(0, 0);         myMeshes[which].Vert3(-0.5, -0.5, -0.5);

                    // Draw left
                    myMeshes[which].Color4(c, c, c, c);      myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c);
                    myMeshes[which].TexCoord2(0, 1);         myMeshes[which].Vert3(0.5, -0.5, -0.5);
                    myMeshes[which].TexCoord2(1+len, 0);     myMeshes[which].Vert3(0.5, 0.5, 0.5*len);
                    myMeshes[which].TexCoord2(1+len, 1);     myMeshes[which].Vert3(0.5, -0.5, 0.5*len);

                    myMeshes[which].TexCoord2(1+len, 0);     myMeshes[which].Vert3(0.5, 0.5, 0.5*len);
                    myMeshes[which].TexCoord2(0, 1);         myMeshes[which].Vert3(0.5, -0.5, -0.5);
                    myMeshes[which].TexCoord2(0, 0);         myMeshes[which].Vert3(0.5, 0.5, -0.5);

                    // Draw right
                    myMeshes[which].Color4(c, c, c, c);      myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c);
                    myMeshes[which].TexCoord2(0, 1);         myMeshes[which].Vert3(-0.5, -0.5, -0.5);
                    myMeshes[which].TexCoord2(1+len, 1);     myMeshes[which].Vert3(-0.5, -0.5, 0.5*len);
                    myMeshes[which].TexCoord2(1+len, 0);     myMeshes[which].Vert3(-0.5, 0.5, 0.5*len);

                    myMeshes[which].TexCoord2(1+len, 0);     myMeshes[which].Vert3(-0.5, 0.5, 0.5*len);
                    myMeshes[which].TexCoord2(0, 0);         myMeshes[which].Vert3(-0.5, 0.5, -0.5);
                    myMeshes[which].TexCoord2(0, 1);         myMeshes[which].Vert3(-0.5, -0.5, -0.5);

                    c = 0.7f;
                    // Draw back
                    myMeshes[which].Color4(c, c, c, c);      myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c);
                    myMeshes[which].TexCoord2(0, 1);         myMeshes[which].Vert3(-0.5, -0.5, -0.5);
                    myMeshes[which].TexCoord2(1, 0);         myMeshes[which].Vert3(0.5, 0.5, -0.5);
                    myMeshes[which].TexCoord2(1, 1);         myMeshes[which].Vert3(0.5, -0.5, -0.5);

                    myMeshes[which].TexCoord2(1, 0);         myMeshes[which].Vert3(0.5, 0.5, -0.5);
                    myMeshes[which].TexCoord2(0, 1);         myMeshes[which].Vert3(-0.5, -0.5, -0.5);
                    myMeshes[which].TexCoord2(0, 0);         myMeshes[which].Vert3(-0.5, 0.5, -0.5);


                    // Draw front
                    myMeshes[which].Color4(c, c, c, c);      myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c);
                    myMeshes[which].TexCoord2(0, 1);         myMeshes[which].Vert3(-0.5, -0.5, 0.5*len);
                    myMeshes[which].TexCoord2(1, 1);         myMeshes[which].Vert3(0.5, -0.5, 0.5*len);
                    myMeshes[which].TexCoord2(1, 0);         myMeshes[which].Vert3(0.5, 0.5, 0.5*len);

                    myMeshes[which].TexCoord2(1, 0);         myMeshes[which].Vert3(0.5, 0.5, 0.5*len);
                    myMeshes[which].TexCoord2(0, 0);         myMeshes[which].Vert3(-0.5, 0.5, 0.5*len);
                    myMeshes[which].TexCoord2(0, 1);         myMeshes[which].Vert3(-0.5, -0.5, 0.5*len);
 
                    z += len;
                }
            }
        }
    }
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

