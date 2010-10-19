//
//  GLFixPlugIn.m
//  GLFix
//
//  Created by Mark Bolding on 3/4/08.

/* It's highly recommended to use CGL macros instead of changing the current context for plug-ins that perform OpenGL rendering */
#import <OpenGL/CGLMacro.h>

#import "GLFixPlugIn.h"

#define	kQCPlugIn_Name				@"fixation cross"
#define	kQCPlugIn_Description		@"renders fixation cross in OpenGL"

@implementation GLFixPlugIn

/*
 Here you need to declare the input / output properties as dynamic as Quartz Composer will handle their implementation
 @dynamic inputFoo, outputBar;
 */

@dynamic inputW, inputH, inputX, inputY, inputZ, inputDisparity, inputThinness, inputTrim, inputColor, inputImage;

+ (NSDictionary*) attributes
{
	/*
	 Return a dictionary of attributes describing the plug-in (QCPlugInAttributeNameKey, QCPlugInAttributeDescriptionKey...).
	 */
	
	return [NSDictionary dictionaryWithObjectsAndKeys:kQCPlugIn_Name, QCPlugInAttributeNameKey, kQCPlugIn_Description, QCPlugInAttributeDescriptionKey, nil];
}

+ (NSDictionary*) attributesForPropertyPortWithKey:(NSString*)key
{
	/*
	 Specify the optional attributes for property based ports (QCPortAttributeNameKey, QCPortAttributeDefaultValueKey...).
	 */
	/* Return the attributes for the plug-in property ports */
	if([key isEqualToString:@"inputW"])
		return [NSDictionary dictionaryWithObjectsAndKeys:@"Width", QCPortAttributeNameKey, [NSNumber numberWithFloat:1], QCPortAttributeDefaultValueKey, nil];
	if([key isEqualToString:@"inputH"])
		return [NSDictionary dictionaryWithObjectsAndKeys:@"Height", QCPortAttributeNameKey, [NSNumber numberWithFloat:1], QCPortAttributeDefaultValueKey, nil];
	if([key isEqualToString:@"inputX"])
		return [NSDictionary dictionaryWithObjectsAndKeys:@"X", QCPortAttributeNameKey, nil];
	if([key isEqualToString:@"inputY"])
		return [NSDictionary dictionaryWithObjectsAndKeys:@"Y", QCPortAttributeNameKey, nil];	
	if([key isEqualToString:@"inputZ"])
		return [NSDictionary dictionaryWithObjectsAndKeys:@"Z", QCPortAttributeNameKey, nil];	
	if([key isEqualToString:@"inputDisparity"])
		return [NSDictionary dictionaryWithObjectsAndKeys:@"Disparity", QCPortAttributeNameKey, nil];
	if([key isEqualToString:@"inputColor"])
		return [NSDictionary dictionaryWithObjectsAndKeys:@"Color", QCPortAttributeNameKey, nil];
	if([key isEqualToString:@"inputImage"])
		return [NSDictionary dictionaryWithObjectsAndKeys:@"Image", QCPortAttributeNameKey, nil];
	if([key isEqualToString:@"inputThinness"])
		return [NSDictionary dictionaryWithObjectsAndKeys:@"Thinness", QCPortAttributeNameKey, [NSNumber numberWithFloat:10], QCPortAttributeDefaultValueKey, nil];
	if([key isEqualToString:@"inputTrim"])
		return [NSDictionary dictionaryWithObjectsAndKeys:@"Trim", QCPortAttributeNameKey, [NSNumber numberWithFloat:0.1], QCPortAttributeDefaultValueKey, nil];

	return nil;
}

+ (QCPlugInExecutionMode) executionMode
{
	/*
	 Return the execution mode of the plug-in: kQCPlugInExecutionModeProvider, kQCPlugInExecutionModeProcessor, or kQCPlugInExecutionModeConsumer.
	 */
	
	return kQCPlugInExecutionModeConsumer;
}

+ (QCPlugInTimeMode) timeMode
{
	/*
	 Return the time dependency mode of the plug-in: kQCPlugInTimeModeNone, kQCPlugInTimeModeIdle or kQCPlugInTimeModeTimeBase.
	 */
	
	return kQCPlugInTimeModeNone;
}

