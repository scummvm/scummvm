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

#ifndef BACKENDS_PLATFORM_IOS7_IOS7_VIDEO_H
#define BACKENDS_PLATFORM_IOS7_IOS7_VIDEO_H

#include <UIKit/UIKit.h>
#include <Foundation/Foundation.h>
#include <QuartzCore/QuartzCore.h>

#include <OpenGLES/EAGL.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "backends/platform/ios7/ios7_keyboard.h"
#include "backends/platform/ios7/ios7_common.h"
#include "backends/platform/ios7/ios7_game_controller.h"

#include "common/list.h"

typedef struct {
	GLfloat x, y;
	GLfloat u,v;
} GLVertex;

uint getSizeNextPOT(uint size);

@interface iPhoneView : UIView {
	Common::List<InternalEvent> _events;
	NSLock *_eventLock;
	SoftKeyboard *_keyboardView;
	Common::List<GameController*> _controllers;
#if TARGET_OS_IOS
	UIInterfaceOrientation _currentOrientation;
#endif
	UIBackgroundTaskIdentifier _backgroundSaveStateTask;

	EAGLContext *_mainContext;
	EAGLContext *_openGLContext;
	GLuint _viewRenderbuffer;

	GLint _renderBufferWidth;
	GLint _renderBufferHeight;
}

@property (nonatomic, assign) BOOL isInGame;
@property (nonatomic, assign) UIInterfaceOrientationMask supportedScreenOrientations;

- (id)initWithFrame:(struct CGRect)frame;

- (uint)createOpenGLContext;
- (void)destroyOpenGLContext;
- (void)refreshScreen;
- (int)getScreenWidth;
- (int)getScreenHeight;

- (void)initSurface;

#if TARGET_OS_IOS
- (void)interfaceOrientationChanged:(UIInterfaceOrientation)orientation;
#endif

- (void)showKeyboard;
- (void)hideKeyboard;
- (BOOL)isKeyboardShown;

- (void)handleMainMenuKey;

- (void)applicationSuspend;
- (void)applicationResume;

- (void)saveApplicationState;
- (void)clearApplicationState;
- (void)restoreApplicationState;

- (void) beginBackgroundSaveStateTask;
- (void) endBackgroundSaveStateTask;

- (void)addEvent:(InternalEvent)event;
- (bool)fetchEvent:(InternalEvent *)event;

- (BOOL)isTouchControllerConnected;
- (BOOL)isMouseControllerConnected;
- (BOOL)isGamepadControllerConnected;
- (void)virtualController:(bool)connect;
@end

#endif
