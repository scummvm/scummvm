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

- (BOOL)shouldHandleTouch:(UITouch *)touch {
	// In iOS touchpads will trigger UITouchTypeIndirect events
	// However, they will also send mose events. Make sure to
	// block the UITouchTypeIndirect but not in Apple TV OS where
	// they are required as the Apple TV remote sends touh events
	// but no mouse events.
#if TARGET_OS_IOS
	return touch.type == UITouchTypeDirect;
#else
	return YES;
#endif
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
	switch (allTouches.count) {
	case 1: {
		_firstTouch = [allTouches anyObject];
		if ([self shouldHandleTouch:_firstTouch]) {
			CGPoint p = [self getLocationInView:_firstTouch];
			[[self view] addEvent:InternalEvent(kInputTouchFirstDown, p.x, p.y)];
		}
		break;
	}
	case 2: {
		_secondTouch = [self secondTouchOtherTouchThan:_firstTouch in:allTouches];
		if (_secondTouch && [self shouldHandleTouch:_secondTouch]) {
			CGPoint p = [self getLocationInView:_firstTouch];
			[[self view] addEvent:InternalEvent(kInputTouchSecondDown, p.x, p.y)];
		}
		break;
	}
	default:
		break;
	}
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
	NSSet *allTouches = [event allTouches];
	for (UITouch *touch in allTouches) {
		if ([self shouldHandleTouch:touch]) {
			if (touch == _firstTouch) {
				CGPoint p = [self getLocationInView:touch];
				[[self view] addEvent:InternalEvent(kInputTouchFirstDragged , p.x, p.y)];
			} else if (touch == _secondTouch) {
				CGPoint p = [self getLocationInView:touch];
				[[self view] addEvent:InternalEvent(kInputTouchSecondDragged , p.x, p.y)];
			}
		}
	}
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
	NSSet *allTouches = [event allTouches];
	switch (allTouches.count) {
	case 1: {
		_firstTouch = [allTouches anyObject];
		if ([self shouldHandleTouch:_firstTouch]) {
			CGPoint p = [self getLocationInView:_firstTouch];
			[[self view] addEvent:InternalEvent(kInputTouchFirstUp, p.x, p.y)];
		}
		break;
	}
	case 2: {
		_secondTouch = [self secondTouchOtherTouchThan:_firstTouch in:allTouches];
		if (_secondTouch && [self shouldHandleTouch:_secondTouch]) {
			CGPoint p = [self getLocationInView:_firstTouch];
			[[self view] addEvent:InternalEvent(kInputTouchSecondUp, p.x, p.y)];
		}
		break;
	}
	default:
		break;
	}
	_firstTouch = nil;
	_secondTouch = nil;
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
	_firstTouch = nil;
	_secondTouch = nil;
}

@end
