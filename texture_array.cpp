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

    // TODO: Remove hardcodes
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_BGR, 16, 16, files.size());

    for(int i = 0;i < files.size();i++) {
        SDL_Surface *curSurf = IMG_Load(files[i].c_str());
        if(curSurf == nullptr) {
            std::cerr << "bad surface: " << files[i] << std::endl;
            exit(1);
        }

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, curSurf->w, curSurf->h,
                             1, GL_BGR, GL_UNSIGNED_BYTE, curSurf->pixels);
        
        SDL_FreeSurface(curSurf);
    }
}

void TextureArray::Bind() {
     glBindTexture(GL_TEXTURE_2D_ARRAY, texArray);
}