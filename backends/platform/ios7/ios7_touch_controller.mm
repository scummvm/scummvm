/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/platform/ios7/ios7_touch_controller.h"
#include "backends/platform/ios7/ios7_video.h"

@implementation TouchController {
	UITouch *_firstTouch;
	UITouch *_secondTouch;
}

@dynamic view;
@dynamic isConnected;

- (id)initWithView:(iPhoneView *)view {
	self = [super initWithView:view];

	_firstTouch = NULL;
	_secondTouch = NULL;

	// Touches should always be present in iOS view
	[self setIsConnected:YES];

	return self;
}

- (UITouch *)secondTouchOtherTouchThan:(UITouch *)touch in:(NSSet *)set {
	NSArray *all = [set allObjects];
	for (UITouch *t in all) {
		if (t != touch) {
			return t;
		}
	}
	return nil;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
	NSSet *allTouches = [event allTouches];
	if (allTouches.count == 1) {
		_firstTouch = [allTouches anyObject];
		if (_firstTouch.type == UITouchTypeDirect) {
			// Move the pointer to the new position
			[self handlePointerMoveTo:[_firstTouch locationInView: [self view]]];
			[self handleMouseButtonAction:kGameControllerMouseButtonLeft isPressed:YES at:[_firstTouch locationInView:[self view]]];
		}
	} else if (allTouches.count == 2) {
		_secondTouch = [self secondTouchOtherTouchThan:_firstTouch in:allTouches];
		if (_secondTouch && _secondTouch.type == UITouchTypeDirect) {
			[self handleMouseButtonAction:kGameControllerMouseButtonRight isPressed:YES at:[_firstTouch locationInView:[self view]]];
		}
	}
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
	NSSet *allTouches = [event allTouches];
	for (UITouch *touch in allTouches) {
		if (touch == _firstTouch ||
			touch == _secondTouch) {
			if (touch.type == UITouchTypeDirect) {
				[self handlePointerMoveTo:[touch locationInView: [self view]]];
			}
		}
	}
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
	NSSet *allTouches = [event allTouches];
	if (allTouches.count == 1) {
		UITouch *touch = [allTouches anyObject];
		if (touch.type == UITouchTypeDirect) {
			[self handleMouseButtonAction:kGameControllerMouseButtonLeft isPressed:NO at:[touch locationInView:[self view]]];
		}
	} else if (allTouches.count == 2) {
		UITouch *touch = [[allTouches allObjects] objectAtIndex:1];
		if (touch.type == UITouchTypeDirect) {
			[self handleMouseButtonAction:kGameControllerMouseButtonRight isPressed:NO at:[touch locationInView:[self view]]];
		}
	}
	_firstTouch = nil;
	_secondTouch = nil;
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
	_firstTouch = nil;
	_secondTouch = nil;
}

@end
