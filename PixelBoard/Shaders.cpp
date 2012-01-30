
#include <iostream>
#include <array>
#include <vector>
#include <algorithm>
#include <string>

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#include "Shaders.h"


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
