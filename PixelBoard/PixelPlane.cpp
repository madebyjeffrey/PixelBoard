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
#include <fstream>
#include <iterator>

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "AppDelegate.h"

#include "PixelPlane.h"
#include "error.h"
#include "matrix.h"
#include "Shaders.h"

//#include "Image.h"

#include "Image2.h"
#include "lodepng.h"


void writeTGA(const char *filename, std::vector<PixelPlane::pixel_type> image);
std::vector<PixelPlane::pixel_type> readTGA(const char *filename);

static char const * vertexShader = R"(
#version 100  // OpenGL ES 2.0
attribute vec2 coord2d;
attribute vec2 texcoord2d;
attribute vec4 background2d;

uniform mat4 transform;

varying lowp vec2 texture;

void main(void) {                        
    gl_Position = transform * vec4(coord2d, 0.0, 1.0); 
    texture = texcoord2d;
}
)";

static char const * fragmentShader = R"(

    #version 100  // OpenGL ES 2.0

    varying lowp vec2 texture;
    uniform sampler2D s_texture;
    uniform sampler2D g_texture;

    void main(void) {        
        gl_FragColor = texture2D(s_texture, texture); // * (texture2D(g_texture, texture).a);
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
    
    
    glClearColor(0.0, 0.0, 0.0, 1.0);
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
    std::fill_n(std::back_inserter(vertices), 16, 0.0); // colour white repeated 4 times 4 elements

    
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
    
    glVertexAttribPointer(attribute_texture, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(8*sizeof(GLfloat)));
    GetError();
    
    
    
    GLuint uniform_texture = glGetUniformLocation(_program, "s_texture");
        GetError();
    glUniform1i(uniform_texture, 0);
        GetError();
    GLuint uniform_texture_grid = glGetUniformLocation(_program, "g_texture");
    GetError();
    glUniform1i(uniform_texture_grid, 1);
}

void PixelPlane::updateTexture()
{
    _image.clear();

/*
    pixel_type pixel = { 15, 0, 0, 15 };
    pixel_type blue = { 0, 15, 15, 15 };

    //    std::fill_n(std::back_inserter(_image), _vwidth * _vheight, pixel);
    for (int row = 0; row < _vheight; ++row)
        for (int col = 0; col < _vwidth; ++col)
        {
            pixel_type p;
            //            p.red = 15*(row/320.0);
            //            p.green = 15*(col/240.0);
            //p.alpha = 0;
            //p.blue = 0;
            p.red = 15;
            p.blue = (col < _vwidth/2 ? 15 : 0);
            p.green = 0;
            p.alpha = 0;
            _image.push_back(p);
        }*/
    
    std::vector<unsigned char> data;
    unsigned w, h;
    
    LodePNG::decode(data, w, h, resourcesFolder + "/cat1.png");
    Image<RGBA8888> image(w, h, transmogrify(data));
    
    auto texture = image_cast<RGB565>(image);
    
    
  /*  
    Image img;
    
    if (documentsLocation)
    {
        std::string str = std::string(documentsLocation);
        //        str += "/image.tga";
        str += "/cat1.tga";
        
        //            _image = readTGA(str.c_str());
        img.loadTGA(str);
        img.convertFormat(PixelFormat::ABGR4444);

        //        std::basic_fstream<pixel_type> of;*/
/*        std::fstream of;
        of.open(str, std::ios::out);

        //        std::copy(begin(_image), end(_image), std::ostream_iterator<pixel_type>(of));

        for (auto i : _image)
        {
            of.write((char*)&i, sizeof(i));
        }
        
        of.close();*/
        //        writeTGA(str.c_str(), _image);
        //    }
    
    //    pixel_type blue = { 0, 15, 15, 15 };
    //    std::fill_n(std::begin(_image), _vwidth * _vheight * 0.95, blue);
    //    _image[0] = blue;
    //    _image[1] = blue;
    
    _imageGrid.clear();
    _imageGrid.reserve(_vwidth * _vheight * 9);
    
    for (int x = 0; x < _vwidth * 3; ++x)
        for (int y = 0; y < _vheight * 3; ++y)
        {
            _imageGrid[y * (_vwidth * 3) + x] = ((x % 3 == 0) || (y % 3) == 0) ? 0.0 : 255.0;
        }
    
    if (_texture)
    {
        glDeleteTextures(1, &_texture);
        glDeleteTextures(1, &_textureGrid);
    }
    
    glGenTextures(1, &_texture);
    GetError();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texture);
        GetError();
    
    //    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _vwidth, _vheight, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, _image.data());
    img.bufferTexture();
        GetError();
    
    
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            GetError();
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
            GetError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            GetError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            GetError();

    
    glGenTextures(1, &_textureGrid);
    GetError();
    glActiveTexture(GL_TEXTURE1);
    GetError();
    glBindTexture(GL_TEXTURE_2D, _textureGrid);
    GetError();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, _vwidth * 3, _vheight * 3, 0, GL_ALPHA, GL_UNSIGNED_BYTE, _imageGrid.data());
    GetError();
    
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    GetError();
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
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

#include <stdio.h>


typedef struct {
    char id_len;                 // ID Field (Number of bytes - max 255)
    char map_type;               // Colormap Field (0 or 1)
    char img_type;               // Image Type (7 options - color vs. compression)
    int  map_first;              // Color Map stuff - first entry index
    int  map_len;                // Color Map stuff - total entries in file
    char map_entry_size;         // Color Map stuff - number of bits per entry
    int  x;                      // X-coordinate of origin 
    int  y;                      // Y-coordinate of origin
    int  width;                  // Width in Pixels
    int  height;                 // Height in Pixels
    char bpp;                    // Number of bits per pixel
    char misc;                   // Other stuff - scan origin and alpha bits
} targa_header;

