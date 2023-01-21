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

#include "common/events.h"
#include "backends/platform/ios7/ios7_gamepad_controller.h"
#include "backends/platform/ios7/ios7_video.h"
#include <GameController/GameController.h>

@implementation GamepadController {
	GCController *_controller;
}

@dynamic view;
@dynamic isConnected;

- (id)initWithView:(iPhoneView *)view {
	self = [super initWithView:view];

	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(controllerDidConnect:)
												 name:@"GCControllerDidConnectNotification"
											   object:nil];

	return self;
}

- (void)controllerDidConnect:(NSNotification *)notification {
	_controller = (GCController*)notification.object;

#if TARGET_OS_TV
	if (_controller.microGamepad != nil) {
		[self setIsConnected:YES];

		_controller.microGamepad.buttonA.valueChangedHandler = ^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
			[self handleJoystickButtonAction:Common::JOYSTICK_BUTTON_A isPressed:pressed];
		};

		_controller.microGamepad.buttonX.valueChangedHandler = ^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
			// Map button X to button B because B is mapped to left button
			[self handleJoystickButtonAction:Common::JOYSTICK_BUTTON_B isPressed:pressed];
		};

		if (@available(iOS 13.0, tvOS 13.0, *)) {
			_controller.microGamepad.buttonMenu.valueChangedHandler = ^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
				[self handleMainMenu:pressed];
			};
		}
	}
#endif
	if (_controller.extendedGamepad != nil) {
		[self setIsConnected:YES];

		_controller.extendedGamepad.leftThumbstick.valueChangedHandler = ^(GCControllerDirectionPad * _Nonnull dpad, float xValue, float yValue) {
			// Convert the given axis values in float (-1 to 1) to ScummVM Joystick
			// Axis value as integers (0 to int16_max)
			int x = xValue * (float)Common::JOYAXIS_MAX;
			int y = yValue * (float)Common::JOYAXIS_MAX;

			// Apple's Y values are reversed from ScummVM's
			[self handleJoystickAxisMotionX:x andY:0-y forJoystick:kGameControllerJoystickLeft];
		};

		_controller.extendedGamepad.rightThumbstick.valueChangedHandler = ^(GCControllerDirectionPad * _Nonnull dpad, float xValue, float yValue) {
			// Convert the given axis values in float (-1 to 1) to ScummVM Joystick
			// Axis value as integers (0 to int16_max)
			int x = xValue * (float)Common::JOYAXIS_MAX;
			int y = yValue * (float)Common::JOYAXIS_MAX;

			// Apple's Y values are reversed from ScummVM's
			[self handleJoystickAxisMotionX:x andY:0-y forJoystick:kGameControllerJoystickRight];
		};

		_controller.extendedGamepad.buttonA.valueChangedHandler = ^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
			[self handleJoystickButtonAction:Common::JOYSTICK_BUTTON_A isPressed:pressed];
		};

		_controller.extendedGamepad.buttonB.valueChangedHandler = ^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
			[self handleJoystickButtonAction:Common::JOYSTICK_BUTTON_B isPressed:pressed];
		};

		_controller.extendedGamepad.buttonX.valueChangedHandler = ^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
			[self handleJoystickButtonAction:Common::JOYSTICK_BUTTON_X isPressed:pressed];
		};

		_controller.extendedGamepad.buttonY.valueChangedHandler = ^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
			[self handleJoystickButtonAction:Common::JOYSTICK_BUTTON_Y isPressed:pressed];
		};
#ifdef __IPHONE_12_1
		_controller.extendedGamepad.leftThumbstickButton.valueChangedHandler = ^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
			[self handleJoystickButtonAction:Common::JOYSTICK_BUTTON_LEFT_STICK isPressed:pressed];
		};

		_controller.extendedGamepad.rightThumbstickButton.valueChangedHandler = ^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
			[self handleJoystickButtonAction:Common::JOYSTICK_BUTTON_RIGHT_STICK isPressed:pressed];
		};
#endif
		_controller.extendedGamepad.leftShoulder.valueChangedHandler = ^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
			[self handleJoystickButtonAction:Common::JOYSTICK_BUTTON_LEFT_SHOULDER isPressed:pressed];
		};

		_controller.extendedGamepad.rightShoulder.valueChangedHandler = ^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
			[self handleJoystickButtonAction:Common::JOYSTICK_BUTTON_RIGHT_SHOULDER isPressed:pressed];
		};

#ifdef __IPHONE_13_0
		if (@available(iOS 13.0, tvOS 13.0, *)) {
			_controller.extendedGamepad.buttonMenu.valueChangedHandler = ^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
				[self handleMainMenu:pressed];
			};
		}
#endif
	}
}

@end
