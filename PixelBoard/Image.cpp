//
//  Image.cpp
//  PixelBoard
//
//  Created by Jeffrey Drake on 12-01-30.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef PixelBoard_Image_cpp
#define PixelBoard_Image_cpp

#include <string>

class Image
{
private:
    unsigned _width, _height;
    
    void loadTGA24(std::string &name);
    void saveTGA24(std::string &name);
public:
    enum PixelFormat
    {
        BGRA4444,
        RGBA8888,
        RGBA5551
    } type;
    
    enum FileFormat
    {
        TGA24, TGA32
    }
    
    
    convertFormat(PixelFormat dest);
    saveFile(FileFormat typ, std::string name);
    loadFile(FileFormat typ, std::string name);
};

void Image::loadTGA24(std::string &name)
{
    
}

void Image::saveTGA24(std::string &name)
{
    
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

#endif
