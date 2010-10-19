//
//  GLFixPlugIn.h
//  GLFix
//
//  Created by Mark Bolding on 3/4/08.
//  
//

#import <Quartz/Quartz.h>

@interface GLFixPlugIn : QCPlugIn
{
}

/*
Declare here the Obj-C 2.0 properties to be used as input and output ports for the plug-in e.g.
@property double inputFoo;
@property(assign) NSString* outputBar;
You can access their values in the appropriate plug-in methods using self.inputFoo or self.inputBar
*/


/* Declare property input ports of type "Number" */
@property double inputH;
@property double inputW;
@property double inputX;
@property double inputY;
@property double inputZ;
@property double inputDisparity;
@property double inputThinness;
@property double inputTrim;

/* Declare a property input port of type "Color" and with the key "inputColor" */
@property(assign) CGColorRef inputColor;

/* Declare a property input port of type "Image" and with the key "inputImage" */
@property(assign) id<QCPlugInInputImageSource> inputImage;



@end
