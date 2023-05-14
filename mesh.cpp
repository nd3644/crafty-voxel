#include "mesh.h"

#include "mesh.h"

Mesh::Mesh() {
    xTrans = yTrans = zTrans = 0.0f;
}

Mesh::~Mesh() {
    if(glIsVertexArray(vertArrObj)) {
        glDeleteVertexArrays(1, &vertArrObj);
    }

    if(glIsBuffer(arrayBuffers[0])) {
        glDeleteBuffers(3, arrayBuffers);
    }

    vVertBuffer.clear();
    vTexCoords.clear();
    ColorBuffer.clear();
    Indices.clear();
}

void Mesh::Vert3(float x, float y,float z) {
    vec3_t v = { x + xTrans, y + yTrans, z + zTrans };
    vVertBuffer.push_back(v);
}

void Mesh::TexCoord2(float x, float y) {
    vec2_t v = { x, y };
    vTexCoords.push_back(v);
}

void Mesh::Color4(float r, float g, float b, float a) {
    RGBA col(r,g,b,a);
    ColorBuffer.push_back(col);
}

void Mesh::SetTranslation(float x, float y, float z) {
    xTrans = x;
    yTrans = y;
    zTrans = z;
}

void Mesh::Index1(int i) {
    Indices.push_back(i);
}

void Mesh::Draw(Mode mode) {
    if(!glIsVertexArray(vertArrObj)) {
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
    }

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, (vVertBuffer.size()*3) * sizeof(float), &vVertBuffer[0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[1]);
	glBufferData(GL_ARRAY_BUFFER, (vTexCoords.size()*2) * sizeof(float), &vTexCoords[0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[2]);
	glBufferData(GL_ARRAY_BUFFER, (ColorBuffer.size()*4) * sizeof(float), &ColorBuffer[0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[3]);
	glBufferData(GL_ARRAY_BUFFER, (Indices.size()) * sizeof(int), &Indices[0], GL_DYNAMIC_DRAW);

	glBindVertexArray(vertArrObj);
	glDrawArrays(mode, 0, vVertBuffer.size());
}

void Mesh::Clean() {
    vVertBuffer.clear();
    vTexCoords.clear();
    ColorBuffer.clear();
    Indices.clear();
}
