#ifndef CLOUD_SYSTEM_H
#define CLOUD_SYSTEM_H

#include <stdlib.h>
#include <vector>
#include "camera.h"

class CloudSystem {
    public:
        struct cloud_t {
            cloud_t() : x(0), z(0) {
                w = (rand() % 32) + 12;
                l = (rand() % 32) + 12;
            };
            float x, z;
            float w, l;
        };

        CloudSystem();
        ~CloudSystem();

        void Update(Camera &cam);
        void Draw();
    private:
        std::vector<cloud_t>myClouds;
};

#endif
