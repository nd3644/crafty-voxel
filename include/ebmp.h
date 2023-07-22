#ifndef EBMP_H
#define EBMP_H

#include <string>
#include <stdint.h>

namespace Eternal {
    struct RGBComponent {
        RGBComponent() {
            R = G = B = 0;
        }
        uint8_t R, G, B, A;
    };

    struct bmp_file_header {
        uint8_t identifier[2]; // 2
        uint32_t i_size;         // 4 = 6
        uint32_t asdf;
        uint32_t i_dataoffset; // 4 = 14
    };

    struct BITMAPINFOHEADER {
        uint32_t size;
        int32_t img_w;
        int32_t img_h;
        uint16_t colorplanes;
        uint16_t bpp;
        uint32_t compression;
        uint32_t data_size;
        uint32_t horiz_res;
        uint32_t vert_res;
        uint32_t palette_length;
        uint32_t important_color_count;
    };


    class Bmp {
        public:
            Bmp();
            Bmp(std::string const &sfilename);
            ~Bmp();

            int Load(std::string const &sfilename);

            int GetWidth() const;
            int GetHeight() const;

            RGBComponent GetPixelRGBA(int x, int y) const;

            std::string GetError() const {
                return sError;
            }
        private:
            RGBComponent Palette[256];
            RGBComponent *PixelData;
            uint8_t *bytes;
            BITMAPINFOHEADER myInfoHeader;
            std::string sError;
    };
}

#endif