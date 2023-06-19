#ifndef TEXTURE_ARRAY_H
#define TEXTURE_ARRAY_H

#include <GL/glew.h>
#include <vector>
#include <string>

class TextureArray {
    public:
        TextureArray();
        ~TextureArray();

        void Load(std::vector<std::string> files);
        void Bind();
    private:
        GLuint texArray;
};

#endif

