#ifndef MESH_H
#define MESH_H

#include "types.h"
#include <GL/glew.h>
#include <vector>

struct special_t {
    float f[16];
};

class Mesh {
    public:
        Mesh();
        ~Mesh();

        enum Mode : GLenum {
            MODE_PONTS = GL_POINTS,
            MODE_TRIANGLES = GL_TRIANGLES
        };


        void Clean();
        void Vert3(float x, float y,float z);
        void TexCoord2(float x, float y);
        void Color4(float r, float g, float b, float a);
        void Draw(Mode mode);

        void SetTranslation(float x, float y, float z);
    private:
        float xTrans, yTrans, zTrans;
        GLuint vertArrObj;
        GLuint arrayBuffers[3];

        std::vector<vec3_t>vVertBuffer; // Vec2 vVertexBuffer[6];
        std::vector<vec2_t>vTexCoords; // Vec2 Vec2 vTexCoords[6];
        std::vector<RGBA>ColorBuffer; // RGBA ColorBuffer[6];
};

#endif