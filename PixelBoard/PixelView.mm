//
//  PixelView.m
//  PixelBoard
//
//  Created by Jeffrey Drake on 12-01-26.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "PixelView.h"

#include "error.h"

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
        
        //        animating = YES;

        
        //        displayLinkSupported = YES;
        //animationFrameInterval = 1;

        //        plane = new PixelPlane(CGSizeMake(170, 170), CGSizeMake(2, 2), CGSizeMake(1,1), 
        //                       [UIColor redColor].CGColor);
        //        plane->Setup();
        
        //        quadScene = new QuadTest();
        //        quadScene->setup(0, 0);
        scene = new PixelPlane();
        

        
        scene->setup(0, 0, 320, 240);

        [self resizeFromLayer: (CAEAGLLayer*)self.layer];
        
        [self startAnimation];
    }
    
    return self;
}


- (void)render {
    [EAGLContext setCurrentContext:context];

    
	
	// use shader program
    //	glUseProgram(program);
	
	// handle viewing matrices
    //	GLfloat proj[16]; //, modelview[16], modelviewProj[16];
	// setup projection matrix (orthographic)
	//mat4f_LoadOrtho(0, 767, 0, 1023, -1.0f, 1.0f, proj);

	scene->render();
    //plane->UpdateProjection(proj);
    //plane->Render();

    //    glBindRenderbuffer(GL_RENDERBUFFER, plane->colorRenderbuffer);
    //    GetError();
    glBindRenderbuffer(GL_RENDERBUFFER, scene->renderBuffer());
    [context presentRenderbuffer:GL_RENDERBUFFER];
}

                                                                       
- (BOOL) resizeFromLayer:(CAEAGLLayer *)layer
{
    GLint width, height;
    
	// Allocate color buffer backing based on the current layer size
    glBindRenderbuffer(GL_RENDERBUFFER, scene->renderBuffer());
    GetError();

    [context renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer];
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
        GetError();
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
        GetError();
	
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
        GetError();
        NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
        return NO;
    }
    GetError();
    
    scene->resize(width, height);
	
    return YES;
}

- (void) dealloc
{
	// tear down GL
/*	if (plane->defaultFramebuffer)
	{
		glDeleteFramebuffers(1, &(plane->defaultFramebuffer));
		plane->defaultFramebuffer = 0;
	}
	
	if (plane->colorRenderbuffer)
	{
		glDeleteRenderbuffers(1, &(plane->colorRenderbuffer));

		plane->colorRenderbuffer = 0;
	}
*/
	delete scene;
    
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
        //		if (displayLinkSupported)
        //		{
			// CADisplayLink is API new to iPhone SDK 3.1. Compiling against earlier versions will result in a warning, but can be dismissed
			// if the system version runtime check for CADisplayLink exists in -initWithCoder:. The runtime check ensures this code will
			// not be called in system versions earlier than 3.1.
			
			displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(drawView:)];
			[displayLink setFrameInterval:animationFrameInterval];
			[displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        //		}
        //		else
        //			animationTimer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)((1.0 / 60.0) * animationFrameInterval) target:self selector:@selector(drawView:) userInfo:nil repeats:TRUE];
		
		animating = TRUE;
	}
}

- (void)stopAnimation
{
	if (animating)
	{
        //	if (displayLinkSupported)
		//{
			[displayLink invalidate];
			displayLink = nil;
		//}
        //		else
		//{
		//	[animationTimer invalidate];
        //			animationTimer = nil;
        //		}
		
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
