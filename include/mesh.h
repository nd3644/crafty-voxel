#ifndef MESH_H
#define MESH_H

#include "types.h"
#include <GL/glew.h>
#include <vector>
#include <iostream>

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
        inline void Vert3(float x, float y,float z) {
            vVertBuffer.emplace_back(x + xTrans, y + yTrans, z + zTrans);
        }

        inline void TexCoord2(float x, float y) {
            vTexCoords.emplace_back(x,y);
        }

        inline void Color4(float r, float g, float b, float a) {    
            ColorBuffer.emplace_back(r,g,b,a);
        }
        inline void Index1(int i) {
            Indices.emplace_back(i);
        }

        void Draw(Mode mode);

        void SetTranslation(float x, float y, float z) {
            xTrans = x;
            yTrans = y;
            zTrans = z;
        }

        bool IsEmpty() {
            return (vVertBuffer.size() == 0);
        }

        void CheckErr() {
            return;
            if (vVertBuffer.empty() || vTexCoords.empty() || ColorBuffer.empty() || Indices.empty())
            {
                std::cout << "There is a serious problem" << std::endl;
                exit(-1);
                return;
            }
            if(vVertBuffer.size() == vTexCoords.size() && vTexCoords.size() == ColorBuffer.size() && ColorBuffer.size() == Indices.size()) {
            }
            else {
                std::cout << "a serious problem" << std::endl;
                exit(-1);
            }
        }

        void BindBufferData();
    private:
        bool bDataIsBound;
        bool bInit;
        float xTrans, yTrans, zTrans;
        GLuint vertArrObj;
        GLuint arrayBuffers[4];

        std::vector<vec3_t>vVertBuffer; 
        std::vector<vec2_t>vTexCoords;
        std::vector<RGBA>ColorBuffer;
        std::vector<int>Indices;
};

#endif