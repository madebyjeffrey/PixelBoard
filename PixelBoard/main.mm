//
//  main.m
//  PixelBoard
//
//  Created by Jeffrey Drake on 12-01-26.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "AppDelegate.h"

#include <iostream>


int main(int argc, char *argv[])
{
int endian = 0x30313233;
char *c = reinterpret_cast<char*>(&endian);
    const size_t endian_size = sizeof(int);
    
    std::cerr << "Endian Result: ";

    for (size_t i = 0; i < endian_size; ++i)
        std::cerr << c[i] << " ";
    
    std::cerr << std::endl;
    
    
    
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
