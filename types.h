#ifndef TYPES_H
#define TYPES_H

struct vec2_t {
	float x, y;
};

struct vec3i_t {
    int x, y, z;
};

struct vec3_t {
	float x, y, z;
};

struct Rect {
    Rect() {
        x = y = w = h = 0;
    }
    Rect(float x, float y, float w, float h) {
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
    }
    bool IsColliding(Rect &b) {
        if (x > b.x + b.w
            || x + w < b.x
            || y > b.y + b.h
            || y + h < b.y) {
            return false;
        }
        return true;
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
