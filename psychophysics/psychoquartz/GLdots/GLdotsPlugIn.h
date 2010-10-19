//
//  GLdotsPlugIn.h
//  GLdots
//
//  Created by Mark Bolding on 3/5/08.
//  Copyright (c) 2008 __MyCompanyName__. All rights reserved.
//

#import <Quartz/Quartz.h>

@interface GLdotsPlugIn : QCPlugIn
{
}

/*
Declare here the Obj-C 2.0 properties to be used as input and output ports for the plug-in e.g.
@property double inputFoo;
@property(assign) NSString* outputBar;
You can access their values in the appropriate plug-in methods using self.inputFoo or self.inputBar
*/

/* Declare property input ports of type "Number" */
@property double inputW;
@property double inputH;
@property double inputX;
@property double inputY;
@property double inputZ;
@property double inputDotSize;
@property double inputNumDots;
@property double inputDisparity;

/* Declare a property input port of type "Color" and with the key "inputColor" */
@property(assign) CGColorRef inputColor;
@property(assign) CGColorRef inputColor2;


@end