void writeheader(targa_header h, FILE *tga);
void readheader(targa_header *h, FILE *tga);
void writeheader(targa_header h, FILE *tga) 
{
    fputc(h.id_len, tga);          // Write chars for ID, map, and image type
    fputc(h.map_type, tga);
    fputc(h.img_type, tga);
    fputc(h.map_first % 256, tga); // Write integer, low order byte first
    fputc(h.map_first / 256, tga); // Write second byte of integer, high order
    fputc(h.map_len % 256, tga);   // Another integer 
    fputc(h.map_len / 256, tga);
    fputc(h.map_entry_size, tga);  // Write a char - only one byte
    fputc(h.x % 256, tga);         // More integers
    fputc(h.x / 256, tga);
    fputc(h.y % 256, tga);
    fputc(h.y / 256, tga);
    fputc(h.width % 256, tga);     // Even more integers
    fputc(h.width / 256, tga);
    fputc(h.height % 256, tga);
    fputc(h.height / 256, tga);
    fputc(h.bpp, tga);             // Write two chars
    fputc(h.misc, tga);
}

void readheader(targa_header *h, FILE *tga)
{
    h->id_len = fgetc(tga);
    h->map_type = fgetc(tga);
    h->img_type = fgetc(tga);
    h->map_first = fgetc(tga) | (fgetc(tga) << 8);
    h->map_len = fgetc(tga) | (fgetc(tga) << 8);
    h->map_entry_size = fgetc(tga);
    h->x = fgetc(tga) | (fgetc(tga) << 8);
    h->y = fgetc(tga) | (fgetc(tga) << 8);
    h->width = fgetc(tga) | (fgetc(tga) << 8);
    h->height = fgetc(tga) | (fgetc(tga) << 8);
    h->bpp = fgetc(tga);
    h->misc = fgetc(tga);
}

//typedef struct { unsigned red : 4; unsigned green : 4; unsigned blue : 4; unsigned alpha : 4;} pixel_type;

//std::vector<pixel_type> _image;

void writeTGA(const char *filename, std::vector<PixelPlane::pixel_type> image)
{
    FILE *tga;               // Pointer to a FILE
    targa_header header;     // Variable of targa_header type
    int x, y;
    
    /* First, set all the fields in the header to appropriate values */
    header.id_len = 0;          /* no ID field */
    header.map_type = 0;        /* no colormap */
    header.img_type = 2;        /* trust me */
    header.map_first = 0;       /* not used */
    header.map_len = 0;         /* not used */
    header.map_entry_size = 0;  /* not used */
    header.x = 0;               /* image starts at (0,0) */
    header.y = 0;
    header.width = 320;         /* image is 200 x 100 */
    header.height = 240;
    header.bpp = 24;            /* 24 bits per pixel */
    header.misc = 0x20;         /* scan from upper left corner */
    
    /* Open a file for writing targa data.  Call the file "test.tga" and
     write in binary mode (wb) so that nothing is lost as characters
     are written to the file */
    
    tga = fopen(filename, "wb"); /* Write the header information  */
    
    writeheader(header, tga);  
    
    /* Write the data for a graphic that measures 100 by 200 pixels. */
    
    for(y = 0; y < 240; y++)      // Create 100 Rows of Pixels 
        for(x = 0; x < 320; x++)   // Create 200 Pixels in each Row
            
        { /* For each pixel, write a character representing the RGB color.
           Notice that the order that is written to the file is B-G-R.
           This sequence just cycles through the colors in some pattern
           but all char values must be integers between 0 and 255. */
            PixelPlane::pixel_type p = image[y * 320 + x];
            int r = (p.red / 15.0) * 255.0;
            int g = (p.green / 15.0) * 255.0;
            int b = (p.blue / 15.0) * 255.0;
            
            fputc(b % 256, tga);
            fputc(g % 256, tga);
            fputc(r % 256, tga);
            //            fputc((4*y)%256, tga);               // Write char for BLUE            
            //            fputc(255 - (x*2 + y*2)%256, tga);   // Write char for GREEN
            //fputc((3*x)%256, tga);               // Write char for RED
        }
    
    /* close the file */
    fclose(tga);
    
    /* that was easy, right? */
}

std::vector<PixelPlane::pixel_type> readTGA(const char *filename)
{
    FILE *tga;
    targa_header header;
    int x, y;
    
    memset(&header, 0, sizeof(header));
    
    tga = fopen(filename, "rb");
    readheader(&header, tga);
    
    std::vector<PixelPlane::pixel_type> pixs;
    pixs.reserve(header.width * header.height);
    
    for (y = 0; y < header.height; ++y)
    {
        for (x = 0; x < header.width ; ++x)
        {
            int b = fgetc(tga);
            int g = fgetc(tga);
            int r = fgetc(tga);
            
            PixelPlane::pixel_type p;
            p.red = (r / 255.0) * 15;
            p.green = (g / 255.0) * 15;
            p.blue = (b / 255.0) * 15;
            p.alpha = 15;
            
            pixs[y * header.width + x] = p;
        }
    }
    
    fclose(tga);
    
    return pixs;
}
 
