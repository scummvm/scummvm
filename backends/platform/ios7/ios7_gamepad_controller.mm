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
#include "backends/platform/ios7/ios7_app_delegate.h"
#include <GameController/GameController.h>

@implementation GamepadController {
	GCController *_controller;
#if TARGET_OS_IOS
#ifdef __IPHONE_15_0
	API_AVAILABLE(ios(15.0))
	GCVirtualController *_virtualControllerThumbstick;
	API_AVAILABLE(ios(15.0))
	GCVirtualController *_virtualControllerMiniThumbstick;
	API_AVAILABLE(ios(15.0))
	GCVirtualController *_virtualControllerDpad;
	API_AVAILABLE(ios(15.0))
	GCVirtualController *_virtualControllerMiniDpad;
	API_AVAILABLE(ios(15.0))
	GCVirtualController *_currentController;
	API_AVAILABLE(ios(15.0))
	GCVirtualControllerConfiguration *_configDpad;
	API_AVAILABLE(ios(15.0))
	GCVirtualControllerConfiguration *_configMiniDpad;
	API_AVAILABLE(ios(15.0))
	GCVirtualControllerConfiguration *_configThumbstick;
	API_AVAILABLE(ios(15.0))
	GCVirtualControllerConfiguration *_configMiniThumbstick;
#endif
#endif
	int _currentDpadXValue;
	int _currentDpadYValue;
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
		_configDpad = [[GCVirtualControllerConfiguration alloc] init];
		_configMiniDpad = [[GCVirtualControllerConfiguration alloc] init];
		_configThumbstick = [[GCVirtualControllerConfiguration alloc] init];
		_configMiniThumbstick = [[GCVirtualControllerConfiguration alloc] init];

		NSArray<NSString *> *_commonElements = [[NSArray alloc] initWithObjects: GCInputButtonA, GCInputButtonB, GCInputButtonX, GCInputButtonY, nil];
		NSArray<NSString *> *_additionalElements = [[NSArray alloc] initWithObjects: GCInputRightThumbstick, GCInputLeftShoulder, GCInputRightShoulder, nil];

		NSMutableSet<NSString *> *_fullSetElementsThumbstick = [[NSMutableSet alloc] initWithObjects: GCInputLeftThumbstick, nil];
		[_fullSetElementsThumbstick addObjectsFromArray:_commonElements];
		[_fullSetElementsThumbstick addObjectsFromArray:_additionalElements];
	
		NSMutableSet<NSString *> *_miniSetElementsThumbstick = [[NSMutableSet alloc] initWithObjects:GCInputLeftThumbstick, nil];
		[_miniSetElementsThumbstick addObjectsFromArray:_commonElements];

		NSMutableSet<NSString *> *_fullSetElementsDpad = [[NSMutableSet alloc] initWithObjects: GCInputDirectionalDpad, nil];
		[_fullSetElementsDpad addObjectsFromArray:_commonElements];
		[_fullSetElementsDpad addObjectsFromArray:_additionalElements];

		NSMutableSet<NSString *> *_miniSetElementsDpad = [[NSMutableSet alloc] initWithObjects:GCInputDirectionalDpad, nil];
		[_miniSetElementsDpad addObjectsFromArray:_commonElements];

		_configThumbstick.elements = _fullSetElementsThumbstick;
		_configMiniThumbstick.elements = _miniSetElementsThumbstick;
		_configDpad.elements = _fullSetElementsDpad;
		_configMiniDpad.elements = _miniSetElementsDpad;

		[_miniSetElementsDpad release];
		[_fullSetElementsDpad release];
		[_miniSetElementsThumbstick release];
		[_fullSetElementsThumbstick release];
		[_commonElements release];
		[_additionalElements release];

		_virtualControllerThumbstick = [[GCVirtualController alloc] initWithConfiguration:_configThumbstick];
		_virtualControllerMiniThumbstick = [[GCVirtualController alloc] initWithConfiguration:_configMiniThumbstick];
		_virtualControllerDpad = [[GCVirtualController alloc] initWithConfiguration:_configDpad];
		_virtualControllerMiniDpad = [[GCVirtualController alloc] initWithConfiguration:_configMiniDpad];
		_currentController = _virtualControllerThumbstick;

		[_configDpad release];
		[_configMiniDpad release];
		[_configThumbstick release];
		[_configMiniThumbstick release];
	}
#endif
#endif
	_currentDpadXValue = 0;
	_currentDpadYValue = 0;
	return self;
}

