//
//  Image.cpp
//  PixelBoard
//
//  Created by Jeffrey Drake on 12-01-30.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//


#include <CoreFoundation/CoreFoundation.h>

#include <cstdint>
#include <cstdio>

#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <iostream>
#include <iterator>
#include <array>


#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "Image.h"


typedef struct {
    uint8_t     id_len;                 // ID Field (Number of bytes - max 255)
    uint8_t     map_type;               // Colormap Field (0 or 1)
    uint8_t     img_type;               // Image Type (7 options - color vs. compression)
    uint16_t    map_first;              // Color Map stuff - first entry index
    uint16_t    map_len;                // Color Map stuff - total entries in file
    uint8_t     map_entry_size;         // Color Map stuff - number of bits per entry
    uint16_t    x;                      // X-coordinate of origin 
    uint16_t    y;                      // Y-coordinate of origin
    uint16_t    width;                  // Width in Pixels
    uint16_t    height;                 // Height in Pixels
    uint8_t    bpp;                    // Number of bits per pixel
    uint8_t    descriptor;              // image descriptor
                                        //    uint8_t     misc;                   // Other stuff - scan origin and alpha bits
} targa_header;

//static_assert(sizeof(targa_header)==18, "Targa Header is not 18 bytes");

auto RGB2RGBA(std::array<uint8_t,3> in) -> decltype(std::array<uint8_t,4>())
{
    std::array<uint8_t,4> out;
    std::move(std::begin(in), std::end(in), std::begin(out));
    out[3] = 255; // alpha value
    return out;
}

auto RGBA8888_RGBA4444(std::array<uint8_t,4> in) -> decltype(std::array<uint8_t,2>())
{
    std::array<uint8_t,2> out;
    out[0] = (in[1] >> 4) | (in[0] & 0xF0);
    out[1] = (in[3] >> 4) | (in[2] & 0xF0);
    return out;
}

auto RGBA8888_ABGR4444(std::array<uint8_t,4> in) -> decltype(std::array<uint8_t,2>())
{   // out[0] low = alpha  out[0] high = blue
    // out[1] low = green  out[1] high = red
    std::array<uint8_t,2> out;
    out[0] = (in[2] & 0XF0) | (in[3] >> 4);
    out[1] = (in[1] & 0xF0) | (in[0] >> 4); //| (in[3] & 0xF0);
    return out;
}

//

void Image::loadTGA(std::string const &name)
{
    const size_t headerSize = 18;
    std::array<uint8_t, headerSize> headerBytes;
    std::ifstream tgaFile;
    targa_header header;
    
    if (_good)
    {
        _data.clear();
    }
    
    tgaFile.open(name, std::ios::in | std::ios::binary);
    
    if (tgaFile.good())
    {
        tgaFile.read((char*)headerBytes.data(), headerSize);
        //std::copy_n(std::istream_iterator<char>(tgaFile), headerSize, reinterpret_cast<char*>(headerBytes.data()));
    }
    
    for (int i = 0; i < headerSize; ++i)
        std::cerr << "item " << i << " is " << (int)headerBytes[i] << std::endl;

    header.id_len = headerBytes[0];
    header.map_type = headerBytes[1];
    header.img_type = headerBytes[2];
    header.map_first = headerBytes[3] | (headerBytes[4] << 8);
    header.map_len = headerBytes[5] | (headerBytes[6] << 8);
    header.map_entry_size = headerBytes[7];
    header.x = headerBytes[8] | (headerBytes[9] << 8);
    header.y = headerBytes[10] | (headerBytes[11] << 8);
    header.width = headerBytes[12] | (headerBytes[13] << 8);
    header.height = headerBytes[14] | (headerBytes[15] << 8);
    header.bpp = headerBytes[16];
    header.descriptor = headerBytes[17];
    
    
    tgaFile.seekg(header.id_len, std::ios_base::cur); // forward past the id field
    
    if (header.map_type == 1)
    {
        tgaFile.seekg(header.map_len * header.map_entry_size, std::ios_base::cur); // again, forward past the colour map
    }
    
    switch (header.bpp)
    {
        case 24:
        {
            typedef std::array<uint8_t, 4> RGBA;
            typedef std::array<uint8_t, 3> RGB;
            
            // load in 24 bit data
            size_t size = (header.bpp >> 3) * header.width * header.height;
            std::vector<RGB> data24(size / 3);
            //data24.resize(data24.capacity());

            tgaFile.read(reinterpret_cast<char*>(data24.data()), size);
            //            std::copy_n(std::istream_iterator<char>(tgaFile), size, reinterpret_cast<char*>(data.data()));
            
            // convert to 32 bit data
            std::vector<RGBA> data32(header.width * header.height);
            //data32.resize(data32.capacity());
            
            std::cerr << "data's data: " << data24.data() << " size: " << data24.size() << std::endl;
            std::cerr << "data32's data: " << data32.data() << " size: " << data32.size() << std::endl;
            
            std::transform(std::begin(data24), std::end(data24), std::begin(data32), RGB2RGBA);

            _data.resize(data32.size() * sizeof(std::array<uint8_t,4>));
            
            std::move(reinterpret_cast<uint8_t*>(data32.data()), reinterpret_cast<uint8_t*>(data32.data()) + data32.size() * sizeof(RGBA),
                      _data.begin());
            
            _good = true;
            _width = header.width;
            _height = header.height;
            _type = RGBA8888;
            
            break;
        }
        case 32:
        {
            // load in 32 bit data
            size_t size = header.width * header.height * 4;
            _data.reserve(size);
            std::copy_n(std::istream_iterator<char>(tgaFile), size, _data.begin());
            
            _good = true;
            _width = header.width;
            _height = header.height;
            _type = RGBA8888;
            break;
        }
        default:
        {
            std::cerr << "TGA file format not supported." << std::endl;
            _good = false;
        }   
    }
    
    tgaFile.close();
}

