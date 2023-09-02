#include "brick_ao.h"

brick_ao_t::brick_ao_t() {
    for(int f = 0;f < 6;f++) {
        for(int v = 0;v < 4;v++) {
            ambientVecs[f][v] = 100;
        }
    }
}

bool brick_ao_t::operator==(brick_ao_t &other) {
    for(int f = 0;f < 6;f++) {
        for(int v = 0;v < 4;v++) {
            if(other.ambientVecs[f][v] != ambientVecs[f][v]) {
                return false;
            }
        }
    }
    return true;
}

bool brick_ao_t::operator!=(brick_ao_t &other) {
    for(int f = 0;f < 6;f++) {
        for(int v = 0;v < 4;v++) {
            if(other.ambientVecs[f][v] != ambientVecs[f][v]) {
                return true;
            }
        }
    }
    return false;
}