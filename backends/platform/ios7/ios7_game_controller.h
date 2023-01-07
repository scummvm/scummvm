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

#ifndef BACKENDS_PLATFORM_IOS7_IOS7_GAME_CONTROLLER_H
#define BACKENDS_PLATFORM_IOS7_IOS7_GAME_CONTROLLER_H

#include <UIKit/UIKit.h>

@class iPhoneView;

@interface GameController : NSObject

typedef enum {
	kGameControllerMouseButtonLeft = 0,
	kGameControllerMouseButtonRight,
	kGameControllerMouseButtonMiddle,
} GameControllerMouseButton;

typedef enum {
	kGameControllerJoystickLeft = 0,
	kGameControllerJoystickRight
} GameControllerJoystick;


@property (nonatomic, readwrite, retain) iPhoneView *view;
@property (nonatomic, assign) BOOL isConnected;

- (id)initWithView:(iPhoneView *)v;

- (void)handlePointerMoveTo:(CGPoint)point;
- (void)handleMouseButtonAction:(GameControllerMouseButton)button isPressed:(bool)pressed at:(CGPoint)point;
- (void)handleJoystickAxisMotionX:(int)x andY:(int)y forJoystick:(GameControllerJoystick)joystick;
- (void)handleJoystickButtonAction:(int)button isPressed:(bool)pressed;

- (void)handleMainMenu:(BOOL)pressed;
@end

#endif /* BACKENDS_PLATFORM_IOS7_IOS7_GAME_CONTROLLER_H */
