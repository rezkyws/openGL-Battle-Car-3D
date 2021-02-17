#ifndef BMPLOADER_H
#define BMPLOADER_H
#pragma once
#include <fstream>
#include <vector>
#include <stdexcept>
#include <iostream>
using namespace std;
#pragma pack(push, 1)
struct BITMAPFILEHEADER{
    uint16_t bfType{0x4D42};          // File type always BM which is 0x4D42
    uint32_t bfSize{0};               // Size of the file (in bytes)
    uint16_t bfReserved1{0};               // Reserved, always 0
    uint16_t bfReserved2{0};               // Reserved, always 0
    uint32_t bfOffBits{0};             // Start xPosition of pixel data (bytes from the beginning of the file)
};
struct BITMAPINFOHEADER {
    uint32_t biSize{ 0 };                      // Size of this header (in bytes)
    int32_t biWidth{ 0 };                      // width of bitmap in pixels
    int32_t biHeight{ 0 };                     // width of bitmap in pixels
    //       (if positive, bottom-up, with origin in lower left corner)
    //       (if negative, top-down, with origin in upper left corner)
    uint16_t biPlanes{ 1 };                    // No. of planes for the target device, this is always 1
    uint16_t biBitCount{ 0 };                 // No. of bits per pixel
    uint32_t biCompress{ 0 };               // 0 or 3 - uncompressed. THIS PROGRAM CONSIDERS ONLY UNCOMPRESSED BMP images
    uint32_t biSizeImage{ 0 };                // 0 - for uncompressed images
    int32_t biXPelsPerMeter{ 0 };
    int32_t biYPelsPerMeter{ 0 };
    uint32_t biClrUsed{ 0 };               // No. color indexes in the color table. Use 0 for the max number of colors allowed by bit_count
    uint32_t biClrImportant{ 0 };
};
#pragma pack(pop)
class BmpLoader{
public:
    unsigned char* textureData;
    int iWidth, iHeight;
    BmpLoader(const char*);
    ~BmpLoader();
private:
    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;
};

#endif
