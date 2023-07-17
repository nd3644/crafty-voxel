#ifndef RENDERER_H
#define RENDERER_H

#include "types.h"
#include <GL/glew.h>

namespace Eternal {
    extern int WIN_W, WIN_H;
    class Renderer {
        public:
            Renderer();
            ~Renderer();


            bool IsInitialized() const;
            void Initialize();

            void SetColor(float r, float g, float b, float a);
            void DrawQuad(Quad &quad);
            void DrawQuad(Rect r);
            void DrawBox(Quad &quad);
            void DrawBox(Rect r);
            void DrawTriangle(Triangle &triangle);
            void DrawLine(float sx, float sy, float fx, float fy);
			void PlotPoint(float x, float y);

        private:
            GLuint vertArrObj;
            GLuint arrayBuffers[3];

            vec2_t vVertexBuffer[6];
            RGBA ColorBuffer[6];
            float ftexcoords[6 * 2];
            bool bInitialized;
    };
}

#endif
