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

#ifndef BACKENDS_PLATFORM_IOS7_IOS7_KEYBOARD_H
#define BACKENDS_PLATFORM_IOS7_IOS7_KEYBOARD_H

#include <UIKit/UIKit.h>
#include <UIKit/UITextView.h>

@class TextInputHandler;

@interface SoftKeyboard : UIView<UITextFieldDelegate> {
	id inputDelegate;
	TextInputHandler *inputView;
}

- (id)initWithFrame:(CGRect)frame;
- (void)dealloc;
- (UITextField *)inputView;
- (void)setInputDelegate:(id)delegate;
- (void)handleKeyPress:(unichar)c;
- (void)handleMainMenuKey;

- (void)showKeyboard;
- (void)hideKeyboard;
- (BOOL)isKeyboardShown;

@end

#endif
