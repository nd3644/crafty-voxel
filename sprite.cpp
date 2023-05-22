#include "sprite.h"
#include "mesh.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>

Eternal::Sprite::Sprite() {
    w = h = 0;
    bFlipU = bFlipV = false;
    SetColor(255,255,255,255);
    bLoaded = false;
}

Eternal::Sprite::~Sprite() {
    Cleanup();
}

void Eternal::Sprite::ClearData() {
}

void Eternal::Sprite::Cleanup() {
    if(glIsTexture(myTexID)) {
        glDeleteTextures(1, &myTexID);
    }

    if(glIsVertexArray(vertArrObj)) {
        glDeleteVertexArrays(1, &vertArrObj);
    }

    if(glIsBuffer(arrayBuffers[0])) {
        glDeleteBuffers(3, arrayBuffers);
    }
}

void Eternal::Sprite::FromData(uint8_t *pixels, int width, int height, int bpp) {
    sName = "manual";

    Cleanup();
    glGenTextures(1, &myTexID);

    glGenVertexArrays(1, &vertArrObj);
    glBindVertexArray(vertArrObj);
    glGenBuffers(3, arrayBuffers);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[0]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[1]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[2]);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, myTexID);
   
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = (bpp == 24) ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);

    w = width;
    h = height;

    bLoaded = true;
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

    SDL_Surface *surf = IMG_Load(sfile.c_str());
    if(surf == nullptr) {
        return;
    }
    else {

    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, myTexID);
   
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf->pixels);

    w = surf->w;
    h = surf->h;

    SDL_FreeSurface(surf);
    bLoaded = true;
}

void Eternal::Sprite::Bind(int unit) {
    glEnable(GL_TEXTURE_2D);
    switch(unit) {
        case 0:
            glActiveTexture(GL_TEXTURE0);
        break;
        case 1:
            glActiveTexture(GL_TEXTURE1);
        break;
        case 2:
            glActiveTexture(GL_TEXTURE2);
        break;
        case 3:
            glActiveTexture(GL_TEXTURE3);
        break;
        case 4:
            glActiveTexture(GL_TEXTURE4);
        break;
        default:
            std::cout << "bindind sprite to invalid unit " << unit << std::endl;
        break;
    };
    glBindTexture(GL_TEXTURE_2D, myTexID);
}

void Eternal::Sprite::ForceResize(int width, int height) {
    w = width;
    h = height;
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


    const float cx = clip.x / w;
    const float cy = clip.y / h;

    const float cw = (clip.x / w) + clip.w / w;
    const float ch = (clip.y / h) + clip.h / h;

    vTexCoords[0].x = cx;        vTexCoords[0].y = cy;
    vTexCoords[1].x = cw;        vTexCoords[1].y = cy;
    vTexCoords[2].x = cx;        vTexCoords[2].y = ch;

    vTexCoords[3].x = cw;        vTexCoords[3].y = cy;
    vTexCoords[4].x = cw;        vTexCoords[4].y = ch;
    vTexCoords[5].x = cx;        vTexCoords[5].y = ch;
    for(int i = 0;i < 6;i++) {
        vVertexBuffer[i].x /= (800 / 2);
        vVertexBuffer[i].y /= (600 / 2);

        if(bFlipV) {
            vTexCoords[i].y = 1.0f - vTexCoords[i].y;
        }
        if(bFlipU) {
            vTexCoords[i].x = 1.0f - vTexCoords[i].x;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, (6*2) * sizeof(float), &vVertexBuffer[0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[1]);
	glBufferData(GL_ARRAY_BUFFER, (6*2) * sizeof(float), &vTexCoords[0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[2]);
	glBufferData(GL_ARRAY_BUFFER, (6*4) * sizeof(float), &ColorBuffer[0], GL_DYNAMIC_DRAW);

	glBindVertexArray(vertArrObj);
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

void Eternal::Sprite::AmendToMesh(Rect &pos, Rect &clip, Mesh &mesh) {
    RGBA cols[6];
    AmendToMesh(pos,clip,mesh,cols);
}

void Eternal::Sprite::AmendToMesh(Rect &pos, Rect &clip, Mesh &mesh, RGBA cols[6]) {
    vVertexBuffer[0].x = pos.x;             vVertexBuffer[0].y = pos.y;
    vVertexBuffer[1].x = pos.x + pos.w;     vVertexBuffer[1].y = pos.y;
    vVertexBuffer[2].x = pos.x;             vVertexBuffer[2].y = pos.y + pos.h;

    vVertexBuffer[3].x = pos.x + pos.w;     vVertexBuffer[3].y = pos.y;
    vVertexBuffer[4].x = pos.x + pos.w;     vVertexBuffer[4].y = pos.y + pos.h;
    vVertexBuffer[5].x = pos.x;             vVertexBuffer[5].y = pos.y + pos.h;


    const float cx = clip.x / w;
    const float cy = clip.y / h;

    const float cw = (clip.x / w) + clip.w / w;
    const float ch = (clip.y / h) + clip.h / h;

    vTexCoords[0].x = cx;        vTexCoords[0].y = cy;
    vTexCoords[1].x = cw;        vTexCoords[1].y = cy;
    vTexCoords[2].x = cx;        vTexCoords[2].y = ch;

    vTexCoords[3].x = cw;        vTexCoords[3].y = cy;
    vTexCoords[4].x = cw;        vTexCoords[4].y = ch;
    vTexCoords[5].x = cx;        vTexCoords[5].y = ch;


/*    for(int i = 0;i < 6;i++) {
        mesh.Vert2(vVertexBuffer[i].x, vVertexBuffer[i].y);
        mesh.TexCoord2(vTexCoords[i].x, vTexCoords[i].y);
        mesh.Color4(cols[i].r, cols[i].g, cols[i].b, cols[i].a);
    }*/
}

bool Eternal::Sprite::IsLoaded() const {
    return bLoaded;
}