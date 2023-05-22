#ifndef SPRITE_H
#define SPRITE_H

#include "types.h"
#include <string>
#include <GL/glew.h>

namespace Eternal {
    class Mesh;
    class Sprite {
        public:
            Sprite();
            ~Sprite();

            void Load(std::string sfilename);
            void FromData(uint8_t *pixels, int width, int height, int bpp);
            
            void Bind(int unit = 0);
            void Draw(Rect &pos, Rect &clip);

            void Draw_NoBind(Rect &pos, Rect &clip);
            void ForceResize(int width, int height);

            void AmendToMesh(Rect &pos, Rect &clip, Eternal::Mesh &mesh);
            void AmendToMesh(Rect &pos, Rect &clip, Eternal::Mesh &mesh, RGBA cols[6]);

            int GetWidth() const { return w; }
            int GetHeight() const { return h; }

            void Flip(bool u, bool v) { bFlipU = u; bFlipV = v; }

            void SetColor(float r, float g, float b, float a);

            bool IsLoaded() const;

        private:
            bool bFlipU, bFlipV;
            void Cleanup();
            void ClearData();

            GLuint myTexID;
            int w, h;
            bool bLoaded;

            GLuint vertArrObj;
            GLuint arrayBuffers[4];

            vec2_t vVertexBuffer[6];
            vec2_t vTexCoords[6];
            RGBA ColorBuffer[6];

            std::string sName;
    };
}

#endif