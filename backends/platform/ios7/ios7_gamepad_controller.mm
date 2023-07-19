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
#include "common/config-manager.h"
#include "backends/platform/ios7/ios7_gamepad_controller.h"
#include "backends/platform/ios7/ios7_video.h"
#include <GameController/GameController.h>

@implementation GamepadController {
	GCController *_controller;
#if TARGET_OS_IOS
#ifdef __IPHONE_15_0
	API_AVAILABLE(ios(15.0))
	GCVirtualController *_virtualController;
	API_AVAILABLE(ios(15.0))
	GCVirtualControllerConfiguration *_config;
#endif
#endif
}

@dynamic view;
@dynamic isConnected;

- (id)initWithView:(iPhoneView *)view {
	self = [super initWithView:view];

	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(controllerDidConnect:)
												 name:@"GCControllerDidConnectNotification"
											   object:nil];

#if TARGET_OS_IOS
#ifdef __IPHONE_15_0
	if (@available(iOS 15.0, *)) {
		// Configure a simple game controller with dPad and A and B buttons
		_config = [[GCVirtualControllerConfiguration alloc] init];
		_config.elements = [[NSSet alloc] initWithObjects:GCInputDirectionPad, GCInputButtonA, GCInputButtonB, GCInputButtonX, GCInputButtonY, nil];
		_virtualController = [[GCVirtualController alloc] initWithConfiguration:_config];
	}
#endif
#endif
	return self;
}

// Undocumented way to retreive the GCControllerView.
// Drill down the layer structure to get the GCControllerView.
// The view layers for iPhones are:
// - TransitionView
//   - DropShadowView
//     - LayoutContainerView
//       - ContainerView
// iPads have an additional layer under the ContainerView
- (BOOL)setGCControllerViewProperties:(NSArray<UIView*>*)subviews {
	BOOL stop = NO;
	for (UIView *view in subviews) {
		if ([[view classForCoder] isEqual:NSClassFromString(@"GCControllerView")]) {
			// Set the virtual controller frame to full screen.
			// Else buttons can be placed partly out of the frame
			// due to the iPhoneView frame is adjusted according
			// to the safe area insets.
			// Also set the frame alpha to the user specified value
			// to make the virtual controller more transparent
			view.alpha = ((float)ConfMan.getInt("onscreen_control_opacity") / 10.0);
			view.frame = [[UIScreen mainScreen] bounds];
			stop = YES;
		} else {
			// Keep drilling
			stop = [self setGCControllerViewProperties:view.subviews];
		}
		if (stop) {
			break;
		}
	}
	return stop;
}

- (void)virtualController:(bool)connect {
#if TARGET_OS_IOS
#ifdef __IPHONE_15_0
	if (@available(iOS 15.0, *)) {
		if (connect) {
			[_virtualController connectWithReplyHandler:^(NSError * _Nullable error) {
				[self setGCControllerViewProperties:[[[UIApplication sharedApplication] keyWindow] subviews]];
			}];
		}
		else {
			[_virtualController disconnect];
			[self setIsConnected:NO];
		}
	}
#endif
#endif
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

		_controller.extendedGamepad.dpad.valueChangedHandler = ^(GCControllerDirectionPad * _Nonnull dpad, float xValue, float yValue) {
			// Negative values are left/down, positive are right/up, 0 is no press
			[self handleJoystickButtonAction:Common::JOYSTICK_BUTTON_DPAD_LEFT isPressed:(xValue < 0)];
			[self handleJoystickButtonAction:Common::JOYSTICK_BUTTON_DPAD_RIGHT isPressed:(xValue > 0)];
			[self handleJoystickButtonAction:Common::JOYSTICK_BUTTON_DPAD_UP isPressed:(yValue > 0)];
			[self handleJoystickButtonAction:Common::JOYSTICK_BUTTON_DPAD_DOWN isPressed:(yValue < 0)];
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
		if (@available(iOS 12.1, tvOS 12.1, *)) {
			_controller.extendedGamepad.leftThumbstickButton.valueChangedHandler = ^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
				[self handleJoystickButtonAction:Common::JOYSTICK_BUTTON_LEFT_STICK isPressed:pressed];
			};
		}

		if (@available(iOS 12.1, tvOS 12.1, *)) {
			_controller.extendedGamepad.rightThumbstickButton.valueChangedHandler = ^(GCControllerButtonInput * _Nonnull button, float value, BOOL pressed) {
				[self handleJoystickButtonAction:Common::JOYSTICK_BUTTON_RIGHT_STICK isPressed:pressed];
			};
		}
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