@end

@implementation GLFixPlugIn (Execution)

- (BOOL) execute:(id<QCPlugInContext>)context atTime:(NSTimeInterval)time withArguments:(NSDictionary*)arguments
{
	/*
	 Called by Quartz Composer whenever the plug-in instance needs to execute.
	 Only read from the plug-in inputs and produce a result (by writing to the plug-in outputs or rendering to the destination OpenGL context) within that method and nowhere else.
	 Return NO in case of failure during the execution (this will prevent rendering of the current frame to complete).
	 
	 The OpenGL context for rendering can be accessed and defined for CGL macros using:
	 CGLContextObj cgl_ctx = [context CGLContextObj];
	 */
	
	CGLContextObj					cgl_ctx = [context CGLContextObj];
	id<QCPlugInInputImageSource>	image;
	GLuint							textureName;
	GLint							saveMode /*,saveName*/;
	//GLboolean						saveEnabled;
	const CGFloat*					colorComponents;
	GLenum							error;
	
	if(cgl_ctx == NULL)
		return NO;
	
	/* Copy the image on the "inputImage" input port to a local variable */
	image = self.inputImage;
	
	/* Get a texture from the image in the context colorspace */
	if(image && [image lockTextureRepresentationWithColorSpace:([image shouldColorMatch] ? [context colorSpace] : [image imageColorSpace]) forBounds:[image imageBounds]])
		textureName = [image textureName];
	else
		textureName = 0;
	
	/* Save and set modelview matrix */
	glGetIntegerv(GL_MATRIX_MODE, &saveMode);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(self.inputX, self.inputY, self.inputZ);
	glScalef(self.inputW, self.inputH, 1.0);
	
	/* Bind texture to unit */
	if(textureName)
		[image bindTextureRepresentationToCGLContext:cgl_ctx textureUnit:GL_TEXTURE0 normalizeCoordinates:YES];
	
	/* Get RGBA components from the color on the "inputColor" input port (the CGColorRef is guaranteed to be of type RGBA) */
	colorComponents = CGColorGetComponents(self.inputColor);
	
	/* Set current color (no need to save / restore as the current color is part of the GL_CURRENT_BIT) */
	glColor4f(colorComponents[0], colorComponents[1], colorComponents[2], colorComponents[3]);
	
	/* Render textured quad (we can use normalized texture coordinates independently of the texture target or the texture vertical flipping state thanks to -bindTextureRepresentationToCGLContext) */


	float x = 0;
	float y = 0;
	float z = 0;
	float h = self.inputTrim;
	float w = h/self.inputThinness;
	float d = self.inputDisparity;
	
	glBegin(GL_QUADS);
		x = x-d/2.0;
		glTexCoord2f(0.0, 0.0);
		glVertex3f(x-w,y-h,z);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(x+w,y-h,z);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(x+w,y+h,z);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(x-w,y+h,z);
	
		glTexCoord2f(0.0, 0.0);
		glVertex3f(x-h,y-w,z);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(x+h,y-w,z);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(x+h,y+w,z);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(x-h,y+w,z);
		
		x=x+d;
		glTexCoord2f(0.0, 0.0);
		glVertex3f(x-w,y-h,z);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(x+w,y-h,z);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(x+w,y+h,z);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(x-w,y+h,z);
	
		glTexCoord2f(0.0, 0.0);
		glVertex3f(x-h,y-w,z);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(x+h,y-w,z);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(x+h,y+w,z);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(x-h,y+w,z);

	glEnd();


	
	/* Unbind texture from unit */
	if(textureName)
		[image unbindTextureRepresentationFromCGLContext:cgl_ctx textureUnit:GL_TEXTURE0];
	
	/* Restore modelview matrix */
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(saveMode);
	
	/* Check for OpenGL errors */
	if(error = glGetError())
		[context logMessage:@"OpenGL error %04X", error];
	
	/* Release texture */
	if(textureName)
		[image unlockTextureRepresentation];
	
	return (error ? NO : YES);
	
}	
@end
