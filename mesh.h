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
            MODE_POINTS = GL_POINTS,
            MODE_TRIANGLES = GL_TRIANGLES
        };

        void Clean();
        void Vert3(float x, float y,float z);
        void TexCoord2(float x, float y);
        void Color4(float r, float g, float b, float a);
        void Index1(int i);
        void Draw(Mode mode);

        void SetTranslation(float x, float y, float z);

        bool IsEmpty() {
            return (vVertBuffer.size() == 0);
        }
    private:
        float xTrans, yTrans, zTrans;
        GLuint vertArrObj;
        GLuint arrayBuffers[4];

        std::vector<vec3_t>vVertBuffer; 
        std::vector<vec2_t>vTexCoords;
        std::vector<RGBA>ColorBuffer;
        std::vector<int>Indices;
};

#endif