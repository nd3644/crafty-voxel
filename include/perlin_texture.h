#ifndef PERLIN_TEXTURE_H
#define PERLIN_TEXTURE_H

#include <libnoise/noise.h>

class PerlinTexture {
    public:
        PerlinTexture();
        ~PerlinTexture();

        void FromModule(noise::module::Module *module);
    private:
};

#endif
