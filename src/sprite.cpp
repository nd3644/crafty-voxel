#include "sprite.h"
#include "mesh.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <tuple>
#include <libnoise/noise.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Eternal::Sprite::Sprite() {
    w = h = 0;
    bFlipU = bFlipV = false;
    SetColor(255,255,255,255);
    bLoaded = false;

    for(int i = 0;i < 6;i++) {
        vVertexBuffer[i].x = vVertexBuffer[i].y = 0;
        vTexCoords[i].x = vTexCoords[i].y = 0;
        Indices[i] = 0;
    }
}

Eternal::Sprite::~Sprite() {
    Cleanup();
}

void Eternal::Sprite::ClearData() {
}

void Eternal::Sprite::Cleanup() {
    if(bLoaded) {
        glDeleteTextures(1, &myTexID);
        glDeleteVertexArrays(1, &vertArrObj);
        glDeleteBuffers(4, arrayBuffers);
    }
}
uint8_t mapDoubleToUint8(double input) {
    // Ensure the input is within the [-1, 1] range
    if (input < -1.0) {
        input = -1.0;
    } else if (input > 1.0) {
        input = 1.0;
    }

    // Map the double to a uint8_t using linear mapping
    uint8_t output = static_cast<uint8_t>((input + 1.0) * 127.5);

    return output;
}
void Eternal::Sprite::FromNoise(noise::module::Module &sourceModule, int width, int height) {
    sName = "FromNoise";

    Cleanup();
    
    glGenTextures(1, &myTexID);

    glGenVertexArrays(1, &vertArrObj);
    glBindVertexArray(vertArrObj);
    glGenBuffers(4, arrayBuffers);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[0]);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
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
    
    glBindTexture(GL_TEXTURE_2D, myTexID);
   
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    auto *pixelBuffer = new glm::u8vec3[width*height];

    for(int x = 0;x < width;x++) {
        for(int y = 0;y < height;y++) {
            uint8_t val = mapDoubleToUint8(sourceModule.GetValue(x,y,0.5));
            pixelBuffer[(y*width)+x] = {val,val,val};
        }
    }

    GLuint format = GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixelBuffer);

    bLoaded = true;

    delete [] pixelBuffer;
}

void Eternal::Sprite::Load(std::string sfile) {
    sName = sfile;

    Cleanup();
    
    glGenTextures(1, &myTexID);

    glGenVertexArrays(1, &vertArrObj);
    glBindVertexArray(vertArrObj);
    glGenBuffers(4, arrayBuffers);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[0]);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
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

    SDL_Surface *surf = IMG_Load(sfile.c_str());
    if(surf == nullptr) {
        std::cerr << "error loading " << sfile << std::endl;
        exit(1);
    }

    
    glBindTexture(GL_TEXTURE_2D, myTexID);
   
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLuint format = (surf->format->BytesPerPixel == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, surf->w, surf->h, 0, format, GL_UNSIGNED_BYTE, surf->pixels);

    w = surf->w;
    h = surf->h;

    SDL_FreeSurface(surf);
    bLoaded = true;
}

void Eternal::Sprite::Bind(int unit) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, myTexID);
}

void Eternal::Sprite::ForceResize(int width, int height) {
    w = width;
    h = height;
}

void Eternal::Sprite::Draw(float x, float y, float w, float h,
                        float cx, float cy, float cw, float ch) {
    Rect r(x,y,w,h);
    Rect c(cx,cy,cw,ch);
    Draw(r,c);
}

void Eternal::Sprite::Draw(Rect &pos, Rect &clip) {
    glBindTexture(GL_TEXTURE_2D, myTexID);
    Draw_NoBind(pos, clip);
}

void Eternal::Sprite::Draw_NoBind(Rect &pos, Rect &clip) {

    vVertexBuffer[0].x = pos.x;             vVertexBuffer[0].y = pos.y;
    vVertexBuffer[1].x = pos.x + pos.w;     vVertexBuffer[1].y = pos.y;
    vVertexBuffer[2].x = pos.x;             vVertexBuffer[2].y = pos.y + pos.h;

    vVertexBuffer[3].x = pos.x + pos.w;     vVertexBuffer[3].y = pos.y;
    vVertexBuffer[4].x = pos.x + pos.w;     vVertexBuffer[4].y = pos.y + pos.h;
    vVertexBuffer[5].x = pos.x;             vVertexBuffer[5].y = pos.y + pos.h;

    vTexCoords[0].x = 0;        vTexCoords[0].y = 0;
    vTexCoords[1].x = 1;        vTexCoords[1].y = 0;
    vTexCoords[2].x = 0;        vTexCoords[2].y = 1;

    vTexCoords[3].x = 1;        vTexCoords[3].y = 0;
    vTexCoords[4].x = 1;        vTexCoords[4].y = 1;
    vTexCoords[5].x = 0;        vTexCoords[5].y = 1;

    for(int i = 0;i < 6;i++)
        Indices[i] = 0;

    glBindVertexArray(vertArrObj);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, (6*2) * sizeof(float), &vVertexBuffer[0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[1]);
	glBufferData(GL_ARRAY_BUFFER, (6*2) * sizeof(float), &vTexCoords[0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[2]);
	glBufferData(GL_ARRAY_BUFFER, (6*4) * sizeof(float), &ColorBuffer[0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[3]);
	glBufferData(GL_ARRAY_BUFFER, (6) * sizeof(int), &Indices[0], GL_DYNAMIC_DRAW);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Eternal::Sprite::SetColor(float r, float g, float b, float a) {
    for(int i = 0;i < 6;i++) {
        ColorBuffer[i].r = r / 255.0f;
        ColorBuffer[i].g = g / 255.0f;
        ColorBuffer[i].b = b / 255.0f;
        ColorBuffer[i].a = a / 255.0f;
    }
}

bool Eternal::Sprite::IsLoaded() const {
    return bLoaded;
}