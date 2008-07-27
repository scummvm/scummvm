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
 * $URL$
 * $Id$
 *
 */

#ifndef _IPHONE_VIDEO__H
#define _IPHONE_VIDEO__H

#import <UIKit/UIKit.h>
#import <GraphicsServices/GraphicsServices.h>
#import <Foundation/Foundation.h>
#import <CoreSurface/CoreSurface.h>

#import <QuartzCore/QuartzCore.h>
#import "iphone_keyboard.h"

@interface iPhoneView : UIView
{
	CoreSurfaceBufferRef _screenSurface;
	NSMutableArray* _events;
	NSLock* _lock;
	SoftKeyboard* _keyboardView;
	CALayer* _screenLayer;

	int _fullWidth;
	int _fullHeight;
	int _widthOffset;
	int _heightOffset;
}

- (id)initWithFrame:(struct CGRect)frame;

- (void)drawRect:(CGRect)frame;

- (CoreSurfaceBufferRef)getSurface;

- (void)initSurface;

- (void)updateScreenRect:(id)rect;

- (id)getEvent;

- (void)deviceOrientationChanged:(int)orientation;

- (void)applicationSuspend;

- (void)applicationResume;

@end



#endif /* _IPHONE_VIDEO__H */
