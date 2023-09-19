#ifndef SPRITE_H
#define SPRITE_H

#include "types.h"
#include <string>
#include <GL/glew.h>

namespace noise { namespace module { class Module; } }
namespace Eternal {
    class Mesh;
    class Sprite {
        public:
            Sprite();
            ~Sprite();

            void Load(std::string sfilename);
            void FromNoise(noise::module::Module &sourceModule, int width, int height);

            void Bind(int unit = 0);

            void Draw(float x, float y, float w, float h,
                        float cx, float cy, float cw, float ch);
            void Draw(Rect &pos, Rect &clip);

            void Draw_NoBind(Rect &pos, Rect &clip);
            void ForceResize(int width, int height);

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
            int Indices[6];

            std::string sName;
    };
}

#endif