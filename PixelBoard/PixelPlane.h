//
//  PixelPlane.h
//  PixelBoard
//
//  Created by Jeffrey Drake on 12-01-29.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef PixelBoard_PixelPlane_h
#define PixelBoard_PixelPlane_h

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include <array>
#include <vector>

class PixelPlane
{
private:
    GLuint _program;
    GLuint _vertexBuffer;
    GLuint _texture;
    
    GLuint _frameBuffer, _renderBuffer;
    int _width, _height;
    int _vwidth, _vheight;
    
    
public:
    //typedef std::array<GLfloat, 4> pixel_type;
    typedef struct { unsigned alpha : 4;  unsigned blue : 4; unsigned green : 4; unsigned red : 4;} pixel_type;
    
    std::vector<pixel_type> _image;
    
public:
    
    const char *documentsLocation;
    
    PixelPlane() : _program(0), _vertexBuffer(0), 
                   _frameBuffer(0), _renderBuffer(0), 
    _width(0), _height(0), _vwidth(0), _vheight(0), documentsLocation(NULL)
                   { }
    
    bool setup(int screenWidth, int screenHeight, int virtualWidth, int virtualHeight);
    void resize(int width, int height);
    
    void render();
    
    GLuint frameBuffer();
    GLuint renderBuffer();
    
    void updateGeometry();
    void updateTexture();
    
    virtual ~PixelPlane();
    
};

#endif
