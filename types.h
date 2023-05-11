#ifndef TYPES_H
#define TYPES_H

struct vec2_t {
	float x, y;
};

struct vec3_t {
	float x, y, z;
};

struct RGBA {
        RGBA() {
            r = g = b = a = 1.0f;
        }
        RGBA(float r, float g, float b, float a) {
            this->r = r;
            this->g = g;
            this->b = b;
            this->a = a;
        }
        float r;
        float g;
        float b;
        float a;
    };

#endif
