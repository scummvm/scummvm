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

	return self;
}

- (void)mouseDidConnect:(NSNotification *)notification {
#ifdef __IPHONE_14_0
	[self setIsConnected:YES];
	_mouse = (GCMouse*)notification.object;

	_mouse.mouseInput.mouseMovedHandler = ^(GCMouseInput * _Nonnull mouse, float deltaX, float deltaY) {
		CGPoint newPosition = [[self view] pointerPosition];
		newPosition.x += deltaX;
		newPosition.y += 0-deltaY;
		[self handlePointerMoveTo:newPosition];
	};

	_mouse.mouseInput.leftButton.valueChangedHandler = ^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
		[self handleMouseButtonAction:kGameControllerMouseButtonLeft isPressed:pressed at:[[self view] pointerPosition]];
	};

	_mouse.mouseInput.rightButton.valueChangedHandler = ^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
		[self handleMouseButtonAction:kGameControllerMouseButtonRight isPressed:pressed at:[[self view] pointerPosition]];
	};
#endif
}

@end
