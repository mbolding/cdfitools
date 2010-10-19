//
//  GLdotsPlugIn.m
//  GLdots
//
//  Created by Mark Bolding on 3/5/08.

/* It's highly recommended to use CGL macros instead of changing the current context for plug-ins that perform OpenGL rendering */
#import <OpenGL/CGLMacro.h>

#import "GLdotsPlugIn.h"

#define	kQCPlugIn_Name				@"random dots"
#define	kQCPlugIn_Description		@"renders random dot fields"

#define maxPts	1024

@implementation GLdotsPlugIn

/*
Here you need to declare the input / output properties as dynamic as Quartz Composer will handle their implementation
@dynamic inputFoo, outputBar;
*/
@dynamic inputW, inputH, inputX, inputY, inputZ, inputDotSize, inputNumDots, inputDisparity, inputColor, inputColor2;



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
		return [NSDictionary dictionaryWithObjectsAndKeys:@"Width", QCPortAttributeNameKey, [NSNumber numberWithFloat:1.0], QCPortAttributeDefaultValueKey, nil];
	if([key isEqualToString:@"inputH"])
		return [NSDictionary dictionaryWithObjectsAndKeys:@"Height", QCPortAttributeNameKey, [NSNumber numberWithFloat:1.0], QCPortAttributeDefaultValueKey, nil];
	if([key isEqualToString:@"inputX"])
		return [NSDictionary dictionaryWithObjectsAndKeys:@"X", QCPortAttributeNameKey, nil];
	if([key isEqualToString:@"inputY"])
		return [NSDictionary dictionaryWithObjectsAndKeys:@"Y", QCPortAttributeNameKey, nil];	
	if([key isEqualToString:@"inputZ"])
		return [NSDictionary dictionaryWithObjectsAndKeys:@"Z", QCPortAttributeNameKey, nil];	
	if([key isEqualToString:@"inputDotSize"])
		return [NSDictionary dictionaryWithObjectsAndKeys:@"Dot Size", QCPortAttributeNameKey, [NSNumber numberWithUnsignedInteger:1], QCPortAttributeDefaultValueKey, nil];
	if([key isEqualToString:@"inputNumDots"])
		return [NSDictionary dictionaryWithObjectsAndKeys:@"Number of Dots", QCPortAttributeNameKey, [NSNumber numberWithUnsignedInteger:100], QCPortAttributeDefaultValueKey, [NSNumber numberWithUnsignedInteger:maxPts], QCPortAttributeMaximumValueKey, [NSNumber numberWithUnsignedInteger:1], QCPortAttributeMinimumValueKey, nil];
	if([key isEqualToString:@"inputDisparity"])
		return [NSDictionary dictionaryWithObjectsAndKeys:@"Disparity", QCPortAttributeNameKey, nil];
	if([key isEqualToString:@"inputColor"])
		return [NSDictionary dictionaryWithObjectsAndKeys:@"Color", QCPortAttributeNameKey, nil];
	if([key isEqualToString:@"inputColor2"])
		return [NSDictionary dictionaryWithObjectsAndKeys:@"Color 2", QCPortAttributeNameKey, nil];

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

@implementation GLdotsPlugIn (Execution)

- (BOOL) startExecution:(id<QCPlugInContext>)context
{
	CGLContextObj					cgl_ctx = [context CGLContextObj];
	
	if(cgl_ctx == NULL) 
		return NO;
	
	return YES;
}

- (BOOL) execute:(id<QCPlugInContext>)context atTime:(NSTimeInterval)time withArguments:(NSDictionary*)arguments
{
	CGLContextObj					cgl_ctx = [context CGLContextObj];
	GLenum							error;
	GLint							saveMode /*,
									saveName */;
	/*GLboolean						saveEnabled;*/
	const CGFloat*					colorComponents;
	const CGFloat*					colorComponents2;
	int								i;
	GLfloat							x,y;
	
	if(cgl_ctx == NULL) 
		return NO;

	/* Save and set modelview matrix */
	glGetIntegerv(GL_MATRIX_MODE, &saveMode);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(self.inputX, self.inputY, self.inputZ);
	glScalef(self.inputW, self.inputH, 1.0);
	/* Get RGBA components from the color on the "inputColor" input port (the CGColorRef is guaranteed to be of type RGBA) */
	colorComponents = CGColorGetComponents(self.inputColor);
	colorComponents2 = CGColorGetComponents(self.inputColor2);
	
	/* Set current color (no need to save / restore as the current color is part of the GL_CURRENT_BIT) */	
	glColor4f(colorComponents2[0], colorComponents2[1], colorComponents2[2], colorComponents2[3]);
	
	glBegin(GL_QUADS);
		glVertex3f(0.5 - self.inputDisparity/2.0, 0.5, 0);
		glVertex3f(-0.5 - self.inputDisparity/2.0, 0.5, 0);
		glVertex3f(-0.5 - self.inputDisparity/2.0, -0.5, 0);
		glVertex3f(0.5 - self.inputDisparity/2.0, -0.5, 0);
	glEnd();
	
	glBegin(GL_QUADS);
		glVertex3f(0.5 + self.inputDisparity/2.0, 0.5, 0);
		glVertex3f(-0.5 + self.inputDisparity/2.0, 0.5, 0);
		glVertex3f(-0.5 + self.inputDisparity/2.0, -0.5, 0);
		glVertex3f(0.5 + self.inputDisparity/2.0, -0.5, 0);
	glEnd();
	
	/* Set current color (no need to save / restore as the current color is part of the GL_CURRENT_BIT) */
	glColor4f(colorComponents[0], colorComponents[1], colorComponents[2], colorComponents[3]);
	glPointSize(self.inputDotSize);
	
	glBegin(GL_POINTS);
	for ( i=0 ; i < self.inputNumDots-1 ; i++ ) {
		x = (random()/2147483648.0) - 0.5;
		y = (random()/2147483648.0) - 0.5;
		glVertex3f( x - self.inputDisparity/2.0, y, 0.0 );
		glVertex3f( x + self.inputDisparity/2.0, y, 0.0 );
	}
	glEnd();


	/* Restore modelview matrix */
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(saveMode);


	/* Check for OpenGL errors */
	if(error = glGetError())
		[context logMessage:@"OpenGL error %04X", error];
	
	return (error ? NO : YES);
	
}


- (void) stopExecution:(id<QCPlugInContext>)context
{
	CGLContextObj					cgl_ctx = [context CGLContextObj];
	if(cgl_ctx == NULL)
		return;
	/*
	Called by Quartz Composer when rendering of the composition stops: perform any required cleanup for the plug-in.
	*/
	
}

@end
