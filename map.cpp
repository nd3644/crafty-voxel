#include "map.h"

#include "ebmp.h"

#include <iostream>
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
    std::vector<std::string>images = { "textures/grass.png", "textures/grass_side.png" };
    myTexArray.Load(images);
}

void Map::Draw() {
    cube_verts[0] = { -0.5, -0.5, -0.5 };
    cube_verts[1] = { 0.5, -0.5, -0.5 };
    cube_verts[2] = { 0.5, 0.5, -0.5 };
    cube_verts[3] = { -0.5, 0.5, -0.5 };

    cube_verts[4] = { -0.5, -0.5, 0.5 };
    cube_verts[5] = { 0.5, -0.5, 0.5 };
    cube_verts[7] = { -0.5, 0.5, 0.5 };
    cube_verts[6] = { 0.5, 0.5, 0.5 };

    glEnable(GL_CULL_FACE);
//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
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
    }
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

                    int id = (rand()%2);
                    for(int i = 0;i < 6;i++) {
                        myMeshes[which].Index1(0);
                    }

                    // Draw top
                    myMeshes[which].Color4(c, c, c, c);      myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c); myMeshes[which].Color4(c, c, c, c);
                    myMeshes[which].TexCoord2(0, 1+len);     myMeshes[which].Vert3(-0.5, 0.5, 0.5 + len);
                    myMeshes[which].TexCoord2(1, 1+len);     myMeshes[which].Vert3(0.5, 0.5, 0.5 + len);
                    myMeshes[which].TexCoord2(1, 0);         myMeshes[which].Vert3(0.5, 0.5, -0.5);

                    myMeshes[which].TexCoord2(0, 1+len);     myMeshes[which].Vert3(-0.5, 0.5, 0.5 + len);
                    myMeshes[which].TexCoord2(1, 0);         myMeshes[which].Vert3(0.5, 0.5, -0.5);
                    myMeshes[which].TexCoord2(0, 0);         myMeshes[which].Vert3(-0.5, 0.5, -0.5);

                    for(int i = 0;i < 4*6;i++) {
                        myMeshes[which].Index1(1);
                    }

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

void DrawMiniMap() {
    
}