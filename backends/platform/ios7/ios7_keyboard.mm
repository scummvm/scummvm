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
#include "common/keyboard.h"

@interface UITextInputTraits
- (void)setAutocorrectionType:(int)type;
- (void)setAutocapitalizationType:(int)type;
- (void)setEnablesReturnKeyAutomatically:(BOOL)val;
@end

@interface TextInputHandler : UITextView {
	SoftKeyboard *softKeyboard;
	UIToolbar *toolbar;
	UIScrollView *scrollView;
}

- (id)initWithKeyboard:(SoftKeyboard *)keyboard;
- (void)updateToolbarSize;

@end


@implementation TextInputHandler

- (id)initWithKeyboard:(SoftKeyboard *)keyboard {
	self = [super initWithFrame:CGRectMake(0.0f, 0.0f, 0.0f, 0.0f)];
	softKeyboard = keyboard;

	[self setAutocorrectionType:UITextAutocorrectionTypeNo];
	[self setAutocapitalizationType:UITextAutocapitalizationTypeNone];
	[self setEnablesReturnKeyAutomatically:NO];
	//UITextInputAssistantItem* item = [self inputAssistantItem];
	//item.leadingBarButtonGroups = @[];
	//item.trailingBarButtonGroups = @[];

	toolbar = [[[UIToolbar alloc] initWithFrame:CGRectMake(0.0f, 0.0f, 0.0f, 0.0f)] autorelease];
	toolbar.barTintColor = keyboard.backgroundColor;
	toolbar.tintColor = keyboard.tintColor;
	toolbar.translucent = NO;

	toolbar.items = @[
		// GMM button
		[[[UIBarButtonItem alloc] initWithTitle:@"\u2630" style:UIBarButtonItemStylePlain target:self action:@selector(mainMenuKey)] autorelease],
		// Escape key
		[[[UIBarButtonItem alloc] initWithTitle:@"Esc" style:UIBarButtonItemStylePlain target:self action:@selector(escapeKey)] autorelease],
		// Return key
		[[[UIBarButtonItem alloc] initWithTitle:@"\u23ce" style:UIBarButtonItemStylePlain target:self action:@selector(returnKey)] autorelease],
		// Function keys
		[[[UIBarButtonItem alloc] initWithTitle:@"F1" style:UIBarButtonItemStylePlain target:self action:@selector(fn1Key)] autorelease],
		[[[UIBarButtonItem alloc] initWithTitle:@"F2" style:UIBarButtonItemStylePlain target:self action:@selector(fn2Key)] autorelease],
		[[[UIBarButtonItem alloc] initWithTitle:@"F3" style:UIBarButtonItemStylePlain target:self action:@selector(fn3Key)] autorelease],
		[[[UIBarButtonItem alloc] initWithTitle:@"F4" style:UIBarButtonItemStylePlain target:self action:@selector(fn4Key)] autorelease],
		[[[UIBarButtonItem alloc] initWithTitle:@"F5" style:UIBarButtonItemStylePlain target:self action:@selector(fn5Key)] autorelease],
		[[[UIBarButtonItem alloc] initWithTitle:@"F6" style:UIBarButtonItemStylePlain target:self action:@selector(fn6Key)] autorelease],
		[[[UIBarButtonItem alloc] initWithTitle:@"F7" style:UIBarButtonItemStylePlain target:self action:@selector(fn7Key)] autorelease],
		[[[UIBarButtonItem alloc] initWithTitle:@"F8" style:UIBarButtonItemStylePlain target:self action:@selector(fn8Key)] autorelease],
		[[[UIBarButtonItem alloc] initWithTitle:@"F9" style:UIBarButtonItemStylePlain target:self action:@selector(fn9Key)] autorelease],
		[[[UIBarButtonItem alloc] initWithTitle:@"F10" style:UIBarButtonItemStylePlain target:self action:@selector(fn10Key)] autorelease],
		[[[UIBarButtonItem alloc] initWithTitle:@"F11" style:UIBarButtonItemStylePlain target:self action:@selector(fn11Key)] autorelease],
		[[[UIBarButtonItem alloc] initWithTitle:@"F12" style:UIBarButtonItemStylePlain target:self action:@selector(fn12Key)] autorelease],
		// Arrow keys
		[[[UIBarButtonItem alloc] initWithTitle:@"\u2190" style:UIBarButtonItemStylePlain target:self action:@selector(leftArrowKey)] autorelease],
		[[[UIBarButtonItem alloc] initWithTitle:@"\u2191" style:UIBarButtonItemStylePlain target:self action:@selector(upArrowKey)] autorelease],
		[[[UIBarButtonItem alloc] initWithTitle:@"\u2192" style:UIBarButtonItemStylePlain target:self action:@selector(rightArrowKey)] autorelease],
		[[[UIBarButtonItem alloc] initWithTitle:@"\u2193" style:UIBarButtonItemStylePlain target:self action:@selector(downArrowKey)] autorelease],
		// Spacer at the end
		[[[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace target:nil action:nil] autorelease],
	];

	self.inputAccessoryView = toolbar;
	[toolbar sizeToFit];

	scrollView = [[UIScrollView alloc] init];
	scrollView.frame = toolbar.frame;
	scrollView.bounds = toolbar.bounds;
	scrollView.autoresizingMask = toolbar.autoresizingMask;
	scrollView.showsVerticalScrollIndicator = false;
	scrollView.showsHorizontalScrollIndicator = false;
	toolbar.autoresizingMask = UIViewAutoresizingNone;
	[scrollView addSubview:toolbar];
	self.inputAccessoryView = scrollView;

	return self;
}

- (void)updateToolbarSize {
	// We need at least a width of 768 pt for the toolbar. If we add more buttons this may need to be increased.
	toolbar.frame = CGRectMake(0, 0, MAX(768, [[UIScreen mainScreen] bounds].size.width), toolbar.frame.size.height);
	toolbar.bounds = toolbar.frame;
	scrollView.contentSize = toolbar.frame.size;
}

- (NSArray *)keyCommands {
	UIKeyCommand *upArrow = [UIKeyCommand keyCommandWithInput: UIKeyInputUpArrow modifierFlags: 0 action: @selector(upArrow:)];
	UIKeyCommand *downArrow = [UIKeyCommand keyCommandWithInput: UIKeyInputDownArrow modifierFlags: 0 action: @selector(downArrow:)];
	UIKeyCommand *leftArrow = [UIKeyCommand keyCommandWithInput: UIKeyInputLeftArrow modifierFlags: 0 action: @selector(leftArrow:)];
	UIKeyCommand *rightArrow = [UIKeyCommand keyCommandWithInput: UIKeyInputRightArrow modifierFlags: 0 action: @selector(rightArrow:)];
	return [[NSArray alloc] initWithObjects: upArrow, downArrow, leftArrow, rightArrow, nil];
}

- (void) upArrow: (UIKeyCommand *) keyCommand {
	[softKeyboard handleKeyPress:Common::KEYCODE_UP];
}

- (void) downArrow: (UIKeyCommand *) keyCommand {
	[softKeyboard handleKeyPress:Common::KEYCODE_DOWN];
}

- (void) leftArrow: (UIKeyCommand *) keyCommand {
	[softKeyboard handleKeyPress:Common::KEYCODE_LEFT];
}

- (void) rightArrow: (UIKeyCommand *) keyCommand {
	[softKeyboard handleKeyPress:Common::KEYCODE_RIGHT];
}

- (void) mainMenuKey {
	[softKeyboard handleMainMenuKey];
}

- (void) escapeKey {
	[softKeyboard handleKeyPress:Common::KEYCODE_ESCAPE];
}

- (void) fn1Key {
	[softKeyboard handleKeyPress:Common::KEYCODE_F1];
}

- (void) fn2Key {
	[softKeyboard handleKeyPress:Common::KEYCODE_F2];
}

- (void) fn3Key {
	[softKeyboard handleKeyPress:Common::KEYCODE_F3];
}

- (void) fn4Key {
	[softKeyboard handleKeyPress:Common::KEYCODE_F4];
}

- (void) fn5Key {
	[softKeyboard handleKeyPress:Common::KEYCODE_F5];
}

- (void) fn6Key {
	[softKeyboard handleKeyPress:Common::KEYCODE_F6];
}

- (void) fn7Key {
	[softKeyboard handleKeyPress:Common::KEYCODE_F7];
}

- (void) fn8Key {
	[softKeyboard handleKeyPress:Common::KEYCODE_F8];
}

- (void) fn9Key {
	[softKeyboard handleKeyPress:Common::KEYCODE_F9];
}

- (void) fn10Key {
	[softKeyboard handleKeyPress:Common::KEYCODE_F10];
}

- (void) fn11Key {
	[softKeyboard handleKeyPress:Common::KEYCODE_F11];
}

- (void) fn12Key {
	[softKeyboard handleKeyPress:Common::KEYCODE_F12];
}

- (void) leftArrowKey {
	[softKeyboard handleKeyPress:Common::KEYCODE_LEFT];
}

- (void) upArrowKey {
	[softKeyboard handleKeyPress:Common::KEYCODE_UP];
}

- (void) rightArrowKey {
	[softKeyboard handleKeyPress:Common::KEYCODE_RIGHT];
}

- (void) downArrowKey {
	[softKeyboard handleKeyPress:Common::KEYCODE_DOWN];
}

- (void) returnKey {
	[softKeyboard handleKeyPress:Common::KEYCODE_RETURN];
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

- (void)handleMainMenuKey {
	[inputDelegate handleMainMenuKey];
}

- (void)showKeyboard {
	[inputView updateToolbarSize];
	[inputView becomeFirstResponder];
}

- (void)hideKeyboard {
	[inputView endEditing:YES];
}

@end
