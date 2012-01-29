//
//  QuadTest.cpp
//  PixelBoard
//
//  Created by Jeffrey Drake on 12-01-29.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include <array>
#include <vector>
#include <algorithm>
#include <string>

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
//#import <QuartzCore/QuartzCore.h>


#include "QuadTest.h"

bool checkCompileStatus(GLuint shader);
bool checkLinkStatus(GLuint prog);

bool checkCompileStatus(GLuint shader)
{
    #if defined(DEBUG)
    GLint logLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = new GLchar[logLength]; //(GLchar *)malloc(logLength);
        glGetShaderInfoLog(shader, logLength, &logLength, log);
        std::cerr << "Shader compile log:" << std::endl;
        std::cerr << log << std::endl;
        delete log;
    }
    #endif
    
    GLint status;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        std::cerr << "Failed to compile shader." << std::endl;
        //        NSLog(@"Failed to compile shader:\n");
        //		int i;
        //		for (i = 0; i < count; i++)
        //        NSLog(@"%s", sources); //[i]);
    }
    
    return status;
}

bool checkLinkStatus(GLuint prog)
{
	GLint status;
	
#if defined(DEBUG)
	GLint logLength;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = new GLchar[logLength]; // (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        std::cerr << "Program link log:" << std::endl;
        std::cerr << log << std::endl;

        delete log;
    }
#endif
    
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
        //	NSLog(@"Failed to link program %d", prog);
        std::cerr << "Failed to link program " << prog << std::endl;
	
	return status;
}


static char const * vertexShader = R"(
    #ifdef GL_ES_VERSION_2_0
    #version 100  // OpenGL ES 2.0
    #else
    #version 120  // OpenGL 2.1
    #endif
    attribute vec2 coord2d;                  
    void main(void) {                        
      gl_Position = vec4(coord2d, 0.0, 1.0); 
    };
    )";

static char const * fragmentShader = R"(
    #ifdef GL_ES_VERSION_2_0
    #version 100  // OpenGL ES 2.0
    #else
    #version 120  // OpenGL 2.1
    #endif
    void main(void) {        
      gl_FragColor[0] = 0.0; 
      gl_FragColor[1] = 0.0; 
      gl_FragColor[2] = 1.0; 
    };
    )";

bool QuadTest::setup(int width, int height)
{
    glGenFramebuffers(1, &_frameBuffer);
    glGenRenderbuffers(1, &_renderBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _renderBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _renderBuffer);

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShader, NULL);
    glCompileShader(vs);
    if (!checkCompileStatus(vs))
        return false;
    
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShader, NULL);
    glCompileShader(fs);
    if (!checkCompileStatus(fs))
        return false;
    
    _program = glCreateProgram();
    glAttachShader(_program, vs);
    glAttachShader(_program, fs);
    glLinkProgram(_program);
    if (!checkLinkStatus(_program))
        return false;
    
    return true;
}

void QuadTest::resize(int width, int height)
{
    
}

void QuadTest::render()
{
    
    
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(_program);
    GLuint attribute_coord2d = glGetAttribLocation(_program, "coord2d");
    if (attribute_coord2d == -1)
    {
        std::cerr << "Could not bind attribute coord2d" << std::endl;
        return;
    }
    
    glEnableVertexAttribArray(attribute_coord2d);
    GLfloat triangle_vertices[] = {
        0.0, 0.8,
        -0.8, -0.8,
        0.8, -0.8,
    };
    
    glVertexAttribPointer(attribute_coord2d, 2, GL_FLOAT, GL_FALSE, 0, triangle_vertices);
    
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(attribute_coord2d);
}

GLuint QuadTest::frameBuffer()
{
    return _frameBuffer;
}

GLuint QuadTest::renderBuffer()
{
    return _renderBuffer;
}

QuadTest::~QuadTest()
{
    glDeleteProgram(_program);
}