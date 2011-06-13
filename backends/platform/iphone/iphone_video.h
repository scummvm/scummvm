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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef _IPHONE_VIDEO__H
#define _IPHONE_VIDEO__H

#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>

#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>

#import "iphone_keyboard.h"

@interface iPhoneView : UIView
{
	void* _screenSurface;
	NSMutableArray* _events;
	SoftKeyboard* _keyboardView;
	CALayer* _screenLayer;

	int _widthOffset;
	int _heightOffset;

	EAGLContext* _context;
	GLuint _viewRenderbuffer;
	GLuint _viewFramebuffer;
	GLint _backingWidth;
	GLint _backingHeight;
	GLint _visibleWidth;
	GLint _visibleHeight;
	GLuint _screenTexture;
	GLuint _overlayTexture;
	GLuint _mouseCursorTexture;
}

- (id)initWithFrame:(struct CGRect)frame;

- (void)drawRect:(CGRect)frame;

- (void *)getSurface;

- (void)initSurface;

- (void)updateSurface;
- (void)updateMainSurface;
- (void)updateOverlaySurface;
- (void)updateMouseSurface;
- (void)clearColorBuffer;

-(void)updateMouseCursor;

- (id)getEvent;

- (void)deviceOrientationChanged:(int)orientation;

- (void)applicationSuspend;

- (void)applicationResume;

@end



#endif /* _IPHONE_VIDEO__H */
