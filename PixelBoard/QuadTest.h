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
    
    GLuint _frameBuffer, _renderBuffer;
public:
    bool setup(int width, int height);
    void resize(int width, int height);
    
    void render();
    
    GLuint frameBuffer();
    GLuint renderBuffer();
    
    virtual ~QuadTest();
    
};

#endif
