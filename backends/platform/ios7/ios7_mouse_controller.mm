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

#include "backends/platform/ios7/ios7_mouse_controller.h"
#include "backends/platform/ios7/ios7_video.h"
#include <GameController/GameController.h>

@implementation MouseController {
#ifdef __IPHONE_14_0
	GCMouse *_mouse;
	CGFloat _dxReminder, _dyReminder;
#endif
}

@dynamic view;
@dynamic isConnected;

- (id)initWithView:(iPhoneView *)view {
	self = [super initWithView:view];

	if (self) {
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(mouseDidConnect:)
													 name:@"GCMouseDidConnectNotification"
												   object:nil];
	}

	_dxReminder = 0.0;
	_dyReminder = 0.0;

	return self;
}

- (void)mouseDidConnect:(NSNotification *)notification {
#ifdef __IPHONE_14_0
	[self setIsConnected:YES];
	_mouse = (GCMouse*)notification.object;

	_mouse.mouseInput.mouseMovedHandler = ^(GCMouseInput * _Nonnull mouse, float deltaX, float deltaY) {
		CGFloat scaledDeltaX = deltaX * [[self view] contentScaleFactor] + _dxReminder;
		CGFloat scaledDeltaY = deltaY * [[self view] contentScaleFactor] + _dyReminder;
		// Add any reminding delta values to be summed up and get the integer part of the delta
		int dx = (int)(scaledDeltaX);
		int dy = (int)(scaledDeltaY);
		// Save the new reminders
		_dxReminder = scaledDeltaX - (CGFloat)dx;
		_dyReminder = scaledDeltaY - (CGFloat)dy;

		[[self view] addEvent:InternalEvent(kInputMouseDelta, -dx, dy)];
	};

	_mouse.mouseInput.leftButton.valueChangedHandler = ^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
		[[self view] addEvent:InternalEvent(pressed ? kInputMouseLeftButtonDown : kInputMouseLeftButtonUp, 0, 0)];
	};

	_mouse.mouseInput.rightButton.valueChangedHandler = ^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
		[[self view] addEvent:InternalEvent(pressed ? kInputMouseRightButtonDown : kInputMouseRightButtonUp, 0, 0)];
	};
#endif
}

@end
