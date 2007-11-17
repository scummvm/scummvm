/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "iphone_video.h"
#include "iphone_common.h"

#import <UIKit/UIKit.h>
#import <UIKit/UIView-Geometry.h>
#import <GraphicsServices/GraphicsServices.h>
#import <Foundation/Foundation.h>
#import <CoreSurface/CoreSurface.h>
#import <LayerKit/LKLayer.h>

static iPhoneView *sharedInstance = nil;
static int _width = 0;
static int _height = 0;
static bool _landscape;
static int _orientation = -1;
static CGRect _screenRect;

// static long lastTick = 0;
// static int frames = 0;

unsigned short* iPhone_getSurface() {
	return CoreSurfaceBufferGetBaseAddress([sharedInstance getSurface]);
}

void iPhone_updateScreen() {
 	[sharedInstance performSelectorOnMainThread:@selector(updateScreen) withObject:nil waitUntilDone: NO];
}

void iPhone_lockSurface() {
	CoreSurfaceBufferLock([sharedInstance getSurface], 3);
}

void iPhone_unlockSurface() {
	CoreSurfaceBufferUnlock([sharedInstance getSurface]);
}

void iPhone_initSurface(int width, int height, bool landscape) {
	_width = width;
	_height = height;
	_landscape = landscape;
	
 	[sharedInstance performSelectorOnMainThread:@selector(initSurface) withObject:nil waitUntilDone: YES];
}

bool iPhone_fetchEvent(int *outEvent, float *outX, float *outY) {
	id event = [sharedInstance getEvent];
	if (event == nil) {
		return false;
	}
	
	id type = [event objectForKey:@"type"];
	
	if (type == nil) {
		printf("fetchEvent says: No type!\n");
		return false;			
	}
	
	*outEvent = [type intValue];
	*outX = [[event objectForKey:@"x"] floatValue];
	*outY = [[event objectForKey:@"y"] floatValue];
	return true;
}

bool getLocalMouseCoords(CGPoint *point) {
	if (point->x < _screenRect.origin.x || point->x > _screenRect.origin.x + _screenRect.size.width ||
		point->y < _screenRect.origin.y || point->y > _screenRect.origin.y + _screenRect.size.height) {
			return false;
	}
	
	point->x = (point->x - _screenRect.origin.x) / _screenRect.size.width;
	point->y = (point->y - _screenRect.origin.y) / _screenRect.size.height;
	
	return true;
}

@implementation iPhoneView

- (id)initWithFrame:(struct CGRect)frame {
	[super initWithFrame: frame];

	_fullWidth = frame.size.width;
	_fullHeight = frame.size.height;
	_screenLayer = nil;

	sharedInstance = self;

	_keyboard = [UIKeyboardImpl sharedInstance];
	//[self addSubview:_keyboard];

	return self;
}

-(void) dealloc {
	[super dealloc];
}

- (CoreSurfaceBufferRef)getSurface {
	return _screenSurface;
}

- (void)drawRect:(CGRect)frame {
	// if (lastTick == 0) {
	// 	lastTick = time(0);
	// }
	// 
	// frames++;
	// if (time(0) > lastTick) {
	// 	lastTick = time(0);
	// 	printf("FPS: %i\n", frames);
	// 	frames = 0;
	// }
}

- (void)updateScreen {
	[sharedInstance setNeedsDisplay];
}

- (void)initSurface {
	//printf("Window: (%d, %d), Surface: (%d, %d)\n", _fullWidth, _fullHeight, _width, _height);

	int pitch = _width * 2;
	int allocSize = 2 * _width * _height;
	char *pixelFormat = "565L";
	
	NSDictionary* dict = [[NSDictionary alloc] initWithObjectsAndKeys:
		kCFBooleanTrue, kCoreSurfaceBufferGlobal,
		 @"PurpleGFXMem", kCoreSurfaceBufferMemoryRegion,
		[NSNumber numberWithInt: pitch], kCoreSurfaceBufferPitch,
		[NSNumber numberWithInt: _width], kCoreSurfaceBufferWidth,
		[NSNumber numberWithInt: _height], kCoreSurfaceBufferHeight,
		[NSNumber numberWithInt: *(int*)pixelFormat], kCoreSurfaceBufferPixelFormat,
		[NSNumber numberWithInt: allocSize], kCoreSurfaceBufferAllocSize,
		nil
	];

	if (_screenSurface != nil) {
		//[[sharedInstance _layer] removeSublayer: screenLayer];
	}

	//("Allocating surface: %d\n", allocSize);
	_screenSurface = CoreSurfaceBufferCreate((CFDictionaryRef)dict);
	//printf("Surface created.\n");
	CoreSurfaceBufferLock(_screenSurface, 3);

	LKLayer* screenLayer = [[LKLayer layer] retain];
	
	if (_landscape) {
		float ratioDifference = ((float)_width / (float)_height) / ((float)_fullWidth / (float)_fullHeight);
		int rectWidth, rectHeight;
		if (ratioDifference < 1.0f) {
			rectWidth = _fullWidth * ratioDifference;
			rectHeight = _fullHeight;
			_widthOffset = (_fullWidth - rectWidth)/2;
			_heightOffset = 0;
		} else {
			rectWidth = _fullWidth;
			rectHeight = _fullHeight / ratioDifference;
			_heightOffset = (_fullHeight - rectHeight)/2;
			_widthOffset = 0;
		}

		//printf("Rect: %i, %i, %i, %i\n", _widthOffset, _heightOffset, rectWidth + _widthOffset, rectHeight + _heightOffset);
		_screenRect = CGRectMake(_widthOffset, _heightOffset, rectWidth + _widthOffset, rectHeight + _heightOffset);
		[screenLayer setFrame: _screenRect];		
	} else {
		float ratio = (float)_height / (float)_width;
		_screenRect = CGRectMake(0, 0, _fullWidth, _fullWidth * ratio);
		[screenLayer setFrame: _screenRect];				
	}

	[screenLayer setContents: _screenSurface];
	[screenLayer setOpaque: YES];
	
	if (_screenLayer != nil) {
		[[sharedInstance _layer] replaceSublayer: _screenLayer with: screenLayer];
	} else {
		[[sharedInstance _layer] addSublayer: screenLayer];
	}
	_screenLayer = screenLayer;
	
	CoreSurfaceBufferUnlock(_screenSurface);
	[dict release];
}


