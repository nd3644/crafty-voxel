#include "map.h"

#include "ebmp.h"

#include <iostream>
#include "shader.h"
#include "cube.h"
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Map::Map() {
	iBricks = new int[width*height*depth];
    for(int i = 0;i < width*height*depth;i++) {
        iBricks[i] = 0;
    }
}

Map::~Map() {
	delete[] iBricks;
}

void Map::FromBMP(std::string sfile) {
	Eternal::Bmp myBmp(sfile);
	if (myBmp.GetError().size() > 0) {
		std::cerr << "BMP Error: " << myBmp.GetError();
		exit(-1);
	}

	for (int x = 0; x < 128;x++) {
		for (int z = 0; z < 128; z++) {
			int height = myBmp.GetPixelRGBA(x, z).R;
			for (int y = 0; y < height/4; y++) {
				SetBrick(x, z, y, 1);
			}
            std::cout << height << " ";
		}
        std::cout << std::endl;
	}
}

void Map::Draw() {
	for (int x = 0; x < 32; x++) {
		for (int z = 0; z < 32; z++) {
			for (int y = 0; y < 32; y++) {
				if (GetBrick(x, z, y) != 0) {
					GLuint model = glGetUniformLocation(myProgram, "Model");

					modelMatrix = glm::mat4(1);
					modelMatrix = glm::translate(modelMatrix, glm::vec3(x, y, z));
					glUniformMatrix4fv(model, 1, GL_FALSE, glm::value_ptr(modelMatrix));

					glBindVertexArray(myVAO);
					glDrawArrays(GL_TRIANGLES, 0, vertList.size());
				}
			}
		}
	}
}