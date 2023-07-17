#ifndef TYPES_H
#define TYPES_H

#include <glm/glm.hpp>

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

    bool IsColliding(Rect &b, glm::vec2 &normal) {
        if (x > b.x + b.w
            || x + w < b.x
            || y > b.y + b.h
            || y + h < b.y) {
            return false;
        }


        // temporary translate to center origin
        x += w / 2;
        y += h / 2;
        
        b.x += b.w / 2;
        b.y += b.h / 2;

        normal.x = 0;
        normal.y = 0;

        glm::vec2 Distance;
        glm::vec2 absDistance;

        float XMagnitute;
        float YMagnitute;

        Distance.x = b.x - x;
        Distance.y = b.y - y;

        float XAdd = (b.w + w) / 2.0f;
        float YAdd = (b.h + h) / 2.0f;


        absDistance.x = (Distance.x < 0.0f) ? -Distance.x : Distance.x;
        absDistance.y = (Distance.y < 0.0f) ? -Distance.y : Distance.y;

        XMagnitute = XAdd - absDistance.x;
        YMagnitute = YAdd - absDistance.y;

        // check most significant overlap
        if (XMagnitute < YMagnitute) {
            normal.x = (Distance.x > 0) ? -XMagnitute : XMagnitute;
        }
        else {
            normal.y = (Distance.y > 0) ? -YMagnitute : YMagnitute;
        }

        // put back origin
        x -= w / 2;
        y -= h / 2;
        
        b.x -= b.w / 2;
        b.y -= b.h / 2;

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


    bool IsColliding(AABB b, glm::vec3 &normal) {
        if (x > b.x + b.w
            || x + w < b.x
            || y > b.y + b.h
            || y + h < b.y
            || z > b.z + b.d
            || z < b.z) {
            return false;
        }

        // temporary translate to center origin
        x += w / 2.0f;
        y += h / 2.0f;
        z += d / 2.0f;
        
        b.x += b.w / 2.0f;
        b.y += b.h / 2.0f;
        b.z += b.d / 2.0f;

        normal.x = 0.0f;
        normal.y = 0.0f;
        normal.z = 0.0f;

        glm::vec3 Distance;
        glm::vec3 absDistance;

        float XMagnitute;
        float YMagnitute;
        float ZMagnitude;

        Distance.x = b.x - x;
        Distance.y = b.y - y;
        Distance.z = b.z - z;

        float XAdd = (b.w + w) / 2.0f;
        float YAdd = (b.h + h) / 2.0f;
        float ZAdd = (b.d + d) / 2.0f;

        absDistance.x = (Distance.x < 0.0f) ? -Distance.x : Distance.x;
        absDistance.y = (Distance.y < 0.0f) ? -Distance.y : Distance.y;
        absDistance.z = (Distance.z < 0.0f) ? -Distance.z : Distance.z;

        XMagnitute = XAdd - absDistance.x;
        YMagnitute = YAdd - absDistance.y;
        ZMagnitude = ZAdd - absDistance.z;

        // check most significant overlap
        if (XMagnitute < YMagnitute && XMagnitute < ZMagnitude) {
            normal.x = (Distance.x > 0) ? -XMagnitute : XMagnitute;
        }
        else if(YMagnitute < XMagnitute && YMagnitute < ZMagnitude) {
            normal.y = (Distance.y > 0) ? -YMagnitute : YMagnitute;
        }
        else {
            normal.z = (Distance.z > 0) ? -ZMagnitude : ZMagnitude;
        }

        // put back origin
        x -= w / 2.0f;
        y -= h / 2.0f;
        z -= d / 2.0f;
        
        b.x -= b.w / 2.0f;
        b.y -= b.h / 2.0f;
        b.z -= b.d / 2.0f;

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

struct Triangle {
    public:
        vec2_t v[3];
};

struct Quad {
    public:
        Quad() {
            v[0].x = v[0].y = 0;
            v[1].x = v[1].y = 0;
            v[2].x = v[2].y = 0;
            v[3].x = v[3].y = 0;
        }
        Quad(float x, float y, float w, float h) {
            v[0].x = x;      v[0].y = y;
            v[1].x = x + w;  v[1].y = y;
            v[2].x = x + w;  v[2].y = y + h;
            v[3].x = x;      v[3].y = y + h;
        }

        void FromRect(Rect &r) {
            v[0].x = r.x;  v[0].y = r.y;
            v[1].x = r.x + r.w;  v[1].y = r.y;
            v[2].x = r.x + r.w;  v[2].y = r.y + r.h;
            v[3].x = r.x;  v[3].y = r.y + r.h;
        }

        vec2_t v[4];
};

struct plane_t {
    vec3_t position;
    vec3_t normal;
};

#endif
