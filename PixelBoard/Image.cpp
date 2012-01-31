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


#pragma pack(1)
alignas(char) typedef struct {
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
        std::copy_n(std::istream_iterator<char>(tgaFile), headerSize, reinterpret_cast<char*>(headerBytes.data()));
    }
    
    auto start = begin(headerBytes);
    std::move(start, start+1, &header.id_len);
    std::move(start+1, start+2, &header.map_type);
    std::move(start+2, start+3, &header.img_type);
    std::move(start+3, start+5, &header.map_first);         header.map_first = CFSwapInt16LittleToHost(header.map_first);
    std::move(start+5, start+7, &header.map_len);           header.map_len = CFSwapInt16LittleToHost(header.map_len);
    std::move(start+7, start+8, &header.map_entry_size);    
    std::move(start+8, start+10, &header.x);                header.x = CFSwapInt16LittleToHost(header.x);
    std::move(start+10, start+12, &header.y);               header.y = CFSwapInt16LittleToHost(header.y);
    std::move(start+12, start+14, &header.width);           header.width = CFSwapInt16LittleToHost(header.width);
    std::move(start+14, start+16, &header.height);          header.height = CFSwapInt16LittleToHost(header.height);
    std::move(start+16, start+17, &header.bpp);             
    std::move(start+17, start+18, &header.descriptor);
    
    tgaFile.seekg(header.id_len, std::ios_base::cur); // forward past the id field
    
    if (header.map_type == 1)
    {
        tgaFile.seekg(header.map_len * header.map_entry_size, std::ios_base::cur); // again, forward past the colour map
    }
    
    switch (header.bpp)
    {
        case 24:
        {
            // load in 24 bit data
            std::vector<std::array<uint8_t,3>> data;
            size_t size = header.bpp >> 3 * header.width * header.height;
            data.reserve(size / 3);
            
            std::copy_n(std::istream_iterator<char>(tgaFile), size, reinterpret_cast<char*>(data.data()));
            
            // convert to 32 bit data
            std::vector<std::array<uint8_t,4>> data32;
            data.reserve(header.width * header.height);
            
            std::transform(std::begin(data), std::end(data), std::begin(data32),
                           [](std::array<uint8_t,3> in) -> decltype(std::array<uint8_t, 4>) {
                               std::array<uint8_t,4> out;
                               std::move(std::begin(in), std::end(in), std::begin(out));
                               out[3] = 255; // alpha value
                               return out;
                           });
            
            break;
        }
            
    }
    
    
}

void Image::loadTGA24(std::string &name)
{
    std::fstream f;
    targa_header p;
    
    if (_data.size() > 0)
        _data.clear();
    
    f.open(name, std::ios::in | std::ios::binary);
    
    if (f.good())
    {
        f.read(reinterpret_cast<char*>(&p), sizeof(p));
        p.map_first = CFSwapInt16LittleToHost(p.map_first);
        p.map_len = CFSwapInt16LittleToHost(p.map_len);
        p.x = CFSwapInt16LittleToHost(p.x);
        p.y = CFSwapInt16LittleToHost(p.y);
        p.width = CFSwapInt16LittleToHost(p.width);
        p.height = CFSwapInt16LittleToHost(p.height);
        
        f.seekg(p.id_len, std::ios_base::cur);  // forward past the id field
        if (p.map_type == 1)
        {
            f.seekg(p.map_len * p.map_entry_size, std::ios_base::cur); // forward past the colour map - we do not handle this            
        }
        
        // deal with the direction in the descriptor at some pointor
        
        if (p.bpp == 24 or p.bpp == 32)       // support 24 bit and 32 bit images
        {
            size_t image_size = p.width * p.height * (p.bpp >> 3);
            _data.resize(image_size);
            f.read((char*)&_data[0], image_size);
            
            // convert data from BGR888 to ABGR8888 if required
            if (p.bpp == 24)
            {
                size_t new_size = p.width * p.height * 4;
                
                for (size_t i = new_size; i > 0; ++i)
                {
                    
                }
            }
            
            //_data = new uint8_t[image_size];
            _good = true;
            
            _width = p.width;
            _height = p.height;
            _type = PixelFormat::RGBA8888;
        
            f.close();
            
            return;
        }
        f.close();
    }
    _good = false;
    return;
}

void Image::convertFormat(PixelFormat dest)
{
    if (_type == PixelFormat::RGBA8888 && dest == PixelFormat::ABGR4444)
    {
        // is data size right?
        if (_data.size() == _width * _height * 4)
        {
            std::cerr << "Converting from RGBA8888 to ABGR4444. Size of data is correct." << std::endl;
            
            for (int i = 0; i < _data.size() / 2; ++i)
            {
                _data[i] = (_data[2*i] >> 4) | (_data[2*i+1] & 0xF0);
            }
            
            _data.resize(_data.size() / 2);
            _type = PixelFormat::ABGR4444;
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
