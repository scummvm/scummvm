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

#include "backends/platform/ios7/ios7_keyboard.h"

@interface UITextInputTraits
- (void)setAutocorrectionType:(int)type;
- (void)setAutocapitalizationType:(int)type;
- (void)setEnablesReturnKeyAutomatically:(BOOL)val;
@end

@interface TextInputHandler : UITextView {
	SoftKeyboard *softKeyboard;
}

- (id)initWithKeyboard:(SoftKeyboard *)keyboard;

@end


@implementation TextInputHandler

- (id)initWithKeyboard:(SoftKeyboard *)keyboard {
	self = [super initWithFrame:CGRectMake(0.0f, 0.0f, 0.0f, 0.0f)];
	softKeyboard = keyboard;

	[self setAutocorrectionType:UITextAutocorrectionTypeNo];
	[self setAutocapitalizationType:UITextAutocapitalizationTypeNone];
	[self setEnablesReturnKeyAutomatically:NO];
	UITextInputAssistantItem* item = [self inputAssistantItem];
	item.leadingBarButtonGroups = @[];
	item.trailingBarButtonGroups = @[];

	return self;
}

@end


@implementation SoftKeyboard

- (id)initWithFrame:(CGRect)frame {
	self = [super initWithFrame:frame];
	inputDelegate = nil;
	inputView = [[TextInputHandler alloc] initWithKeyboard:self];
	inputView.delegate = self;
	return self;
}

- (NSArray *)keyCommands {
	UIKeyCommand *upArrow = [UIKeyCommand keyCommandWithInput: UIKeyInputUpArrow modifierFlags: 0 action: @selector(upArrow:)];
	UIKeyCommand *downArrow = [UIKeyCommand keyCommandWithInput: UIKeyInputDownArrow modifierFlags: 0 action: @selector(downArrow:)];
	UIKeyCommand *leftArrow = [UIKeyCommand keyCommandWithInput: UIKeyInputLeftArrow modifierFlags: 0 action: @selector(leftArrow:)];
	UIKeyCommand *rightArrow = [UIKeyCommand keyCommandWithInput: UIKeyInputRightArrow modifierFlags: 0 action: @selector(rightArrow:)];
	return [[NSArray alloc] initWithObjects: upArrow, downArrow, leftArrow, rightArrow, nil];
}

- (void) upArrow: (UIKeyCommand *) keyCommand {
//	[self resignFirstResponder];
	[softKeyboard handleKeyPress:273];
}

- (void) downArrow: (UIKeyCommand *) keyCommand {
	[softKeyboard handleKeyPress:274];
}

- (void) leftArrow: (UIKeyCommand *) keyCommand {
	[softKeyboard handleKeyPress:276];
}

- (void) rightArrow: (UIKeyCommand *) keyCommand {
	[softKeyboard handleKeyPress:275];
}

- (BOOL)textView:(UITextView *)textView shouldChangeTextInRange:(NSRange)range replacementText:(NSString *)text {
	unichar c;
	if (text.length) {
		c = [text characterAtIndex:0];
	}
	else {
		c = '\b';
	}
	[inputDelegate handleKeyPress:c];
	return YES;
}

- (UITextView *)inputView {
	return inputView;
}

- (void)setInputDelegate:(id)delegate {
	inputDelegate = delegate;
}

- (void)handleKeyPress:(unichar)c {
	[inputDelegate handleKeyPress:c];
}

- (void)showKeyboard {
	[inputView becomeFirstResponder];
}

- (void)hideKeyboard {
	[inputView endEditing:YES];
}

@end
