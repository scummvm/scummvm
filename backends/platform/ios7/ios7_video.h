/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

#include "common/list.h"
#include "graphics/scaler.h"

typedef struct {
	GLfloat x, y;
	GLfloat u,v;
} GLVertex;

@interface iPhoneView : UIView {
	VideoContext _videoContext;

	Common::List<InternalEvent> _events;
	NSLock *_eventLock;
	SoftKeyboard *_keyboardView;
	BOOL _keyboardVisible;

	UIBackgroundTaskIdentifier _backgroundSaveStateTask;

	EAGLContext *_context;
	GLuint _viewRenderbuffer;
	GLuint _viewFramebuffer;
	GLuint _screenTexture;
	GLuint _overlayTexture;
	GLuint _mouseCursorTexture;

	GLuint _vertexShader;
	GLuint _fragmentShader;

	GLuint _vertexBuffer;

	GLuint _screenSizeSlot;
	GLuint _textureSlot;
	GLuint _shakeXSlot;
	GLuint _shakeYSlot;

	GLuint _positionSlot;
	GLuint _textureCoordSlot;

	GLint _renderBufferWidth;
	GLint _renderBufferHeight;

	GLVertex _gameScreenCoords[4];
	CGRect _gameScreenRect;

	GLVertex _overlayCoords[4];
	CGRect _overlayRect;

	GLVertex _mouseCoords[4];

	GLint _mouseHotspotX, _mouseHotspotY;
	GLint _mouseWidth, _mouseHeight;
	GLfloat _mouseScaleX, _mouseScaleY;

	int _scaledShakeXOffset;
	int _scaledShakeYOffset;

	UITouch *_firstTouch;
	UITouch *_secondTouch;

#ifdef ENABLE_IOS7_SCALERS
	uint8_t *_scalerMemorySrc;
	uint8_t *_scalerMemoryDst;
	size_t _scalerMemorySrcSize;
	size_t _scalerMemoryDstSize;
	int _scalerScale;
	ScalerProc *_scaler;
#endif
}

- (id)initWithFrame:(struct CGRect)frame;

- (VideoContext *)getVideoContext;

- (void)createScreenTexture;
- (void)initSurface;
- (void)setViewTransformation;

- (void)setGraphicsMode;

- (void)updateSurface;
- (void)updateMainSurface;
- (void)updateOverlaySurface;
- (void)updateMouseSurface;
- (void)clearColorBuffer;

- (void)notifyMouseMove;
- (void)updateMouseCursorScaling;
- (void)updateMouseCursor;

- (void)deviceOrientationChanged:(UIDeviceOrientation)orientation;

- (void)showKeyboard;
- (void)hideKeyboard;
- (BOOL)isKeyboardShown;

- (void)applicationSuspend;
- (void)applicationResume;

- (void)saveApplicationState;
- (void)clearApplicationState;
- (void)restoreApplicationState;

- (void) beginBackgroundSaveStateTask;
- (void) endBackgroundSaveStateTask;

- (bool)fetchEvent:(InternalEvent *)event;

@end

#endif
