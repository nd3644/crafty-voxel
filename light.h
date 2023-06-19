#ifndef LIGHT_H
#define LIGHT_H

#include "types.h"

struct light {
    light() {
        r = g = b = 1.0f;
    }
    float intensity;
    float r, g, b;
};

#endif
