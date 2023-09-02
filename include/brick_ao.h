#ifndef BRICK_AO_H
#define BRICK_AO_H

#include <cstdint>

struct brick_ao_t {
    brick_ao_t();
    uint8_t ambientVecs[6][4];
    bool operator==(brick_ao_t &other);
    bool operator!=(brick_ao_t &other);
};

#endif
