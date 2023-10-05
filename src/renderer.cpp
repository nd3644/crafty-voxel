#include "renderer.h"
#include <stdio.h>

Eternal::Renderer::Renderer() {
    bInitialized = false;
}

Eternal::Renderer::~Renderer() {
    if(bInitialized) {
        glDeleteVertexArrays(1, &vertArrObj);
        glDeleteBuffers(2, arrayBuffers);
    }
}

void Eternal::Renderer::Initialize() {
    bInitialized = true;

    glGenVertexArrays(1, &vertArrObj);
    glBindVertexArray(vertArrObj);
    glGenBuffers(2, arrayBuffers);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[0]);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[1]);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

}

void Eternal::Renderer::SetColor(float r, float g, float b, float a) {
    for(int i = 0;i < 6;i++) {
        ColorBuffer[i].r = r;
        ColorBuffer[i].g = g;
        ColorBuffer[i].b = b;
        ColorBuffer[i].a = a;
    }
}

void Eternal::Renderer::DrawQuad(Rect r) {
    Quad q;
    q.FromRect(r);
    DrawQuad(q);
}

void Eternal::Renderer::DrawBox(Quad &q) {
    DrawLine(q.v[0].x, q.v[0].y, q.v[1].x, q.v[1].y); // top
    DrawLine(q.v[2].x, q.v[2].y, q.v[3].x, q.v[3].y); // bottom
    DrawLine(q.v[0].x, q.v[0].y, q.v[3].x, q.v[3].y); // left
    DrawLine(q.v[1].x, q.v[1].y, q.v[2].x, q.v[2].y); // right
}

void Eternal::Renderer::DrawBox(Rect r) {
    Quad q;
    q.FromRect(r);
    DrawBox(q);
}

void Eternal::Renderer::DrawQuad(Quad &quad) {
    glBindTexture(GL_TEXTURE_2D, 0);

    vVertexBuffer[0].x = quad.v[0].x; vVertexBuffer[0].y = quad.v[0].y;
    vVertexBuffer[1].x = quad.v[1].x; vVertexBuffer[1].y = quad.v[1].y;
    vVertexBuffer[2].x = quad.v[2].x; vVertexBuffer[2].y = quad.v[2].y;

    vVertexBuffer[3].x = quad.v[0].x; vVertexBuffer[3].y = quad.v[0].y;
    vVertexBuffer[4].x = quad.v[2].x; vVertexBuffer[4].y = quad.v[2].y;
    vVertexBuffer[5].x = quad.v[3].x; vVertexBuffer[5].y = quad.v[3].y;

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, (6*2) * sizeof(float), &vVertexBuffer[0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[1]);
	glBufferData(GL_ARRAY_BUFFER, (6*4) * sizeof(float), &ColorBuffer[0], GL_DYNAMIC_DRAW);

	glBindVertexArray(vertArrObj);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Eternal::Renderer::DrawTriangle(Triangle &triangle) {
    glBindTexture(GL_TEXTURE_2D, 0);
    
    vVertexBuffer[0].x = triangle.v[0].x; vVertexBuffer[0].y = triangle.v[0].y;
    vVertexBuffer[1].x = triangle.v[1].x; vVertexBuffer[1].y = triangle.v[1].y;
    vVertexBuffer[2].x = triangle.v[2].x; vVertexBuffer[2].y = triangle.v[2].y;


    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, (3*2) * sizeof(float), &vVertexBuffer[0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[1]);
	glBufferData(GL_ARRAY_BUFFER, (3*4) * sizeof(float), &ColorBuffer, GL_DYNAMIC_DRAW);
	glBindVertexArray(vertArrObj);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Eternal::Renderer::DrawLine(float sx, float sy, float fx, float fy) {
    glBindTexture(GL_TEXTURE_2D, 0);
    
    vVertexBuffer[0].x = sx;    vVertexBuffer[0].y = sy;
    vVertexBuffer[1].x = fx;    vVertexBuffer[1].y = fy;

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, (2*2) * sizeof(float), &vVertexBuffer[0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[1]);
	glBufferData(GL_ARRAY_BUFFER, (2*4) * sizeof(float), &ColorBuffer, GL_DYNAMIC_DRAW);

	glBindVertexArray(vertArrObj);
	glDrawArrays(GL_LINES, 0, 2);
}

void Eternal::Renderer::PlotPoint(float x, float y) {
	glBindTexture(GL_TEXTURE_2D, 0);
    
    vVertexBuffer[0].x = x;    vVertexBuffer[0].y = y;

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float), &vVertexBuffer[0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[1]);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float), &ColorBuffer, GL_DYNAMIC_DRAW);

	glBindVertexArray(vertArrObj);
	glDrawArrays(GL_POINTS, 0, 1);
}

bool Eternal::Renderer::IsInitialized() const {
    return bInitialized;
}