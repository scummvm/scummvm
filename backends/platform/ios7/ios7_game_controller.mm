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

#include "backends/platform/ios7/ios7_game_controller.h"
#include "backends/platform/ios7/ios7_video.h"

@interface GameController()
@property (nonatomic, assign) BOOL firstButtonPressed;
@property (nonatomic, assign) BOOL secondButtonPressed;
@end

@implementation GameController

@synthesize view;

- (id)initWithView:(iPhoneView *)view {
	self = [super init];
	if (self) {
		[self setView:view];
	}
	_firstButtonPressed = _secondButtonPressed = NO;
	return self;
}

- (void)handlePointerMoveTo:(CGPoint)point {
	int x, y;

	// Only set valid mouse coordinates in games
	if (![view getMouseCoords:point eventX:&x eventY:&y]) {
		return;
	}

	[view setPointerPosition:point];

	if (_firstButtonPressed) {
		[view addEvent:InternalEvent(kInputMouseDragged, x, y)];
	} else if (_secondButtonPressed) {
		[view addEvent:InternalEvent(kInputMouseSecondDragged, x, y)];
	} else {
		[view addEvent:InternalEvent(kInputMouseDragged, x, y)];
	}
}

- (void)handleMouseButtonAction:(GameControllerMouseButton)button isPressed:(bool)pressed at:(CGPoint)point{
	int x, y;

	// Only set valid mouse coordinates in games
	if (![view getMouseCoords:[view pointerPosition] eventX:&x eventY:&y]) {
		return;
	}

	[view setPointerPosition:point];

	switch (button) {
	case kGameControllerMouseButtonLeft:
		if (pressed) {
			_firstButtonPressed = YES;
			[view addEvent:InternalEvent(kInputMouseDown, x, y)];
		} else {
			_firstButtonPressed = NO;
			[view addEvent:InternalEvent(kInputMouseUp, x, y)];
		}
		break;

	case kGameControllerMouseButtonRight:
		if (pressed) {
			_secondButtonPressed = YES;
			[view addEvent:InternalEvent(kInputMouseSecondDown, x, y)];
		} else {
			_secondButtonPressed = NO;
			[view addEvent:InternalEvent(kInputMouseSecondUp, x, y)];
		}
		break;

	default:
		break;
	}
}

@end
