//
//  PixelPlane.cpp
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
#include <cmath>

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "PixelPlane.h"
#include "error.h"
#include "matrix.h"
#include "Shaders.h"


static char const * vertexShader = R"(
#version 100  // OpenGL ES 2.0
attribute vec2 coord2d;
attribute vec2 texcoord2d;
attribute vec4 background2d;

uniform mat4 transform;

varying highp vec2 texture;
varying highp vec2 position;
varying highp vec4 background;

void main(void) {                        
    gl_Position = transform * vec4(coord2d, 0.0, 1.0); 
    texture = texcoord2d;
    position = coord2d;
    background = background2d;
}
)";

static char const * fragmentShader = R"(

    #version 100  // OpenGL ES 2.0

    varying highp vec2 texture;
    uniform sampler2D s_texture;
    uniform highp vec2 cellsize;
    uniform highp vec2 imageSize;
    uniform highp vec2 margin;
    varying highp vec2 position;
    varying highp vec4 background;

    void main(void) {        
        highp vec2 pos = position - margin;
        highp vec2 total_cell = cellsize + vec2(1,1);
        
        highp float xmod = mod(floor(pos.x), floor(total_cell.x));
        highp float ymod = mod(floor(pos.y), floor(total_cell.y));
        
        if (xmod == 0.0 || ymod == 0.0)
        {
            gl_FragColor = background; //vec4(0, 0, 0, 0);
        }
        else
        {
            gl_FragColor = texture2D(s_texture, texture);
        }
/*        gl_FragColor[0] = 0.0; 
        gl_FragColor[1] = 0.0; 
        gl_FragColor[2] = 1.0;  */
        
    }
)";

bool PixelPlane::setup(int screenWidth, int screenHeight, int virtualWidth, int virtualHeight)
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
    
    _width = screenWidth;
    _height = screenHeight;
    _vwidth = virtualWidth;
    _vheight = virtualHeight;
    
    // updateGeometry();
    updateTexture();
    
    return true;
}

void PixelPlane::resize(int width, int height)
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

void PixelPlane::render()
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

void PixelPlane::updateGeometry()
{
    std::cerr << "Size: " << _width << " x " << _height << std::endl;
    
    std::vector<GLfloat> vertices;
    vertices.push_back((_width % _vwidth)/2);
    vertices.push_back((_height % _vheight)/2);
    
    vertices.push_back(_width - (_width % _vwidth)/2);
    vertices.push_back((_height % _vheight)/2);
    
    vertices.push_back((_width % _vwidth)/2);
    vertices.push_back(_height - (_height % _vheight)/2);
    
    vertices.push_back(_width - (_width % _vwidth)/2);
    vertices.push_back(_height - (_height % _vheight)/2);

    // texture coords index 8
    vertices.push_back(0.0);
    vertices.push_back(0.0);
    
    vertices.push_back(1.0);
    vertices.push_back(0.0);
    
    vertices.push_back(0.0);
    vertices.push_back(1.0);
    
    vertices.push_back(1.0);
    vertices.push_back(1.0);
    
    // colour index 16
    //    GLfloat colour[4] = {1.0, 1.0, 1.0, 1.0};
    std::fill_n(std::back_inserter(vertices), 16, 1.0); // colour white repeated 4 times 4 elements

    
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
    
    GLuint attribute_texture = glGetAttribLocation(_program, "texcoord2d");
        GetError();
    glEnableVertexAttribArray(attribute_texture);
        GetError();
    
    glVertexAttribPointer(attribute_texture, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)8);
    GetError();
    
    GLuint attribute_background2D = glGetAttribLocation(_program, "background2d");
        GetError();
    glEnableVertexAttribArray(attribute_background2D);
        GetError();
    glVertexAttribPointer(attribute_background2D, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)16);
        GetError();
    
    
    GLuint uniform_texture = glGetUniformLocation(_program, "s_texture");
        GetError();
    glUniform1i(uniform_texture, 0);
        GetError();
    
    GLuint uniform_cellsize = glGetUniformLocation(_program, "cellsize");
            GetError();
    GLfloat cellsize[2];
    cellsize[0] = floor(_width / _vwidth) - 1.0;
    cellsize[1] = floor(_height / _vheight) - 1.0;
    
    std::cerr << "Cell Size: " << cellsize[0] << " x " << cellsize[1] << std::endl;
    glUniform2fv(uniform_cellsize, 1, cellsize);
            GetError();
    
    GLuint uniform_imageSize = glGetUniformLocation(_program, "imageSize");
    glUniform2f(uniform_imageSize, _vwidth, _vheight);
    
    GLuint uniform_margin = glGetUniformLocation(_program, "margin");
    glUniform2f(uniform_margin, (_width % _vwidth)/2, (_height % _vheight)/2);
}

void PixelPlane::updateTexture()
{
    _image.clear();
    
    pixel_type pixel = { 15, 0, 0, 15 };


    std::fill_n(std::back_inserter(_image), _vwidth * _vheight, pixel);
    
    if (_texture)
    {
        glDeleteTextures(1, &_texture);
    }
    
    glGenTextures(1, &_texture);
    GetError();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texture);
        GetError();
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _vwidth, _vheight, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, _image.data());
        GetError();
    
    
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            GetError();
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
            GetError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            GetError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            GetError();

}

GLuint PixelPlane::frameBuffer()
{
    return _frameBuffer;
}

GLuint PixelPlane::renderBuffer()
{
    return _renderBuffer;
}

PixelPlane::~PixelPlane()
{
    glDeleteProgram(_program);
}