void Image::convertFormat(PixelFormat dest)
{
    if (_type == PixelFormat::RGBA8888 && dest == PixelFormat::ABGR4444)
    {
        // is data size right?
        if (_data.size() == _width * _height * 4)
        {
            std::cerr << "Converting from RGBA8888 to ABGR4444. Size of data is correct." << std::endl;
            
            typedef std::array<uint8_t, 4> RGBA8888;
            typedef std::array<uint8_t, 2> RGBA4444;
            
            RGBA8888 *data32 = reinterpret_cast<RGBA8888*>(_data.data());
            
            std::cerr << "Pixel 0 @ 32: " << (int)data32[0][0] << ", " << (int)data32[0][1] << ", " << (int)data32[0][2] << ", " << (int)data32[0][3] << std::endl;
            
            std::vector<RGBA4444> data16(_width * _height);
            
            std::transform(data32, data32 + _width * _height, std::begin(data16), RGBA8888_ABGR4444);
            
            _data.clear();
            _data.reserve(_width * _height * 2);
            std::move(reinterpret_cast<uint8_t*>(data16.data()), reinterpret_cast<uint8_t*>(data16.data()) + _width * _height * 2,
                      std::begin(_data));
            
            std::cerr << "Pixel 0 @ 16: " << (int)((_data[0] & 0xf0) >> 4) << ", " << (int)(_data[0] & 0x0f) << ", " 
            << (int)((_data[1] & 0xf0) >> 4) << ", " << (int)(_data[1] & 0x0f) << std::endl;
            
            _type = PixelFormat::ABGR4444;
            
            return;
        }
        else
        {
            std::cerr << "Converting from RGBA8888 to BGRA4444. Size of data is not correct, should be " 
                      << _width * _height * 4 << " but is " << _data.size() << std::endl;
        }
    }
}

void Image::bufferTexture()
{
    if (_type == PixelFormat::ABGR4444)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, _data.data());
    }
    
}

#if 0
void Image::saveTGA24(std::string &name)
{
    
}



void writeheader(targa_header h, FILE *tga);
void readheader(targa_header *h, FILE *tga);
void writeheader(targa_header h, FILE *tga) 
{
    fputc(h.id_len, tga);          // Write chars for ID, map, and image type
    fputc(h.map_type, tga);
    fputc(h.img_type, tga);
    fputc(h.map_first % 256, tga); // Write integer, low order byte first
    fputc(h.map_first / 256, tga); // Write second byte of integer, high order
    fputc(h.map_len % 256, tga);   // Another integer 
    fputc(h.map_len / 256, tga);
    fputc(h.map_entry_size, tga);  // Write a char - only one byte
    fputc(h.x % 256, tga);         // More integers
    fputc(h.x / 256, tga);
    fputc(h.y % 256, tga);
    fputc(h.y / 256, tga);
    fputc(h.width % 256, tga);     // Even more integers
    fputc(h.width / 256, tga);
    fputc(h.height % 256, tga);
    fputc(h.height / 256, tga);
    fputc(h.bpp, tga);             // Write two chars
    fputc(h.misc, tga);
}

