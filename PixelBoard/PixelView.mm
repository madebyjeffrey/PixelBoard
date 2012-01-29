//
//  PixelView.m
//  PixelBoard
//
//  Created by Jeffrey Drake on 12-01-26.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "PixelView.h"

#include "error.h"

// uniform index
enum {
	UNIFORM_MODELVIEW_PROJECTION_MATRIX,
	NUM_UNIFORMS
};
GLint uniforms[NUM_UNIFORMS];

// attribute index
enum {
	ATTRIB_VERTEX,
	ATTRIB_COLOR,
	NUM_ATTRIBUTES
};


@implementation PixelView

@synthesize animating, animationFrameInterval;

+ (Class) layerClass
{
    return [CAEAGLLayer class];
}

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code
    }
    return self;
}

- (id) initWithCoder:(NSCoder *)aDecoder
{
    if (self = [super initWithCoder: aDecoder])
    {
        CAEAGLLayer *layer = (CAEAGLLayer*)self.layer;
        
        layer.opaque = TRUE;
        layer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:FALSE], 
                                        kEAGLDrawablePropertyRetainedBacking, 
                                        kEAGLColorFormatRGBA8, 
                                        kEAGLDrawablePropertyColorFormat, 
                                        nil];
        
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
        
        if (!context || ![EAGLContext setCurrentContext: context]) // || ![self loadShaders])
        {
            return nil;
        }
        
        displayLinkSupported = YES;
        animationFrameInterval = 1;

        plane = new PixelPlane(CGSizeMake(170, 170), CGSizeMake(2, 2), CGSizeMake(1,1), 
                       [UIColor redColor].CGColor);
        plane->Setup();

        [self resizeFromLayer: (CAEAGLLayer*)self.layer];
    }
    
    return self;
}

- (void)render {
    
    // Replace the implementation of this method to do your own custom drawing
    
/*    const GLfloat squareVertices[] = {
        -0.5f, -0.5f,
        0.5f,  -0.5f,
        -0.5f,  0.5f,
        0.5f,   0.5f,
    };
    const GLubyte squareColors[] = {
        255, 255,   0, 255,
        0,   255, 255, 255,
        0,     0,   0,   0,
        255,   0, 255, 255,
    };
*/	
    [EAGLContext setCurrentContext:context];
    
    glBindFramebuffer(GL_FRAMEBUFFER, plane->defaultFramebuffer);
        GetError();
    glViewport(0, 0, plane->backingWidth, plane->backingHeight);
        GetError();
    
    glClearColor(0.5f, 0.4f, 0.5f, 1.0f);
        GetError();
    glClear(GL_COLOR_BUFFER_BIT);
        GetError();
	
	// use shader program
    //	glUseProgram(program);
	
	// handle viewing matrices
	GLfloat proj[16]; //, modelview[16], modelviewProj[16];
	// setup projection matrix (orthographic)
	mat4f_LoadOrtho(0, 767, 0, 1023, -1.0f, 1.0f, proj);

	
    plane->UpdateProjection(proj);
    plane->Render();

    glBindRenderbuffer(GL_RENDERBUFFER, plane->colorRenderbuffer);
        GetError();
    [context presentRenderbuffer:GL_RENDERBUFFER];
}

                                                                       
- (BOOL) resizeFromLayer:(CAEAGLLayer *)layer
{
	// Allocate color buffer backing based on the current layer size
    glBindRenderbuffer(GL_RENDERBUFFER, plane->colorRenderbuffer);
    GetError();

    [context renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer];
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &(plane->backingWidth));
        GetError();
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &(plane->backingHeight));
        GetError();
	
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
            GetError();
        NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
        return NO;
    }
        GetError();
	
    return YES;
}

- (void) dealloc
{
	// tear down GL
	if (plane->defaultFramebuffer)
	{
		glDeleteFramebuffers(1, &(plane->defaultFramebuffer));
		plane->defaultFramebuffer = 0;
	}
	
	if (plane->colorRenderbuffer)
	{
		glDeleteRenderbuffers(1, &(plane->colorRenderbuffer));

		plane->colorRenderbuffer = 0;
	}
	
	// tear down context
	if ([EAGLContext currentContext] == context)
        [EAGLContext setCurrentContext:nil];
	
	//[context release];
	context = nil;
	
    //	[super dealloc];
}


- (void) drawView:(id)sender
{
    [self render];
}

- (void) layoutSubviews
{
	[self resizeFromLayer:(CAEAGLLayer*)self.layer];
    [self drawView:nil];
}

- (NSInteger) animationFrameInterval
{
	return animationFrameInterval;
}

- (void) setAnimationFrameInterval:(NSInteger)frameInterval
{
	// Frame interval defines how many display frames must pass between each time the
	// display link fires. The display link will only fire 30 times a second when the
	// frame internal is two on a display that refreshes 60 times a second. The default
	// frame interval setting of one will fire 60 times a second when the display refreshes
	// at 60 times a second. A frame interval setting of less than one results in undefined
	// behavior.
	if (frameInterval >= 1)
	{
		animationFrameInterval = frameInterval;
		
		if (animating)
		{
			[self stopAnimation];
			[self startAnimation];
		}
	}
}

- (void) startAnimation
{
	if (!animating)
	{
		if (displayLinkSupported)
		{
			// CADisplayLink is API new to iPhone SDK 3.1. Compiling against earlier versions will result in a warning, but can be dismissed
			// if the system version runtime check for CADisplayLink exists in -initWithCoder:. The runtime check ensures this code will
			// not be called in system versions earlier than 3.1.
			
			displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(drawView:)];
			[displayLink setFrameInterval:animationFrameInterval];
			[displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
		}
		else
			animationTimer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)((1.0 / 60.0) * animationFrameInterval) target:self selector:@selector(drawView:) userInfo:nil repeats:TRUE];
		
		animating = TRUE;
	}
}

- (void)stopAnimation
{
	if (animating)
	{
		if (displayLinkSupported)
		{
			[displayLink invalidate];
			displayLink = nil;
		}
		else
		{
			[animationTimer invalidate];
			animationTimer = nil;
		}
		
		animating = FALSE;
	}
}


/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect
{
    // Drawing code
}
*/

@end
