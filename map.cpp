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
}

Map::~Map() {
    if(iBricks != nullptr)
        delete[] iBricks;


        for (int i = 0; i < width / 16; i++) {
            for (int j = 0; j < depth / 16; j++) {
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
/*
*/
    std::vector<std::string>images = { "textures/grass.png", "textures/grass_side.png","textures/dirt.png" };
    myTexArray.Load(images);
    LoadBrickMetaData();
    ProcessMap_Simple();


    Chunks = new Mesh**[width / 16];
    for (int i = 0; i < width / 16; i++) {
        Chunks[i] = new Mesh*[depth / 16];
        for (int j = 0; j < depth / 16; j++) {
            Chunks[i][j] = new Mesh();
        }
    }
}

void Map::DrawChunk(int chunkX, int chunkZ) {
    Mesh &mesh = *Chunks[chunkX][chunkZ];
    if(mesh.IsEmpty()) {
        for (int y = 0; y < 32; y++) {
            for (int x = 0; x < 16;x++) {
                for (int z = 0; z < 16; z++) {
                    int xindex = (chunkX*16)+x;
                    int zindex = (chunkZ*16)+z;

                    if (GetBrick(xindex, zindex, y) <= 0) {
                        continue;
                    }
                    if(GetBrick(x-1,z,y) != 0
                    && GetBrick(x+1,z,y) != 0
                    && GetBrick(x,z-1,y) != 0
                    && GetBrick(x,z+1,y) != 0
                    && GetBrick(x,z,y+1) != 0
                    && GetBrick(x,z,y-1) != 0) {
        //                    std::cout << "yes" << std::endl;
                        continue;
                    }

                    float posX = (chunkX*16)+x;
                    float posZ = (chunkZ*16)+z;
                    mesh.SetTranslation(posX,y,posZ);
                    float c = 0.8f;
                    int brickID = GetBrick(xindex,zindex,y);

                    for(int i = 0;i < 5*6;i++)
                        mesh.Index1(0);//BrickLookup[brickID-1][i/6]);

                    // Draw top
                    mesh.Color4(c, c, c, c);      mesh.Color4(c, c, c, c); mesh.Color4(c, c, c, c); mesh.Color4(c, c, c, c); mesh.Color4(c, c, c, c); mesh.Color4(c, c, c, c);
                    mesh.TexCoord2(0, 1);     mesh.Vert3(-0.5, 0.5, 0.5 );
                    mesh.TexCoord2(1, 1);     mesh.Vert3(0.5, 0.5, 0.5 );
                    mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, -0.5);

                    mesh.TexCoord2(0, 1);     mesh.Vert3(-0.5, 0.5, 0.5 );
                    mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, -0.5);
                    mesh.TexCoord2(0, 0);         mesh.Vert3(-0.5, 0.5, -0.5);

                    c = 0.4f;
                    // Draw left
                    mesh.Color4(c, c, c, c);      mesh.Color4(c, c, c, c); mesh.Color4(c, c, c, c); mesh.Color4(c, c, c, c); mesh.Color4(c, c, c, c); mesh.Color4(c, c, c, c);
                    mesh.TexCoord2(0, 1);         mesh.Vert3(0.5, -0.5, -0.5);
                    mesh.TexCoord2(1, 0);     mesh.Vert3(0.5, 0.5, 0.5);
                    mesh.TexCoord2(1, 1);     mesh.Vert3(0.5, -0.5, 0.5);

                    mesh.TexCoord2(1, 0);     mesh.Vert3(0.5, 0.5, 0.5);
                    mesh.TexCoord2(0, 1);         mesh.Vert3(0.5, -0.5, -0.5);
                    mesh.TexCoord2(0, 0);         mesh.Vert3(0.5, 0.5, -0.5);

                    c = 0.6f;
                    // Draw right
                    mesh.Color4(c, c, c, c);      mesh.Color4(c, c, c, c); mesh.Color4(c, c, c, c); mesh.Color4(c, c, c, c); mesh.Color4(c, c, c, c); mesh.Color4(c, c, c, c);
                    mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, -0.5);
                    mesh.TexCoord2(1, 1);     mesh.Vert3(-0.5, -0.5, 0.5);
                    mesh.TexCoord2(1, 0);     mesh.Vert3(-0.5, 0.5, 0.5);

                    mesh.TexCoord2(1, 0);     mesh.Vert3(-0.5, 0.5, 0.5);
                    mesh.TexCoord2(0, 0);         mesh.Vert3(-0.5, 0.5, -0.5);
                    mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, -0.5);

                    c = 0.7f;
                    // Draw back
                    mesh.Color4(c, c, c, c);      mesh.Color4(c, c, c, c); mesh.Color4(c, c, c, c); mesh.Color4(c, c, c, c); mesh.Color4(c, c, c, c); mesh.Color4(c, c, c, c);
                    mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, -0.5);
                    mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, -0.5);
                    mesh.TexCoord2(1, 1);         mesh.Vert3(0.5, -0.5, -0.5);

                    mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, -0.5);
                    mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, -0.5);
                    mesh.TexCoord2(0, 0);         mesh.Vert3(-0.5, 0.5, -0.5);


                    // Draw front
                    mesh.Color4(c, c, c, c);      mesh.Color4(c, c, c, c); mesh.Color4(c, c, c, c); mesh.Color4(c, c, c, c); mesh.Color4(c, c, c, c); mesh.Color4(c, c, c, c);
                    mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, 0.5);
                    mesh.TexCoord2(1, 1);         mesh.Vert3(0.5, -0.5, 0.5);
                    mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, 0.5);

                    mesh.TexCoord2(1, 0);         mesh.Vert3(0.5, 0.5, 0.5);
                    mesh.TexCoord2(0, 0);         mesh.Vert3(-0.5, 0.5, 0.5);
                    mesh.TexCoord2(0, 1);         mesh.Vert3(-0.5, -0.5, 0.5);
                }
            }
        }
    }
    mesh.Draw(Mesh::MODE_TRIANGLES);
}

