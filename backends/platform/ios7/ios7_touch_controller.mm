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
	UITouch *_touch;
}

@dynamic view;
@dynamic isConnected;

- (id)initWithView:(iPhoneView *)view {
	self = [super initWithView:view];

	_touch = nil;
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
	return touch != nil && (touch.type == UITouchTypeDirect || touch.type == UITouchTypePencil);
#else
	return YES;
#endif
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
	_touch = [touches anyObject];

	if ([self shouldHandleTouch:_touch]) {
		CGPoint p = [self getLocationInView:_touch];
		[[self view] addEvent:InternalEvent(kInputTouchBegan, p.x, p.y)];
	}
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
	UITouch *t = [touches anyObject];

	if (t != _touch) {
		// We shouldn't end up here but if we do bail out
		return;
	}
	_touch = t;
	if ([self shouldHandleTouch:_touch]) {
		CGPoint p = [self getLocationInView:_touch];
		[[self view] addEvent:InternalEvent(kInputTouchMoved, p.x, p.y)];
	}
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
	_touch = nil;
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
	_touch = nil;
}

@end
