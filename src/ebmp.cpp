#include "ebmp.h"
#include <fstream>
#include <iostream>

Eternal::Bmp::Bmp(std::string const &sfilename) {
    PixelData = nullptr;
    Load(sfilename);
}

Eternal::Bmp::Bmp() {
    PixelData = nullptr;
}

Eternal::Bmp::~Bmp() {
    if(PixelData != nullptr) {
        delete [] PixelData;
    }
}

int Eternal::Bmp::Load(std::string const &sfilename) {

    if(PixelData != nullptr) {
        delete [] PixelData;
    }

    bmp_file_header myHeader;
    std::ifstream fileHandle(sfilename, std::ios::in | std::ios::binary);
    if(!fileHandle.is_open()) {
        sError = "File error";
        return 1;
    }
    std::cout << "name: " << sfilename << std::endl;
    fileHandle.read((char *)&myHeader.identifier, 2);
    fileHandle.read((char *)&myHeader.i_size, 4);
    fileHandle.read((char *)&myHeader.asdf, 4);
    fileHandle.read((char *)&myHeader.i_dataoffset, 4);

    if(myHeader.identifier[0] != 0x42 || myHeader.identifier[1] != 0x4D) {
        sError = "Magic number invalid";
        return 1;
    }

    uint32_t infoHeaderSize = 0;

    fileHandle.read((char *)&myInfoHeader.size, sizeof(myInfoHeader.size));

    fileHandle.read((char *)&myInfoHeader.img_w, sizeof(myInfoHeader.img_w));
    fileHandle.read((char *)&myInfoHeader.img_h, sizeof(myInfoHeader.img_h));
    fileHandle.read((char *)&myInfoHeader.colorplanes, sizeof(myInfoHeader.colorplanes));
    fileHandle.read((char *)&myInfoHeader.bpp, sizeof(myInfoHeader.bpp));
    fileHandle.read((char *)&myInfoHeader.compression, sizeof(myInfoHeader.compression));
    if(myInfoHeader.compression != 0) {
        sError = "Compression unsupported";
    }
    fileHandle.read((char *)&myInfoHeader.data_size, sizeof(myInfoHeader.data_size));
    if(myInfoHeader.data_size == 0) {
        printf("yah!");
        myInfoHeader.data_size = myInfoHeader.img_w * myInfoHeader.img_h;
    }
    fileHandle.read((char *)&myInfoHeader.horiz_res, sizeof(myInfoHeader.horiz_res));
    fileHandle.read((char *)&myInfoHeader.vert_res, sizeof(myInfoHeader.vert_res));
    fileHandle.read((char *)&myInfoHeader.palette_length, sizeof(myInfoHeader.palette_length));
    fileHandle.read((char *)&myInfoHeader.important_color_count, sizeof(myInfoHeader.important_color_count));

/*  std::cout << "size totals: " << myInfoHeader.size << std::endl;
    std::cout << "w/h: " << (int)myInfoHeader.img_w << " " << (int)myInfoHeader.img_h << std::endl;
    std::cout << "pl: " << (int)myInfoHeader.palette_length << std::endl;
    std::cout << "bpp: " << myInfoHeader.bpp << std::endl;
    std::cout << "comp: " << myInfoHeader.compression << std::endl; */

    if(myInfoHeader.palette_length != 0) {
        for(int i = 0;i < myInfoHeader.palette_length;i++) {
            fileHandle.read((char*)&Palette[i].R, sizeof(uint8_t));
            fileHandle.read((char*)&Palette[i].G, sizeof(uint8_t));
            fileHandle.read((char*)&Palette[i].B, sizeof(uint8_t));
            fileHandle.read((char*)&Palette[i].A, sizeof(uint8_t));
        }
    }

    unsigned char temp[4];
    PixelData = new RGBComponent[myInfoHeader.img_w * myInfoHeader.img_h];
    bytes = new uint8_t[myInfoHeader.img_w * myInfoHeader.img_h];
    fileHandle.seekg(myHeader.i_dataoffset, std::ios::beg);

    const int w = myInfoHeader.img_w;
	const int h = myInfoHeader.img_h;
    if(myInfoHeader.bpp == 8) {
        for(int i = 0;i < myInfoHeader.data_size;i++) {
            fileHandle.read((char*)&temp,1);


            PixelData[i].R = temp[0];
        }
    }
    else if(myInfoHeader.bpp == 24) {
        for(int i = 0;i < w*h;i++) {
            fileHandle.read((char*)&temp,1);
            PixelData[i].R = temp[0];
            fileHandle.read((char*)&temp,1);
            PixelData[i].G = temp[0];
            fileHandle.read((char*)&temp,1);
            PixelData[i].B = temp[0];   
        }
    }
    else if(myInfoHeader.bpp == 32) {
        for(int i = 0;i < w*h;i++) {            
            fileHandle.read((char*)&temp,1);
            PixelData[i].R = temp[0];
            fileHandle.read((char*)&temp,1);
            PixelData[i].G = temp[0];
            fileHandle.read((char*)&temp,1);
            PixelData[i].B = temp[0];  
            fileHandle.read((char*)&temp,1);
            PixelData[i].A = temp[0]; 
        }
    }
    else {
        sError = "bitmap is not 8/24/32 bpp";
        return 1;
    }
    
    sError.clear();
    return 0;
}

int Eternal::Bmp::GetWidth() const {
    return myInfoHeader.img_w;
}

int Eternal::Bmp::GetHeight() const {
    return myInfoHeader.img_h;
}

Eternal::RGBComponent Eternal::Bmp::GetPixelRGBA(int x, int y) const {
    if(myInfoHeader.bpp == 8) {
        return Palette[PixelData[((myInfoHeader.img_h - y) * myInfoHeader.img_w) + x].R];
    }
    else {
        return PixelData[((myInfoHeader.img_h - y) * myInfoHeader.img_w) + x];
    }
}