#include "texture_array.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>


TextureArray::TextureArray() {

}

TextureArray::~TextureArray() {
    if(glIsTexture(texArray)) {
        glDeleteTextures(1, &texArray);
    }
}

void TextureArray::Load(std::vector<std::string> files) {
    
    glGenTextures(1, &texArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texArray);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // TODO: Remove hardcodes
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 4, GL_RGB8, 16, 16, files.size());

    for(size_t i = 0;i < files.size();i++) {
        SDL_Surface *curSurf = IMG_Load(files[i].c_str());
        if(curSurf == nullptr) {
            std::cerr << "bad surface: " << files[i] << std::endl;
            exit(1);
        }

         GLuint format = (curSurf->format->BytesPerPixel == 4) ? GL_RGBA : GL_RGB;
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, curSurf->w, curSurf->h,
                             1, format, GL_UNSIGNED_BYTE, curSurf->pixels);
        
        SDL_FreeSurface(curSurf);
    }
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << err << std::endl;
        exit(1);
    }
}

void TextureArray::Bind() {
     glBindTexture(GL_TEXTURE_2D_ARRAY, texArray);
}