//
//  QuadTest.h
//  PixelBoard
//
//  Created by Jeffrey Drake on 12-01-29.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef PixelBoard_QuadTest_h
#define PixelBoard_QuadTest_h

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>


class QuadTest
{
private:
    GLuint _program;
    GLuint _vertexBuffer;
    
    GLuint _frameBuffer, _renderBuffer;
    int _width, _height;
public:
    QuadTest() : _program(0), _vertexBuffer(0), _frameBuffer(0), _renderBuffer(0), _width(0), _height(0) { }
    
    bool setup(int width, int height);
    void resize(int width, int height);
    
    void render();
    
    GLuint frameBuffer();
    GLuint renderBuffer();
    
    void updateGeometry();
    
    virtual ~QuadTest();
    
};

#endif
