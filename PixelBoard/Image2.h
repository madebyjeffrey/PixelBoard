//
//  Image2.h
//  PixelBoard
//
//  Created by Jeffrey Drake on 12-01-31.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef PixelBoard_Image2_h
#define PixelBoard_Image2_h

#include <cstdint>
#include <vector>
#include <string>
#include <array>
#include <fstream>
#include <iostream>
#include <type_traits>

struct RGBA8888
{
    typedef uint32_t PixelType;
    typedef std::vector<uint32_t> ContainerType;
    
    static uint8_t red(PixelType &p) { return (p & 0x000000ff) >> 0;} 
    static uint8_t blue(PixelType &p) { return (p & 0x0000ff00) >> 8;} 
    static uint8_t green(PixelType &p) { return (p & 0x00ff0000) >> 16;} 
    static uint8_t alpha(PixelType &p) { return (p & 0xff000000) >> 24;}  
};

struct RGB565
{
    //   RRRRR GGGGGG BBBBB
    //   0     5      11
    typedef uint16_t PixelType;
    typedef std::vector<uint16_t> ContainerType;
    
    static uint8_t red(PixelType &p) { return (p & 0x01F) >> 0;} 
    static uint8_t blue(PixelType &p) { return (p & 0x7E0) >> 5;} 
    static uint8_t green(PixelType &p) { return (p & 0xF800) >> 11;} 
};


std::vector<RGBA8888::PixelType> transmogrify(std::vector<unsigned char> &p);


template <typename Format>
class Image
{
private:
    std::vector<typename Format::PixelType> _pixels;
    size_t _width, _height, _bpp;
    
public:
    std::vector<typename Format::PixelType> &data() 
    {
        return _pixels;
    }
    
    size_t width() const { return _width; }
    size_t height() const  { return _height; }
    size_t bpp() const { return _bpp; }
    
    Image() 
    : _pixels(),
      _width(0),
      _height(0),
      _bpp(0)
    {
        
    }
    
    Image(Image const &copy)
    : _pixels(copy._pixels),
      _width(copy._width),
      _height(copy._height),
      _bpp(copy._bpp)
    {
        
    }
    
    Image(Image &&move)
    : _pixels(move._pixels),
      _width(move._width),
      _height(move._height),
      _bpp(move._bpp)
    {
        
    }
    
    
    Image(size_t width, size_t height)
    : _pixels(width * height * sizeof(Format::PixelType)),
      _width(width),
      _height(height),
      _bpp(sizeof(Format::PixelType << 3))
    {
        
    }
    
    Image(size_t width, size_t height, std::vector<typename Format::PixelType> &pdata)
    : _pixels(width * height * sizeof(Format::PixelType >> 3)),
      _width(width),
      _height(height),
      _bpp(sizeof(Format::PixelType << 3))
    {
        std::copy(std::begin(pdata), std::end(pdata), std::begin(_pixels));
    }
    
    Image(size_t width, size_t height, std::vector<typename Format::PixelType> &&pdata)
    : _pixels(width * height * sizeof(Format::PixelType >> 3)),
      _width(width),
      _height(height),
      _bpp(sizeof(Format::PixelType << 3))
    {
        std::move(std::begin(pdata), std::end(pdata), std::begin(_pixels));
    }
};

template <typename PixelType>
typename std::enable_if<std::is_same<PixelType, RGB565>::value, PixelType>::PixelType
    pixel_cast(RGBA8888::PixelType &source)
{

    RGB565::PixelType pix = (PixelType::red(source) >> 3) |
                            (PixelType::green(source) >> 2) << 5 |
                            (PixelType::blue(source) >> 3) << 11;
    return pix;
}
           

template <typename PixelType>
typename std::enable_if<std::is_same<PixelType, RGB565>::value, PixelType>::ContainerType
    image_cast(Image<RGBA8888> &source)
{
    Image<PixelType> p(source.width(), source.height());
    RGBA8888::ContainerType &src = source.data();
    typename PixelType::ContainerType &dst = p.data();
    
    std::transform(std::begin(src), std::end(src), std::begin(dst), pixel_cast<PixelType>);
    
    return p;
}

template <typename PixelType>
bool bufferTexture(typename std::enable_if<std::is_same<PixelType, RGB565>::value, PixelType>::ContainerType &source)
{
    return true;
}



#endif
