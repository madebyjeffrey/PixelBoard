//
//  PixelView.h
//  PixelBoard
//
//  Created by Jeffrey Drake on 12-01-26.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

//#import "Shaders.h"
//#import "matrix.h"

//#import "PixelScene.h"

#include "QuadTest.h"

@interface PixelView : UIView
{
@private
    EAGLContext *context;
    //    PixelPlane *plane;
    
    QuadTest *quadScene;
    
    BOOL animating;
	//BOOL displayLinkSupported;
	// NSInteger animationFrameInterval;
	// Use of the CADisplayLink class is the preferred method for controlling your animation timing.
	// CADisplayLink will link to the main display and fire every vsync when added to a given run-loop.
	// The NSTimer class is used only as fallback when running on a pre 3.1 device where CADisplayLink
	// isn't available.
	id displayLink;
    //NSTimer *animationTimer;
    
}

- (void) render;
- (BOOL) resizeFromLayer:(CAEAGLLayer *)layer;

@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic) NSInteger animationFrameInterval;

- (void) startAnimation;
- (void) stopAnimation;
- (void) drawView:(id)sender;

@end
