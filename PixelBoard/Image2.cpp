//
//  Image2.cpp
//  PixelBoard
//
//  Created by Jeffrey Drake on 12-01-31.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>

#include "Image2.h"

std::vector<RGBA8888::PixelType> transmogrify(std::vector<unsigned char> &p)
{
    if (p.size() % 4 == 0)
    {
        auto i = std::begin(p);
        std::vector<RGBA8888::PixelType> out(p.size() / 4);
        auto outi = std::begin(out);
        
        
        while (i != std::end(p))
        {
            *outi = *i | *i++ << 8 | *i++ << 16 | *i++ << 24;
            
            ++outi;
        }
        
        return out;
    }
    
    return std::vector<RGBA8888::PixelType>();
}

#if 0


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

template <typename Format>
Image<Format> loadTarga(const std::string &name)
{
    const size_t headerSize = 18;
    std::array<uint8_t, headerSize> headerBytes;
    std::ifstream tgaFile;
    targa_header header;
    
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
            Image<RGB888> image;
            
            std::vector<RGB888::PixelType> ledata(header.width * header.height);
            
            for (auto i = le
                 
                 }
                 }
                 
                 
                 }
                 
#endif
                 
#if 0
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
                 
                 
#endif
