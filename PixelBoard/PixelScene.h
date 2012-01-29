//
//  PixelScene.h
//  PixelBoard
//
//  Created by Jeffrey Drake on 12-01-27.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef PixelBoard_PixelScene_h
#define PixelBoard_PixelScene_h

#include <array>
#include <vector>
#include <algorithm>
#include <string>

#import <UIKit/UIKit.h>

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <QuartzCore/QuartzCore.h>

class PixelPlane 
{
private:
    typedef std::array<GLfloat, 4> PixelType;
    alignas(PixelType) std::vector<PixelType> Plane;
    alignas(PixelType) std::vector<GLfloat> Geometry;
    CGSize _overallSize, _individualSize, _border;
    GLuint _texture;
    GLuint _bufferObject;
    GLuint _program;

	
    
    // The OpenGL names for the framebuffer and renderbuffer used to render to this view

    
public:

    	GLuint defaultFramebuffer, colorRenderbuffer;
    
    GLint backingWidth;
	GLint backingHeight;
    
    PixelPlane(CGSize overallSize, CGSize individualSize, CGSize border, CGColorRef const fill);
    void CreateTexture();
    void CreateGeometry();
    bool Setup();
    void Render();
    void UpdateProjection(GLfloat m[16]);
    void CreateFrameBuffer();
    void SetSize(GLuint width, GLuint height);
    ~PixelPlane();
};


#endif
