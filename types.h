#ifndef TYPES_H
#define TYPES_H

struct vec2_t {
	float x, y;
};

struct vec3_t {
	float x, y, z;
};

struct Rect {
    Rect() {
        x = y = w = h = 0;
    }
    float x, y, w, h;
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

struct RGB {
    RGB() {
        r = g = b = 1.0f;
    }
    RGB(float r, float g, float b) {
        this->r = r;
        this->g = g;
        this->b = b;
    }
    float r;
    float g;
    float b;
};

#endif
