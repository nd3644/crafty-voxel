#include "mesh.h"

#include "mesh.h"
#include <iostream>
#include <chrono>

#include "globals.h"

Mesh::Mesh() {
    xTrans = yTrans = zTrans = 0.0f;
    glGenVertexArrays(1, &vertArrObj);
    glBindVertexArray(vertArrObj);
    glGenBuffers(4, arrayBuffers);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[2]);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, 0);  
    glEnableVertexAttribArray(2);

    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[3]);
    glVertexAttribIPointer(3, 1, GL_INT, 0, 0);

    bDataIsBound = false;
}

Mesh::~Mesh() {
    if(glIsVertexArray(vertArrObj)) {
        glDeleteVertexArrays(1, &vertArrObj);
    }

    if(glIsBuffer(arrayBuffers[0])) {
        glDeleteBuffers(4, arrayBuffers);
    }

    vVertBuffer.clear();
    vTexCoords.clear();
    ColorBuffer.clear();
    Indices.clear();
}

void Mesh::BindBufferData() {
    glBindVertexArray(vertArrObj);

    CheckErr();

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, (vVertBuffer.size()*3) * sizeof(float), &vVertBuffer[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[1]);
	glBufferData(GL_ARRAY_BUFFER, (vTexCoords.size()*2) * sizeof(float), &vTexCoords[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[2]);
	glBufferData(GL_ARRAY_BUFFER, (ColorBuffer.size()*4) * sizeof(float), &ColorBuffer[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[3]);
	glBufferData(GL_ARRAY_BUFFER, (Indices.size()) * sizeof(int), &Indices[0], GL_STATIC_DRAW);

    bDataIsBound = true;
}

void Mesh::Draw(Mode mode) {
    if(!bDataIsBound){
        BindBufferData();
    }
    CheckErr();

	glBindVertexArray(vertArrObj);
	glDrawArrays(mode, 0, vVertBuffer.size());
    gblPolyCount += (vVertBuffer.size()/3);
}

void Mesh::Clean() {
    vVertBuffer.clear();
    vTexCoords.clear();
    ColorBuffer.clear();
    Indices.clear();
}
