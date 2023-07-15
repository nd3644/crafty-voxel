#include "chunkmesh.h"

#include "chunkmesh.h"

#include "mesh.h"
#include <iostream>
#include <chrono>
#include <cstddef>

#include "globals.h"


/* 
struct triangle_t {
    glm::vec3 v[3]; // verts
    glm::vec3 c[3]; // color
    glm::vec2 t[3]; // texcoords
    int i[3];       // indices
};
*/

ChunkMesh::ChunkMesh() {
    xTrans = yTrans = zTrans = 0.0f;
    glGenVertexArrays(1, &vertArrObj);
    glBindVertexArray(vertArrObj);
    glGenBuffers(1, arrayBuffers);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(triangle_t), (void*)offsetof(triangle_t, v));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(triangle_t), (void*)offsetof(triangle_t, t));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(triangle_t), (void*)offsetof(triangle_t, c));
    glEnableVertexAttribArray(2);

    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 1, GL_INT, sizeof(triangle_t), (void*)offsetof(triangle_t, i));

    bDataIsBound = false;
}

ChunkMesh::~ChunkMesh() {
    if(glIsVertexArray(vertArrObj)) {
        glDeleteVertexArrays(1, &vertArrObj);
    }

    if(glIsBuffer(arrayBuffers[0])) {
        glDeleteBuffers(4, arrayBuffers);
    }
    Triangles.clear();
}

void ChunkMesh::BindBufferData() {
    Triangles.shrink_to_fit();

    glBindVertexArray(vertArrObj);

    CheckErr();

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, Triangles.size() * sizeof(triangle_t), &Triangles[0], GL_STATIC_DRAW);

/*    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[1]);
	glBufferData(GL_ARRAY_BUFFER, Triangles.size() * sizeof(triangle_t), &Triangles[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[2]);
	glBufferData(GL_ARRAY_BUFFER, Triangles.size() * sizeof(triangle_t), &Triangles[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[3]);
	glBufferData(GL_ARRAY_BUFFER, Triangles.size() * sizeof(triangle_t), &Triangles[0], GL_STATIC_DRAW);*/

    bDataIsBound = true;
}

void ChunkMesh::Draw(Mode mode) {
    if(!bDataIsBound){
        BindBufferData();
    }
    CheckErr();

	glBindVertexArray(vertArrObj);
	glDrawArrays(mode, 0, Triangles.size()*3);
}

void ChunkMesh::Clean() {
    Triangles.clear();
}
