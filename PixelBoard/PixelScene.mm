//
//  PixelScene.cpp
//  PixelBoard
//
//  Created by Jeffrey Drake on 12-01-27.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <array>
#include <vector>
#include <algorithm>
#include <string>

#import <UIKit/UIKit.h>

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <QuartzCore/QuartzCore.h>

#include "Shaders.h"

#include "PixelScene.h"
#include "error.h"

//enum {
    //	UNIFORM_MODELVIEW_PROJECTION_MATRIX,
    //	NUM_UNIFORMS
    //};
//static GLint uniforms[NUM_UNIFORMS];

enum {
	ATTRIB_VERTEX,
    ATTRIB_TEXCOORDS,
	NUM_ATTRIBUTES
};

std::string const vertexShader = R"(
    attribute vec4 position;
    attribute vec2 texcoord;

    uniform mat4 modelViewProjectionMatrix;

    varying vec2 textureVarying;

    void main()
    {
        gl_Position = modelViewProjectionMatrix * position;
        textureVarying = texcoord;
    }

    )";

std::string const fragmentShader = R"(
    #ifdef GL_ES
    // define default precision for float, vec, mat.
    precision highp float;
    #endif

    varying vec2 textureVarying;

    void main()
    {
        vec4 n = vec4(0,0,1,1);
        n.xy = textureVarying;
        gl_FragColor = n; //vec4(0,0,1, 1); //textureVarying;
    }
    )";


PixelPlane::PixelPlane(CGSize overallSize, CGSize individualSize, CGSize border, CGColorRef fill)
: _overallSize(overallSize), _individualSize(individualSize), _border(border),
    _program(0), _bufferObject(0), _texture(0)
{
    UIColor *color = [UIColor colorWithCGColor: fill];
    CGFloat r, g, b, a;
    PixelType px;

    // unfortuntely, this is the only way to convert this
    [color getRed: &r green: &g blue:&b alpha:&a];

    // These should optimize away, but just incase they are not exactly the same
    px[0] = static_cast<GLfloat>(r);
    px[1] = static_cast<GLfloat>(g);
    px[2] = static_cast<GLfloat>(b);
    px[3] = static_cast<GLfloat>(a);
    
    std::fill_n(std::back_inserter(Plane), overallSize.width * overallSize.height, px);
}

void PixelPlane::CreateTexture()
{
    glGenTextures(1, &_texture);
    
    glBindTexture(GL_TEXTURE_2D, _texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _overallSize.width, _overallSize.height, 0, GL_RGBA, GL_FLOAT, static_cast<void*>(Plane.data()));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void PixelPlane::CreateGeometry()
{
    // vertex then texture
    
    GLfloat vertices[] = { 0.0f, 0.0f, 0.0f, 1.0f,                              // vertices
                           _overallSize.width, 0.f, 0.f, 1.0f,
                           0.0f, _overallSize.height, 0.f, 1.0f,
                           _overallSize.width, _overallSize.height, 0.f, 1.0f,
                            0.0f, 0.0f,                                       // texture coordinates
                            1.0f, 0.0f,
                            0.0f, 1.0f,
                            1.0f, 1.0f };
    
    size_t const v_index = 0;
    size_t const v_size = sizeof(GLfloat) * 4 * 4;
    size_t const t_index = v_index + v_size;
    //    size_t const t_size = sizeof(GLfloat) * 2 * 4;
        
    
    Geometry.clear();
    std::move(vertices, vertices+sizeof(vertices), std::back_inserter(Geometry));

    glGenBuffers(1, &_bufferObject);
        GetError();

    glBindBuffer(GL_ARRAY_BUFFER, _bufferObject);
        GetError();
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        GetError();
    
    glEnableVertexAttribArray(ATTRIB_VERTEX);
        GetError();
    glEnableVertexAttribArray(ATTRIB_TEXCOORDS);
        GetError();
    
    glVertexAttribPointer(ATTRIB_VERTEX, 4, GL_FLOAT, GL_FALSE, 0, v_index);
        GetError();
    glVertexAttribPointer(ATTRIB_TEXCOORDS, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(t_index));
        GetError();
    
}

void PixelPlane::Render()
{
    glUseProgram(_program);
        GetError();
    glBindBuffer(GL_ARRAY_BUFFER, _bufferObject);
        GetError();

    //    validateProgram(_program);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        GetError();
    glFlush();
        GetError();
}

void PixelPlane::CreateFrameBuffer()
{
    // Create default framebuffer object. The backing will be allocated for the current layer in -resizeFromLayer
    glGenFramebuffers(1, &defaultFramebuffer);
    glGenRenderbuffers(1, &colorRenderbuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer);

}

void PixelPlane::SetSize(GLuint width, GLuint height)
{
    backingWidth = width;
    backingHeight = height;
}
    
void PixelPlane::UpdateProjection(GLfloat m[16])
{
    GLuint projMatrix = glGetUniformLocation(_program, "modelViewProjectionMatrix");
        GetError();
    
    glUniformMatrix4fv(projMatrix, 16, GL_FALSE, m);
        GetError();
}

bool PixelPlane::Setup()
{
    GLuint fragShader = 0, vertShader = 0;
    
    CreateFrameBuffer();
    CreateGeometry();
    CreateTexture();
    
    GetError();;
    _program = glCreateProgram();
        GetError();
    
    if (!compileShaderSource(&vertShader, GL_VERTEX_SHADER, 1, vertexShader)) {
		destroyShaders(vertShader, fragShader, _program);
		return NO;
	}
	
	// create and compile fragment shader
	if (!compileShaderSource(&fragShader, GL_FRAGMENT_SHADER, 1, fragmentShader)) {
		destroyShaders(vertShader, fragShader, _program);
		return NO;
	}
	
	// attach vertex shader to program
	glAttachShader(_program, vertShader);
	    GetError();
	// attach fragment shader to program
	glAttachShader(_program, fragShader);
        GetError();
	
	// bind attribute locations
	// this needs to be done prior to linking
	glBindAttribLocation(_program, ATTRIB_VERTEX, "position");
        GetError();
	glBindAttribLocation(_program, ATTRIB_TEXCOORDS, "texcoord");
        GetError();
	
	// link program
	if (!linkProgram(_program)) {
		destroyShaders(vertShader, fragShader, _program);
		return NO;
	}
	
	// get uniform locations
    //	uniforms[UNIFORM_MODELVIEW_PROJECTION_MATRIX] = glGetUniformLocation(_program, "modelViewProjectionMatrix");
    //        GetError();
	
	// release vertex and fragment shaders
	if (vertShader) {
		glDeleteShader(vertShader);
            GetError();
		vertShader = 0;
	}
	if (fragShader) {
		glDeleteShader(fragShader);
            GetError();
		fragShader = 0;
	}
    
    //    if (!validateProgram(_program))
    //    {
    //        NSLog(@"Validation of program failed.");
    //    }
	
	return YES;
    
}

PixelPlane::~PixelPlane()
{
    glDeleteTextures(1, &_texture);
    
}