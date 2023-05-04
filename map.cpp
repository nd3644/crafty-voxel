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
			int height = myBmp.GetPixelRGBA(x, z).R / 4;
			for (int y = 0; y < height; y++) {
				SetBrick(x, z, y, 1);
			}
		}
	}
}

void Map::Draw() {
	for (int x = 0; x < 32; x++) {
		for (int z = 0; z < 32; z++) {
			for (int y = 0; y < 16; y++) {
				if (GetBrick(x, z, 256 - y) != 0) {
					GLuint model = glGetUniformLocation(myProgram, "Model");
					GLuint view = glGetUniformLocation(myProgram, "View");
					modelMatrix = glm::mat4(1);
					modelMatrix = glm::translate(modelMatrix, glm::vec3(x*2, y*2, -5*2 - z));
					glUniformMatrix4fv(model, 1, GL_FALSE, glm::value_ptr(modelMatrix));

					viewMatrix = glm::mat4(1);
					viewMatrix = glm::rotate(viewMatrix, 0.5f, glm::vec3(0, 0, 0.5f));
					viewMatrix = glm::translate(viewMatrix, glm::vec3(2,5,-5));
					glUniformMatrix4fv(view, 1, GL_FALSE, glm::value_ptr(viewMatrix));

					glBindVertexArray(myVAO);
					glDrawArrays(GL_TRIANGLES, 0, vertList.size());
				}
			}
		}
	}
}