void readheader(targa_header *h, FILE *tga)
{
    h->id_len = fgetc(tga);
    h->map_type = fgetc(tga);
    h->img_type = fgetc(tga);
    h->map_first = fgetc(tga) | (fgetc(tga) << 8);
    h->map_len = fgetc(tga) | (fgetc(tga) << 8);
    h->map_entry_size = fgetc(tga);
    h->x = fgetc(tga) | (fgetc(tga) << 8);
    h->y = fgetc(tga) | (fgetc(tga) << 8);
    h->width = fgetc(tga) | (fgetc(tga) << 8);
    h->height = fgetc(tga) | (fgetc(tga) << 8);
    h->bpp = fgetc(tga);
    h->misc = fgetc(tga);
}

//typedef struct { unsigned red : 4; unsigned green : 4; unsigned blue : 4; unsigned alpha : 4;} pixel_type;

//std::vector<pixel_type> _image;

void writeTGA(const char *filename, std::vector<PixelPlane::pixel_type> image)
{
    FILE *tga;               // Pointer to a FILE
    targa_header header;     // Variable of targa_header type
    int x, y;
    
    /* First, set all the fields in the header to appropriate values */
    header.id_len = 0;          /* no ID field */
    header.map_type = 0;        /* no colormap */
    header.img_type = 2;        /* trust me */
    header.map_first = 0;       /* not used */
    header.map_len = 0;         /* not used */
    header.map_entry_size = 0;  /* not used */
    header.x = 0;               /* image starts at (0,0) */
    header.y = 0;
    header.width = 320;         /* image is 200 x 100 */
    header.height = 240;
    header.bpp = 24;            /* 24 bits per pixel */
    header.misc = 0x20;         /* scan from upper left corner */
    
    /* Open a file for writing targa data.  Call the file "test.tga" and
     write in binary mode (wb) so that nothing is lost as characters
     are written to the file */
    
    tga = fopen(filename, "wb"); /* Write the header information  */
    
    writeheader(header, tga);  
    
    /* Write the data for a graphic that measures 100 by 200 pixels. */
    
    for(y = 0; y < 240; y++)      // Create 100 Rows of Pixels 
        for(x = 0; x < 320; x++)   // Create 200 Pixels in each Row
            
        { /* For each pixel, write a character representing the RGB color.
           Notice that the order that is written to the file is B-G-R.
           This sequence just cycles through the colors in some pattern
           but all char values must be integers between 0 and 255. */
            PixelPlane::pixel_type p = image[y * 320 + x];
            int r = (p.red / 15.0) * 255.0;
            int g = (p.green / 15.0) * 255.0;
            int b = (p.blue / 15.0) * 255.0;
            
            fputc(b % 256, tga);
            fputc(g % 256, tga);
            fputc(r % 256, tga);
            //            fputc((4*y)%256, tga);               // Write char for BLUE            
            //            fputc(255 - (x*2 + y*2)%256, tga);   // Write char for GREEN
            //fputc((3*x)%256, tga);               // Write char for RED
        }
    
    /* close the file */
    fclose(tga);
    
    /* that was easy, right? */
}

std::vector<PixelPlane::pixel_type> readTGA(const char *filename)
{
    FILE *tga;
    targa_header header;
    int x, y;
    
    memset(&header, 0, sizeof(header));
    
    tga = fopen(filename, "rb");
    readheader(&header, tga);
    
    std::vector<PixelPlane::pixel_type> pixs;
    pixs.reserve(header.width * header.height);
    
    for (y = 0; y < header.height; ++y)
    {
        for (x = 0; x < header.width ; ++x)
        {
            int b = fgetc(tga);
            int g = fgetc(tga);
            int r = fgetc(tga);
            
            PixelPlane::pixel_type p;
            p.red = (r / 255.0) * 15;
            p.green = (g / 255.0) * 15;
            p.blue = (b / 255.0) * 15;
            p.alpha = 15;
            
            pixs[y * header.width + x] = p;
        }
    }
    
    fclose(tga);
    
    return pixs;
}
#endif
