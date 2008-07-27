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
#import <UIKit/UIKeyboardLayoutQWERTY.h>
#import <QuartzCore/QuartzCore.h>

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
	[sharedInstance performSelectorOnMainThread:@selector(setNeedsDisplay) withObject:nil waitUntilDone: NO];
}

void iPhone_updateScreenRect(int x1, int y1, int x2, int y2) {
	//CGRect rect = CGRectMake(x1, y1, x2, y2);
	//[sharedInstance performSelectorOnMainThread:@selector(updateScreenRect:) withObject: [NSValue valueWithRect:rect] waitUntilDone: NO];
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
	if (point->x < _screenRect.origin.x || point->x >= _screenRect.origin.x + _screenRect.size.width ||
		point->y < _screenRect.origin.y || point->y >= _screenRect.origin.y + _screenRect.size.height) {
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
	_keyboardView = nil;
	//[super setTapDelegate: self];

	return self;
}

-(void) dealloc {
	[super dealloc];

	if (_keyboardView != nil) {
		[_keyboardView dealloc];
	}
}

- (CoreSurfaceBufferRef)getSurface {
	return _screenSurface;
}

- (void)drawRect:(CGRect)frame {
	// if (lastTick == 0) {
	//	lastTick = time(0);
	// }
	//
	// frames++;
	// if (time(0) > lastTick) {
	//	lastTick = time(0);
	//	printf("FPS: %i\n", frames);
	//	frames = 0;
	// }
}

- (void)updateScreenRect:(id)rect {
	// NSRect nsRect = [rect rectValue];
	// CGRect cgRect = CGRectMake(nsRect.origin.x, nsRect.origin.y, nsRect.size.width, nsRect.size.height);
	// [sharedInstance setNeedsDisplayInRect: cgRect];
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

	CALayer* screenLayer = [[CALayer layer] retain];

	if (_keyboardView != nil) {
		[_keyboardView removeFromSuperview];
		[[_keyboardView inputView] removeFromSuperview];
	}

	if (_landscape) {
		float ratioDifference = ((float)_width / (float)_height) / ((float)_fullWidth / (float)_fullHeight);
		int rectWidth, rectHeight;
		if (ratioDifference < 1.0f) {
			rectWidth = _fullWidth * ratioDifference;
			rectHeight = _fullHeight;
			_widthOffset = (_fullWidth - rectWidth) / 2;
			_heightOffset = 0;
		} else {
			rectWidth = _fullWidth;
			rectHeight = _fullHeight / ratioDifference;
			_heightOffset = (_fullHeight - rectHeight) / 2;
			_widthOffset = 0;
		}

		//printf("Rect: %i, %i, %i, %i\n", _widthOffset, _heightOffset, rectWidth, rectHeight);
		_screenRect = CGRectMake(_widthOffset, _heightOffset, rectWidth, rectHeight);
		[screenLayer setFrame: _screenRect];
	} else {
		float ratio = (float)_height / (float)_width;
		_screenRect = CGRectMake(0, 0, _fullWidth, _fullWidth * ratio);
		[screenLayer setFrame: _screenRect];

		CGRect keyFrame = CGRectMake(0.0f, _screenRect.size.height, _fullWidth, _fullHeight - _screenRect.size.height);
		if (_keyboardView == nil) {
			_keyboardView = [[SoftKeyboard alloc] initWithFrame:keyFrame];
			[_keyboardView setInputDelegate:self];
		}
		
		[self addSubview:[_keyboardView inputView]];
		[self addSubview: _keyboardView];
		[[_keyboardView inputView] becomeFirstResponder];
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
	//printf("mouseDown()\n");
	CGRect rect = GSEventGetLocationInWindow(event);
	CGPoint point = CGPointMake(rect.origin.x, rect.origin.y);
	
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

- (void)touchesBegan {
	//printf("touchesBegan()\n");	
}
	
- (void)mouseUp:(GSEvent*)event {
	//printf("mouseUp()\n");
	CGRect rect = GSEventGetLocationInWindow(event);
	CGPoint point = CGPointMake(rect.origin.x, rect.origin.y);
	
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
	CGRect rect = GSEventGetLocationInWindow(event);
	CGPoint point = CGPointMake(rect.origin.x, rect.origin.y);
	
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
	CGRect rect = GSEventGetLocationInWindow(event);
	CGPoint point = CGPointMake(rect.origin.x, rect.origin.y);

	
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

- (void)mouseExited:(GSEvent*)event {
	//printf("mouseExited().\n");
	// [self addEvent:
	//	[[NSDictionary alloc] initWithObjectsAndKeys:
	//	 @"mouseExited", @"type",
	//	 nil
	//	]
	// ];
}

- (void)mouseMoved:(GSEvent*)event
{
	//printf("mouseMoved()\n");
	// struct CGPoint point = GSEventGetLocationInWindow(event);
	// 
	// if (!getLocalMouseCoords(&point))
	// 	return;
	// 
	// [self addEvent:
	// 	[[NSDictionary alloc] initWithObjectsAndKeys:
	// 	 [NSNumber numberWithInt:kInputMouseSecondToggled], @"type",
	// 	 [NSNumber numberWithFloat:point.x], @"x",
	// 	 [NSNumber numberWithFloat:point.y], @"y",
	// 	 nil
	// 	]
	// ];
}

- (void)handleKeyPress:(unichar)c {
	[self addEvent:
		[[NSDictionary alloc] initWithObjectsAndKeys:
		 [NSNumber numberWithInt:kInputKeyPressed], @"type",
		 [NSNumber numberWithFloat:(float)c], @"x",
		 [NSNumber numberWithFloat:0], @"y",
		 nil
		]
	];
}

- (BOOL)canHandleSwipes {
	return TRUE;
}

- (int)swipe:(int)num withEvent:(GSEvent*)event {
	//printf("swipe: %i\n", num);

	[self addEvent:
		[[NSDictionary alloc] initWithObjectsAndKeys:
		 [NSNumber numberWithInt:kInputSwipe], @"type",
		 [NSNumber numberWithFloat:(float)num], @"x",
		 [NSNumber numberWithFloat:0], @"y",
		 nil
		]
	];
}

- (void)view:(UIView *)view handleTapWithCount:(int)count event:(GSEvent *)event fingerCount:(int)fingerCount{
	//printf("handleTapWithCount(%i, %i)\n", count, fingerCount);
}

- (void)applicationSuspend {
	[self addEvent:
		[[NSDictionary alloc] initWithObjectsAndKeys:
		 [NSNumber numberWithInt:kInputApplicationSuspended], @"type",
		 [NSNumber numberWithFloat:0], @"x",
		 [NSNumber numberWithFloat:0], @"y",
		 nil
		]
	];
}

- (void)applicationResume {
	[self addEvent:
		[[NSDictionary alloc] initWithObjectsAndKeys:
		 [NSNumber numberWithInt:kInputApplicationResumed], @"type",
		 [NSNumber numberWithFloat:0], @"x",
		 [NSNumber numberWithFloat:0], @"y",
		 nil
		]
	];
}

@end

