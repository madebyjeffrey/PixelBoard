//
//  Image.h
//  PixelBoard
//
//  Created by Jeffrey Drake on 12-01-30.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef PixelBoard_Image_h
#define PixelBoard_Image_h

enum PixelFormat
{
    None,
    ABGR4444,
    RGBA8888,
    RGBA5551
};

class Image
{
private:
    bool _good;
    unsigned _width, _height;
    std::vector<uint8_t> _data;
    PixelFormat _type;
    
    
    //void saveTGA24(std::string &name);
public:
    Image() : _good(false), _width(0), _height(0), _data() { }
    
    
    enum FileFormat
    {
        TGA24, TGA32
    };
    
    void loadTGA(std::string const &name);
    
    void loadTGA24(std::string &name);
    void convertFormat(PixelFormat dest);
    void bufferTexture();
    //    saveFile(FileFormat typ, std::string name);
    //loadFile(FileFormat typ, std::string name);
};




#endif
