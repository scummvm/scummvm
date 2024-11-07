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
#include "backends/platform/ios7/ios7_game_controller.h"
#include "backends/platform/ios7/ios7_video.h"

@interface GameController()
@property (nonatomic, assign) BOOL firstButtonPressed;
@property (nonatomic, assign) BOOL secondButtonPressed;
@end

@implementation GameController

@synthesize view;

- (id)initWithView:(iPhoneView *)v {
	self = [super init];
	if (self) {
		[self setView:v];
	}
	_firstButtonPressed = _secondButtonPressed = NO;
	return self;
}

- (CGPoint)getLocationInView:(UITouch *)touch {
	CGPoint p = [touch locationInView:[self view]];
	p.x *= [[self view] contentScaleFactor];
	p.y *= [[self view] contentScaleFactor];
	return p;
}

- (void)handleJoystickAxisMotionX:(int)x andY:(int)y forJoystick:(GameControllerJoystick)joystick {
	if (joystick == kGameControllerJoystickLeft) {
		[view addEvent:InternalEvent(kInputJoystickAxisMotion, Common::JOYSTICK_AXIS_LEFT_STICK_X, x)];
		[view addEvent:InternalEvent(kInputJoystickAxisMotion, Common::JOYSTICK_AXIS_LEFT_STICK_Y, y)];
	} else {
		[view addEvent:InternalEvent(kInputJoystickAxisMotion, Common::JOYSTICK_AXIS_RIGHT_STICK_X, x)];
		[view addEvent:InternalEvent(kInputJoystickAxisMotion, Common::JOYSTICK_AXIS_RIGHT_STICK_Y, y)];
	}
}

- (void)handleJoystickButtonAction:(int)button isPressed:(bool)pressed {
	bool addEvent = true;
	if (button == Common::JOYSTICK_BUTTON_A) {
		if (_firstButtonPressed) {
			if (pressed) {
				addEvent = false;
			}
		}
		_firstButtonPressed = pressed;
	} else if (button == Common::JOYSTICK_BUTTON_B) {
		if (_secondButtonPressed) {
			if (pressed) {
				addEvent = false;
			}
		}
		_secondButtonPressed = pressed;
	}
	// Do not send button presses if keyboard is shown because if e.g.
	// the "Do you want to quit?" dialog is shown the wait for user
	// input will end (treating the button push as a mouse click) while
	// the user tried to select the "y" or "n" character on the tvOS
	// keyboard.
	if (addEvent && ![view isKeyboardShown]) {
		[view addEvent:InternalEvent(pressed ? kInputJoystickButtonDown : kInputJoystickButtonUp, button, 0)];
	}
}

- (void)handleMainMenu:(BOOL)pressed {
	if (!pressed) { // released
		[view handleMainMenuKey];
	}
}
@end
