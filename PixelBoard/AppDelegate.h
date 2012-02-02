//
//  AppDelegate.h
//  PixelBoard
//
//  Created by Jeffrey Drake on 12-01-26.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#if defined (__OBJC2__)

#import <UIKit/UIKit.h>

@class ViewController;

@interface AppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIWindow *window;

@property (strong, nonatomic) ViewController *viewController;

@end

#endif

#if defined (__cplusplus)

#include <string>

extern std::string documentsFolder;
extern std::string resourcesFolder;

#endif
