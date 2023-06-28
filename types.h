#ifndef TYPES_H
#define TYPES_H

struct vec2_t {
    vec2_t() : x(0), y(0) { }
    vec2_t(float xpos, float ypos) : x(xpos), y(ypos) { }
	float x, y;
};

struct vec3i_t {
    int x, y, z;
};

struct vec3_t {
    vec3_t() : x(0), y(0), z(0) { }
    vec3_t(float xpos, float ypos, float zpos) : x(xpos), y(ypos), z(zpos) {}
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

struct AABB {
    AABB() : x(0), y(0), z(0), w(0), h(0), d(0) { }
    AABB(float sx, float sy, float sz, float sw, float sh, float sd) : x(sx), y(sy), z(sz), w(sw), h(sh), d(sd) { }
    float x, y, z;
    float w, h, d;

    bool IsColliding(AABB &b) {
        if (x > b.x + b.w
            || x + w < b.x
            || y > b.y + b.h
            || y + h < b.y
            || z > b.z + b.d
            || z < b.z) {
            return false;
        }
        return true;
    }
};

struct RGBA {
    RGBA() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) { }
    RGBA(float red, float green, float blue, float alpha) : r(red), g(green), b(blue), a(alpha) { }
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