void Map::Draw() {

    for(int x = 0;x < 256/16;x++) {
        for(int z = 0;z < 256/16;z++) {
            DrawChunk(x,z);
        }
    }

    glEnable(GL_CULL_FACE);
/*//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    int pCount = 0;

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
                    float c = 0.8f;
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
                    myMeshes[which].TexCoord2(0, 1+len);     myMeshes[which].Vert3(-0.5, 0.5, 0.5 + len);
                    myMeshes[which].TexCoord2(1, 1+len);     myMeshes[which].Vert3(0.5, 0.5, 0.5 + len);
                    myMeshes[which].TexCoord2(1, 0);         myMeshes[which].Vert3(0.5, 0.5, -0.5);

                    myMeshes[which].TexCoord2(0, 1+len);     myMeshes[which].Vert3(-0.5, 0.5, 0.5 + len);
                    myMeshes[which].TexCoord2(1, 0);         myMeshes[which].Vert3(0.5, 0.5, -0.5);
                    myMeshes[which].TexCoord2(0, 0);         myMeshes[which].Vert3(-0.5, 0.5, -0.5);

                    // Draw bottom
                    myMeshes[which].Color4(c, c, c, c);      myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c);
                    myMeshes[which].TexCoord2(0, 1+len);     myMeshes[which].Vert3(-0.5, -0.5, 0.5 + len);
                    myMeshes[which].TexCoord2(1, 1+len);     myMeshes[which].Vert3(0.5, -0.5, 0.5 + len);
                    myMeshes[which].TexCoord2(1, 0);         myMeshes[which].Vert3(0.5, -0.5, -0.5);

                    myMeshes[which].TexCoord2(0, 1+len);     myMeshes[which].Vert3(-0.5, -0.5, 0.5 + len);
                    myMeshes[which].TexCoord2(1, 0);         myMeshes[which].Vert3(0.5, -0.5, -0.5);
                    myMeshes[which].TexCoord2(0, 0);         myMeshes[which].Vert3(-0.5, -0.5, -0.5);

                    c = 0.4f;
                    // Draw left
                    myMeshes[which].Color4(c, c, c, c);      myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c);
                    myMeshes[which].TexCoord2(0, 1);         myMeshes[which].Vert3(0.5, -0.5, -0.5);
                    myMeshes[which].TexCoord2(1+len, 0);     myMeshes[which].Vert3(0.5, 0.5, 0.5+len);
                    myMeshes[which].TexCoord2(1+len, 1);     myMeshes[which].Vert3(0.5, -0.5, 0.5+len);

                    myMeshes[which].TexCoord2(1+len, 0);     myMeshes[which].Vert3(0.5, 0.5, 0.5+len);
                    myMeshes[which].TexCoord2(0, 1);         myMeshes[which].Vert3(0.5, -0.5, -0.5);
                    myMeshes[which].TexCoord2(0, 0);         myMeshes[which].Vert3(0.5, 0.5, -0.5);

                    c = 0.6f;
                    // Draw right
                    myMeshes[which].Color4(c, c, c, c);      myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c);
                    myMeshes[which].TexCoord2(0, 1);         myMeshes[which].Vert3(-0.5, -0.5, -0.5);
                    myMeshes[which].TexCoord2(1+len, 1);     myMeshes[which].Vert3(-0.5, -0.5, 0.5+len);
                    myMeshes[which].TexCoord2(1+len, 0);     myMeshes[which].Vert3(-0.5, 0.5, 0.5+len);

                    myMeshes[which].TexCoord2(1+len, 0);     myMeshes[which].Vert3(-0.5, 0.5, 0.5+len);
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
                    myMeshes[which].TexCoord2(0, 1);         myMeshes[which].Vert3(-0.5, -0.5, 0.5+len);
                    myMeshes[which].TexCoord2(1, 1);         myMeshes[which].Vert3(0.5, -0.5, 0.5+len);
                    myMeshes[which].TexCoord2(1, 0);         myMeshes[which].Vert3(0.5, 0.5, 0.5+len);

                    myMeshes[which].TexCoord2(1, 0);         myMeshes[which].Vert3(0.5, 0.5, 0.5+len);
                    myMeshes[which].TexCoord2(0, 0);         myMeshes[which].Vert3(-0.5, 0.5, 0.5+len);
                    myMeshes[which].TexCoord2(0, 1);         myMeshes[which].Vert3(-0.5, -0.5, 0.5+len);
 
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
                if(GetBrick(x,z,y+1) == 1 || GetBrick(x,z,y+1) == 2) {
                    SetBrick(x,z,y,2);
                }
            }
        }
    }
}