// Undocumented way to retrieve the GCControllerView.
// Drill down the layer structure to get the GCControllerView.
// The view layers for iPhones are:
// - TransitionView
//   - DropShadowView
//     - LayoutContainerView
//       - ContainerView
// iPads have an additional layer under the ContainerView
#if TARGET_OS_IOS
- (BOOL)setGCControllerViewProperties:(NSArray<UIView*>*)subviews {
	BOOL stop = NO;
	for (UIView *view in subviews) {
		if ([[view classForCoder] isEqual:NSClassFromString(@"GCControllerView")]) {
			// Set the frame alpha to the user specified value
			// to make the virtual controller more transparent
			view.alpha = ((float)ConfMan.getInt("gamepad_controller_opacity") / 10.0);

			// Since the iOS7 view controller frame is adjusted for the safe area, the same
			// has to be done for the gamepad controller view. One could think that subviews
			// would adjust automatically but it seems that the gamepad controller buttons
			// can be positioned outside the device screen if not adjusting manually.
			if (@available(iOS 11.0, *)) {
				UIEdgeInsets insets = [[[UIApplication sharedApplication] keyWindow] safeAreaInsets];
				UIInterfaceOrientation orientation = [iOS7AppDelegate currentOrientation];

				// Set anchor point to lower right corner
				view.layer.anchorPoint = CGPointMake(1, 1);

				// Specify the position of the view layer from the anchor point
				if (orientation == UIInterfaceOrientationLandscapeLeft) {
					view.layer.position = CGPointMake(view.frame.size.width, view.layer.position.y);
				} else if (orientation == UIInterfaceOrientationLandscapeRight) {
					// When a device with e.g. a sensor bar is rotated so the sensor bar
					// is to the left, we can adjust the anchor point a bit more to the left
					// to make the left thumb buttons be at the same distance from the screen
					// border.
					view.layer.position = CGPointMake(view.frame.size.width - insets.left, view.layer.position.y);
				}
			}
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
#endif

- (void)virtualController:(bool)connect {
#if TARGET_OS_IOS
#ifdef __IPHONE_15_0
	if (@available(iOS 15.0, *)) {
		GCVirtualController *controller;
		switch (ConfMan.getInt("gamepad_controller_directional_input")) {
		case kDirectionalInputThumbstick:
			controller = ConfMan.getBool("gamepad_controller_minimal_layout") ? _virtualControllerMiniThumbstick : _virtualControllerThumbstick;
			break;
		case kDirectionalInputDpad:
		default:
			controller = ConfMan.getBool("gamepad_controller_minimal_layout") ? _virtualControllerMiniDpad : _virtualControllerDpad;
			break;
		}

		if (_currentController != controller) {
			[_currentController disconnect];
		}

		if (connect) {
			[controller connectWithReplyHandler:^(NSError * _Nullable error) {
				[self setGCControllerViewProperties:[[[UIApplication sharedApplication] keyWindow] subviews]];
			}];
			_currentController = controller;
		}
		else {
			[_currentController disconnect];
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
			// Change xValue to only be -1, 0, or 1
			xValue = xValue < 0.f ? -1.f : xValue > 0.f ? 1.f : 0.f;
			if (xValue != _currentDpadXValue) {
				if (_currentDpadXValue < 0) {
					[self handleJoystickButtonAction:Common::JOYSTICK_BUTTON_DPAD_LEFT isPressed:false];
				} else if (_currentDpadXValue > 0) {
					[self handleJoystickButtonAction:Common::JOYSTICK_BUTTON_DPAD_RIGHT isPressed:false];
				}
				if (xValue < 0) {
					[self handleJoystickButtonAction:Common::JOYSTICK_BUTTON_DPAD_LEFT isPressed:true];
				} else if (xValue > 0) {
					[self handleJoystickButtonAction:Common::JOYSTICK_BUTTON_DPAD_RIGHT isPressed:true];
				}
				_currentDpadXValue = xValue;
			}

			// Change yValue to only be -1, 0, or 1
			yValue = yValue < 0.f ? -1.f : yValue > 0.f ? 1.f : 0.f;
			if (yValue != _currentDpadYValue) {
				if (_currentDpadYValue < 0) {
					[self handleJoystickButtonAction:Common::JOYSTICK_BUTTON_DPAD_DOWN isPressed:false];
				} else if (_currentDpadYValue > 0) {
					[self handleJoystickButtonAction:Common::JOYSTICK_BUTTON_DPAD_UP isPressed:false];
				}
				if (yValue < 0) {
					[self handleJoystickButtonAction:Common::JOYSTICK_BUTTON_DPAD_DOWN isPressed:true];
				} else if (yValue > 0) {
					[self handleJoystickButtonAction:Common::JOYSTICK_BUTTON_DPAD_UP isPressed:true];
				}
				_currentDpadYValue = yValue;
			}
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
