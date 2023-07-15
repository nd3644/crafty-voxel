#ifndef CHUNK_MESH_H
#define CHUNK_MESH_H

#include "types.h"
#include <GL/glew.h>
#include <vector>
#include <iostream>

class ChunkMesh {


public:
    struct triangle_t {
        public:
        vec3_t v[3]; // verts
        vec3_t c[3]; // color
        vec2_t t[2]; // texcoords
        int i[3];       // indices
    };
        ChunkMesh();
        ~ChunkMesh();

        enum Mode : GLenum {
            MODE_POINTS = GL_POINTS,
            MODE_TRIANGLES = GL_TRIANGLES
        };

        void Clean();

        inline void Triangle(triangle_t &t) {
            Triangles.emplace_back(t);
        }
/*        inline void Vert3(float x, float y,float z) {
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
*/
        void Draw(Mode mode);

        void SetTranslation(float x, float y, float z) {
            xTrans = x;
            yTrans = y;
            zTrans = z;
        }

        bool IsEmpty() {
            return (Triangles.size() == 0);
        }

        void CheckErr() {
            return;
            if (Triangles.empty())
            {
                std::cout << "There is a serious problem" << std::endl;
                exit(-1);
                return;
            }
            else {
                std::cout << "a serious problem" << std::endl;
                exit(-1);
            }
        }

        void BindBufferData();
    private:
        bool bDataIsBound;
        float xTrans, yTrans, zTrans;
        GLuint vertArrObj;
        GLuint arrayBuffers[4];
        std::vector<triangle_t>Triangles;

/*        std::vector<vec3_t>vVertBuffer; 
        std::vector<vec2_t>vTexCoords;
        std::vector<RGBA>ColorBuffer;
        std::vector<int>Indices;*/
};

#endif