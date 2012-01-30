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

#include "QuadTest.h"
#include "error.h"
#include "matrix.h"

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
bool checkValidationStatus(GLuint prog);
bool checkValidationStatus(GLuint prog)
{
	GLint status;
	
#if defined(DEBUG)
	GLint logLength;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = new GLchar[logLength]; // (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        std::cerr << "Program validate log:" << std::endl;
        std::cerr << log << std::endl;
        
        delete log;
    }
#endif
    
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
        //	NSLog(@"Failed to link program %d", prog);
        std::cerr << "Failed to validate program " << prog << std::endl;
	
	return status;
}

static char const * vertexShader = R"(
    #version 100  // OpenGL ES 2.0
    attribute vec2 coord2d;      

    uniform mat4 transform;

    void main(void) {                        
      gl_Position = transform * vec4(coord2d, 0.0, 1.0); 
    }
    )";

static char const * fragmentShader = R"(
    #version 100  // OpenGL ES 2.0
    void main(void) {        
      gl_FragColor[0] = 0.0; 
      gl_FragColor[1] = 0.0; 
      gl_FragColor[2] = 1.0; 
    }
    )";

bool QuadTest::setup(int width, int height)
{
    glGenFramebuffers(1, &_frameBuffer);
    GetError();
    
    glGenRenderbuffers(1, &_renderBuffer);
    GetError();

    
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
    GetError();
    
    glBindRenderbuffer(GL_RENDERBUFFER, _renderBuffer);
    GetError();
    
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _renderBuffer);
    GetError();
    

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GetError();
    
    glShaderSource(vs, 1, &vertexShader, NULL);
    GetError();
    
    glCompileShader(vs);
    GetError();
    
    if (!checkCompileStatus(vs))
        return false;
    
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    GetError();
    
    glShaderSource(fs, 1, &fragmentShader, NULL);
    GetError();
    
    glCompileShader(fs);
    GetError();
    
    if (!checkCompileStatus(fs))
        return false;
    
    _program = glCreateProgram();
    GetError();
    
    glAttachShader(_program, vs);
    GetError();
    
    glAttachShader(_program, fs);
    GetError();
    
    glLinkProgram(_program);
    GetError();
    
    if (!checkLinkStatus(_program))
        return false;
    
    _width = width;
    _height = height;
    
    // updateGeometry();
    
    return true;
}

void QuadTest::resize(int width, int height)
{
    _width = width;
    _height = height;
    
    GLfloat m[16];
    
    glUseProgram(_program);
    
    mat4f_LoadOrtho(0, width, 0, height, 1.0, -1.0, m);
    GLuint transform = glGetUniformLocation(_program, "transform");
    
    glUniformMatrix4fv(transform, 1, GL_FALSE, m);
    
    updateGeometry();
}

void QuadTest::render()
{
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
    GetError();
    
    glViewport(0, 0, _width, _height);    
    GetError();
    
    
    glClearColor(1.0, 1.0, 1.0, 1.0);
    GetError();
    
    glClear(GL_COLOR_BUFFER_BIT);
    GetError();
    
    glUseProgram(_program);
    GetError();

    
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    GetError();
    
    glValidateProgram(_program);
    checkValidationStatus(_program);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    GetError();
    
}

void QuadTest::updateGeometry()
{
    std::cerr << "Size: " << _width << " x " << _height << std::endl;
    
    std::vector<GLfloat> vertices;
    vertices.push_back(5.0);
    vertices.push_back(5.0);
    
    vertices.push_back(_width - 5.0);
    vertices.push_back(5.0);
    
    vertices.push_back(5.0);
    vertices.push_back(_height - 5.0);
    
    vertices.push_back(_width - 5.0);
    vertices.push_back(_height - 5.0);
    
    //    size_t const v_index = 0;
    //    size_t const v_size = sizeof(GLfloat) * 4 * 4;
    //    size_t const t_index = v_index + v_size;
    //    size_t const t_size = sizeof(GLfloat) * 2 * 4;
    
    if (_vertexBuffer)
    {
        glDeleteBuffers(1, &_vertexBuffer);
        GetError();
    }
    
    glGenBuffers(1, &_vertexBuffer);
    GetError();
    
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    GetError();
    std::cerr << "size of vector: " << vertices.size() * sizeof(GLfloat) << std::endl;
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    GetError();
    
    glUseProgram(_program);
    GetError();
    
    GLuint attribute_coord2d = glGetAttribLocation(_program, "coord2d");
    GetError();
    
    if (attribute_coord2d == -1)
    {
        std::cerr << "Could not bind attribute coord2d" << std::endl;
        return;
    }

    glEnableVertexAttribArray(attribute_coord2d);
    GetError();
    
    
    glVertexAttribPointer(attribute_coord2d, 2, GL_FLOAT, GL_FALSE, 0, 0);
    GetError();
    

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