- (void)lock {
	[_lock lock];
}

- (void)unlock {
	[_lock unlock];
}

- (id)getEvent {
	if (_events == nil || [_events count] == 0) {
		return nil;
	}

	[self lock];

	id event = [_events objectAtIndex: 0];
	if (event == nil) {
		return nil;
	}
	
	[_events removeObjectAtIndex: 0];
	[self unlock];
		
	return event;
}

- (void)addEvent:(NSDictionary*)event {
	[self lock];
	
	if(_events == nil)
		_events = [[NSMutableArray alloc] init];
		
	[_events addObject: event];
	
	[self unlock];
}

- (void)deviceOrientationChanged:(int)orientation {
	[self addEvent:
		[[NSDictionary alloc] initWithObjectsAndKeys:
		 [NSNumber numberWithInt:kInputOrientationChanged], @"type",
		 [NSNumber numberWithFloat:(float)orientation], @"x",
		 [NSNumber numberWithFloat:0], @"y",
		 nil
		]
	];
}

- (void)mouseDown:(GSEvent*)event {
	struct CGPoint point = GSEventGetLocationInWindow(event);
	
	if (!getLocalMouseCoords(&point))
		return;

	[self addEvent:
		[[NSDictionary alloc] initWithObjectsAndKeys:
		 [NSNumber numberWithInt:kInputMouseDown], @"type",
		 [NSNumber numberWithFloat:point.x], @"x",
		 [NSNumber numberWithFloat:point.y], @"y",
		 nil
		]
	];
}

- (void)mouseUp:(GSEvent*)event {
	struct CGPoint point = GSEventGetLocationInWindow(event);
	
	if (!getLocalMouseCoords(&point))
		return;
	
	[self addEvent:
		[[NSDictionary alloc] initWithObjectsAndKeys:
		 [NSNumber numberWithInt:kInputMouseUp], @"type",
		 [NSNumber numberWithFloat:point.x], @"x",
		 [NSNumber numberWithFloat:point.y], @"y",
		 nil
		]
	];
}

- (void)mouseDragged:(GSEvent*)event {
	//printf("mouseDragged()\n");
	struct CGPoint point = GSEventGetLocationInWindow(event);
	
	if (!getLocalMouseCoords(&point))
		return;
	
	[self addEvent:
		[[NSDictionary alloc] initWithObjectsAndKeys:
		 [NSNumber numberWithInt:kInputMouseDragged], @"type",
		 [NSNumber numberWithFloat:point.x], @"x",
		 [NSNumber numberWithFloat:point.y], @"y",
		 nil
		]
	];
}

- (void)mouseEntered:(GSEvent*)event {
	//printf("mouseEntered()\n");
	// struct CGPoint point = GSEventGetLocationInWindow(event);
	// [self addEvent:
	// 	[[NSDictionary alloc] initWithObjectsAndKeys:
	// 	 [NSNumber numberWithInt:kInputMouseSecondStartDrag], @"type",
	// 	 [NSNumber numberWithFloat:(point.x/_fullWidth)], @"x",
	// 	 [NSNumber numberWithFloat:(point.y/_fullHeight)], @"y",
	// 	 nil
	// 	]
	// ];
}

- (void)mouseExited:(GSEvent*)event {
	//printf("mouseExited().\n");
	// [self addEvent:
	// 	[[NSDictionary alloc] initWithObjectsAndKeys:
	// 	 @"mouseExited", @"type",
	// 	 nil
	// 	]
	// ];
}

- (void)mouseMoved:(GSEvent*)event
{
	//printf("mouseMoved()\n");
	struct CGPoint point = GSEventGetLocationInWindow(event);
	
	if (!getLocalMouseCoords(&point))
		return;

	[self addEvent:
		[[NSDictionary alloc] initWithObjectsAndKeys:
		 [NSNumber numberWithInt:kInputMouseSecondToggled], @"type",
		 [NSNumber numberWithFloat:point.x], @"x",
		 [NSNumber numberWithFloat:point.y], @"y",
		 nil
		]
	];
}

- (void)keyDown:(GSEvent*)event
{
	printf("keyDown()\n");
	[self addEvent:
		[[NSDictionary alloc] initWithObjectsAndKeys:
		 @"keyDown", @"type",
		 nil
		]
	];
}

- (void)keyUp:(GSEvent*)event
{
	printf("keyUp()\n");
	[self addEvent:
		[[NSDictionary alloc] initWithObjectsAndKeys:
		  @"keyUp", @"type",
		 nil
		]
	];
}

@end

