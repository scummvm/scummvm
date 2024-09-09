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

#include "backends/platform/ios7/ios7_keyboard.h"
#include "common/keyboard.h"
#include "common/config-manager.h"
#ifdef __IPHONE_14_0
#include <GameController/GameController.h>
#endif

@interface UITextInputTraits
- (void)setAutocorrectionType:(int)type;
- (void)setAutocapitalizationType:(int)type;
- (void)setEnablesReturnKeyAutomatically:(BOOL)val;
@end

@interface TextInputHandler : UITextField<UITabBarDelegate, UITextInput> {
	SoftKeyboard *softKeyboard;
	UITabBar *toolbar;
	UIScrollView *scrollView;
}

- (id)initWithKeyboard:(SoftKeyboard *)keyboard;
- (void)dealloc;
- (void)attachAccessoryView;
- (void)detachAccessoryView;

@end


@implementation TextInputHandler

- (id)initWithKeyboard:(SoftKeyboard *)keyboard {
	self = [super initWithFrame:CGRectMake(0.0f, 0.0f, 0.0f, 0.0f)];
	softKeyboard = keyboard;

	[self setAutocorrectionType:UITextAutocorrectionTypeNo];
	[self setSpellCheckingType:UITextSpellCheckingTypeNo];
	[self setAutocapitalizationType:UITextAutocapitalizationTypeNone];
	[self setEnablesReturnKeyAutomatically:NO];
#if TARGET_OS_IOS
	// Hide the input assistent bar. The API is only available since IOS 9.0.
	// The code only compils with the iOS 9.0+ SDK, and only works on iOS 9.0
	// or above.
#ifdef __IPHONE_9_0
	if ( @available(iOS 9,*) ) {
		UITextInputAssistantItem* item = [self inputAssistantItem];
		if (item) {
			item.leadingBarButtonGroups = @[];
			item.trailingBarButtonGroups = @[];
		}
	}
#endif
#endif

	toolbar = [[UITabBar alloc] initWithFrame:CGRectMake(0.0f, 0.0f, 0.0f, 0.0f)];
	toolbar.barTintColor = keyboard.backgroundColor;
	toolbar.tintColor = [UIColor grayColor];
	toolbar.translucent = NO;
	toolbar.delegate = self;

	toolbar.items = @[
		// Keyboard layout button
		[[[UITabBarItem alloc] initWithTitle:@"123" image:nil tag:0] autorelease],
		// GMM button
		[[[UITabBarItem alloc] initWithTitle:@"\u2630" image:nil tag:1] autorelease],
		// Escape key
		[[[UITabBarItem alloc] initWithTitle:@"Esc" image:nil tag:2] autorelease],
		// Tab key
		[[[UITabBarItem alloc] initWithTitle:@"Tab" image:nil tag:3] autorelease],
		// Return key
		[[[UITabBarItem alloc] initWithTitle:@"\u23ce" image:nil tag:4] autorelease],
		// Function keys
		[[[UITabBarItem alloc] initWithTitle:@"F1" image:nil tag:5] autorelease],
		[[[UITabBarItem alloc] initWithTitle:@"F2" image:nil tag:6] autorelease],
		[[[UITabBarItem alloc] initWithTitle:@"F3" image:nil tag:7] autorelease],
		[[[UITabBarItem alloc] initWithTitle:@"F4" image:nil tag:8] autorelease],
		[[[UITabBarItem alloc] initWithTitle:@"F5" image:nil tag:9] autorelease],
		[[[UITabBarItem alloc] initWithTitle:@"F6" image:nil tag:10] autorelease],
		[[[UITabBarItem alloc] initWithTitle:@"F7" image:nil tag:11] autorelease],
		[[[UITabBarItem alloc] initWithTitle:@"F8" image:nil tag:12] autorelease],
		[[[UITabBarItem alloc] initWithTitle:@"F9" image:nil tag:13] autorelease],
		[[[UITabBarItem alloc] initWithTitle:@"F10" image:nil tag:14] autorelease],
		[[[UITabBarItem alloc] initWithTitle:@"F11" image:nil tag:15] autorelease],
		[[[UITabBarItem alloc] initWithTitle:@"F12" image:nil tag:16] autorelease],
		// Arrow keys
		[[[UITabBarItem alloc] initWithTitle:@"\u2190" image:nil tag:17] autorelease],
		[[[UITabBarItem alloc] initWithTitle:@"\u2191" image:nil tag:18] autorelease],
		[[[UITabBarItem alloc] initWithTitle:@"\u2192" image:nil tag:19] autorelease],
		[[[UITabBarItem alloc] initWithTitle:@"\u2193" image:nil tag:20] autorelease]
	];

	// Increase the font size on the UITabBarItems to make them readable on small displays
	for (UITabBarItem *item in toolbar.items) {
		[item setTitleTextAttributes: [NSDictionary dictionaryWithObjectsAndKeys: [UIFont fontWithName:@"Helvetica" size:20.0], NSFontAttributeName, nil] forState:UIControlStateNormal];
	}

#if TARGET_OS_TV
	// In tvOS a UITabBarItem is selected when moving to the selected item, in other words
	// no click is required. This is not a great user experience since the user needs to
	// scroll to the wanted UITabBarItem causing the delegate function
	// tabBar:(UITabBar *)tabBar didSelectItem:(UITabBarItem *)item to be called multiple
	// times. Instead add a tap gesture on the UITabBar and let the delegate function set
	// the selected item. Then trigger the action when the user press the button.
	UITapGestureRecognizer *tapGesture = [[[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(selectUITabBarItem:)] autorelease];
	[toolbar addGestureRecognizer:tapGesture];
#endif

	self.inputAccessoryView = toolbar;
	[toolbar sizeToFit];

#if TARGET_OS_IOS
	// In tvOS the UITabBar is scrollable but not in iOS. In iOS the UITabBar must be
	// put in a UIScrollView to be scrollable.
	scrollView = [[UIScrollView alloc] init];
	scrollView.frame = toolbar.frame;
	scrollView.bounds = toolbar.bounds;
	scrollView.autoresizingMask = toolbar.autoresizingMask;
	scrollView.showsVerticalScrollIndicator = false;
	scrollView.showsHorizontalScrollIndicator = false;
	toolbar.autoresizingMask = UIViewAutoresizingNone;
	[scrollView addSubview:toolbar];
	self.inputAccessoryView = nil;

#endif
	return self;
}

-(void)dealloc {
	[toolbar release];
	[scrollView release];
	[super dealloc];
}

/* There's a difference between UITextFields and UITextViews that the
 * delegate function textView:shouldChangeTextInRange:replacementText:
 * is called when pressing the backward button on a keyboard also when
 * the textView is empty. This is not the case for UITextFields, the
 * function textField:shouldChangeTextInRange:replacementText: is not
 * called if the textField is empty which is problematic in the cases
 * where there's already text in the open dialog (e.g. the save dialog
 * when the user wants to overwrite an existing slot). There's currently
 * no possibility to propagate existing text elements from dialog into
 * the textField. To be able to handle the cases where the user wants to
 * delete existing texts when the textField is empty the inputView has
 * to implement the UITextInput protocol function deleteBackward that is
 * called every time the backward key is pressed.
 * Propagate all delete callbacks to the backend.
 */
-(void)deleteBackward {
	[softKeyboard handleKeyPress:'\b' withModifierFlags:0];
	[super deleteBackward];
}

-(void)selectUITabBarItem:(UITapGestureRecognizer *)recognizer {
	switch ([[toolbar selectedItem] tag]) {
	case 0:
		[self switchKeyboardLayout];
		break;
	case 1:
		[self mainMenuKey];
		break;
	case 2:
		[self escapeKey];
		break;
	case 3:
		[self tabKey];
		break;
	case 4:
		[self returnKey];
		break;
	case 5:
		[self fn1Key];
		break;
	case 6:
		[self fn2Key];
		break;
	case 7:
		[self fn3Key];
		break;
	case 8:
		[self fn4Key];
		break;
	case 9:
		[self fn5Key];
		break;
	case 10:
		[self fn6Key];
		break;
	case 11:
		[self fn7Key];
		break;
	case 12:
		[self fn8Key];
		break;
	case 13:
		[self fn9Key];
		break;
	case 14:
		[self fn10Key];
		break;
	case 15:
		[self fn11Key];
		break;
	case 16:
		[self fn12Key];
		break;
	case 17:
		[self leftArrowKey];
		break;
	case 18:
		[self upArrowKey];
		break;
	case 19:
		[self rightArrowKey];
		break;
	case 20:
		[self downArrowKey];
		break;
	default:
		break;
	}
}

-(void)tabBar:(UITabBar *)tabBar didSelectItem:(UITabBarItem *)item {
#if TARGET_OS_IOS
	// In iOS the UITabBarItem is selected on touch. Trigger the action
	// on the selected item.
	[self selectUITabBarItem:nil];
#endif
}

- (void)attachAccessoryView {
	// We need at least a width of 1024 pt for the toolbar. If we add more buttons this may need to be increased.
	toolbar.frame = CGRectMake(0, 0, MAX(CGFloat(1024), [[UIScreen mainScreen] bounds].size.width), toolbar.frame.size.height);
	toolbar.bounds = toolbar.frame;
	toolbar.selectedItem = nil;
	self.inputAccessoryView = toolbar;
#if TARGET_OS_IOS
	scrollView.contentSize = toolbar.frame.size;
	self.inputAccessoryView = scrollView;
#endif
	[self reloadInputViews];
}

- (void)detachAccessoryView {
	self.inputAccessoryView = nil;
	[self reloadInputViews];
}

- (void) setWantsPriority: (UIKeyCommand*) keyCommand {
	// In iOS 15 the UIKeyCommand has a new property wantsPriorityOverSystemBehavior that is needed to
	// receive some keys (such as the arrow keys).
	if ([keyCommand respondsToSelector:@selector(setWantsPriorityOverSystemBehavior:)]) {
		[keyCommand setValue:[NSNumber numberWithBool:YES] forKey:@"wantsPriorityOverSystemBehavior"];
	}
}

- (UIKeyCommand *)createKeyCommandForKey:(NSString *)key withModifierFlags:(UIKeyModifierFlags)flags andSelector:(SEL)selector {
	UIKeyCommand *k = [UIKeyCommand keyCommandWithInput:key modifierFlags:flags action:selector];
	[self setWantsPriority:k];
	return k;
}

- (NSArray *)overloadKeys:(NSArray<NSString *> *)keys withSelector:(SEL)selector {
	NSMutableArray<UIKeyCommand *> *overloadedKeys = [[[NSMutableArray alloc] init] autorelease];
	for (NSString *key in keys) {
		[overloadedKeys addObject:[self createKeyCommandForKey:key withModifierFlags:0 andSelector:selector]];
		[overloadedKeys addObject:[self createKeyCommandForKey:key withModifierFlags:UIKeyModifierShift andSelector:selector]];
		[overloadedKeys addObject:[self createKeyCommandForKey:key withModifierFlags:UIKeyModifierControl andSelector:selector]];
		[overloadedKeys addObject:[self createKeyCommandForKey:key withModifierFlags:UIKeyModifierAlternate andSelector:selector]];
		[overloadedKeys addObject:[self createKeyCommandForKey:key withModifierFlags:UIKeyModifierCommand andSelector:selector]];
		// UIKeyModifierAlphaShift seems broken since iOS 13
		[overloadedKeys addObject:[self createKeyCommandForKey:key withModifierFlags:UIKeyModifierAlphaShift andSelector:selector]];
		[overloadedKeys addObject:[self createKeyCommandForKey:key withModifierFlags:UIKeyModifierNumericPad andSelector:selector]];
	}
	return overloadedKeys;
}

- (NSArray *)overloadArrowKeys {
	NSArray<NSString *> *arrowKeys = [[[NSArray alloc] initWithObjects:UIKeyInputUpArrow, UIKeyInputDownArrow, UIKeyInputLeftArrow, UIKeyInputRightArrow, nil] autorelease];
	return [self overloadKeys:arrowKeys withSelector:@selector(handleArrowKey:)];
}

- (NSArray *)overloadRomanLetters {
	NSString *romanLetters = @"abcdefghijklmnopqrstuvwxyz";
	NSMutableArray<NSString *> *letters = [[[NSMutableArray alloc] init] autorelease];
	for (NSUInteger x = 0; x < romanLetters.length; x++) {
		unichar c = [romanLetters characterAtIndex:x];
		[letters addObject:[NSString stringWithCharacters:&c length:1]];
	}
	return [self overloadKeys:letters withSelector:@selector(handleLetterKey:)];;
}

- (NSArray *)overloadNumbers {
	NSString *numbers = @"0123456789";
	NSMutableArray<NSString *> *numArray = [[[NSMutableArray alloc] init] autorelease];
	for (NSUInteger x = 0; x < numbers.length; x++) {
		unichar c = [numbers characterAtIndex:x];
		[numArray addObject:[NSString stringWithCharacters:&c length:1]];
	}
	return [self overloadKeys:numArray withSelector:@selector(handleNumberKey:)];
}

- (NSArray *)overloadFnKeys {
#ifdef __IPHONE_13_4
	if (@available(iOS 13.4, *)) {
		NSArray<NSString *> *fnKeys = [[[NSArray alloc] initWithObjects:UIKeyInputF1, UIKeyInputF2, UIKeyInputF3, UIKeyInputF4, UIKeyInputF5, UIKeyInputF6, UIKeyInputF7, UIKeyInputF8, UIKeyInputF9, UIKeyInputF10, UIKeyInputF11, UIKeyInputF12, nil] autorelease];
		return [self overloadKeys:fnKeys withSelector:@selector(handleFnKey:)];
	}
#endif
	return nil;
}

- (NSArray *)overloadSpecialKeys {
#ifdef __IPHONE_13_4
	NSMutableArray<NSString *> *specialKeys = [[[NSMutableArray alloc] initWithObjects:UIKeyInputEscape, UIKeyInputPageUp, UIKeyInputPageDown, nil] autorelease];

	if (@available(iOS 13.4, *)) {
		[specialKeys addObject: UIKeyInputHome];
		[specialKeys addObject: UIKeyInputEnd];
	}
	return [self overloadKeys:specialKeys withSelector:@selector(handleSpecialKey:)];
#else
	return nil;
#endif
}

- (int)convertModifierFlags:(UIKeyModifierFlags)flags {
	return (((flags & UIKeyModifierShift) ? Common::KBD_SHIFT : 0) |
		((flags & UIKeyModifierControl) ? Common::KBD_CTRL : 0) |
		((flags & UIKeyModifierAlternate) ? Common::KBD_ALT : 0) |
		((flags & UIKeyModifierCommand) ? Common::KBD_META : 0) |
		((flags & UIKeyModifierAlphaShift) ? Common::KBD_CAPS : 0) |
		((flags & UIKeyModifierNumericPad) ? Common::KBD_NUM : 0));
}

- (void)handleArrowKey:(UIKeyCommand *)keyCommand {
	if (keyCommand.input == UIKeyInputUpArrow) {
		[self upArrow:keyCommand];
	} else if (keyCommand.input == UIKeyInputDownArrow) {
		[self downArrow:keyCommand];
	} else if (keyCommand.input == UIKeyInputLeftArrow) {
		[self leftArrow:keyCommand];
	} else {
		[self rightArrow:keyCommand];
	}
}

- (void)handleLetterKey:(UIKeyCommand *)keyCommand {
	UniChar c = [[keyCommand input] characterAtIndex:0];
	if ((keyCommand.modifierFlags & UIKeyModifierShift) ||
		(keyCommand.modifierFlags & UIKeyModifierAlphaShift)) {
		// Convert to capital letter
		c -= 32;
	}
	[softKeyboard handleKeyPress: c withModifierFlags:[self convertModifierFlags:keyCommand.modifierFlags]];
}

- (void)handleNumberKey:(UIKeyCommand *)keyCommand {
	if (keyCommand.modifierFlags == UIKeyModifierCommand) {
		switch ([[keyCommand input] characterAtIndex:0]) {
		case '1':
			[self fn1Key];
			break;
		case '2':
			[self fn2Key];
			break;
		case '3':
			[self fn3Key];
			break;
		case '4':
			[self fn4Key];
			break;
		case '5':
			[self fn5Key];
			break;
		case '6':
			[self fn6Key];
			break;
		case '7':
			[self fn7Key];
			break;
		case '8':
			[self fn8Key];
			break;
		case '9':
			[self fn9Key];
			break;
		case '0':
			[self fn10Key];
			break;
		default:
			break;
		}
	} else if (keyCommand.modifierFlags == (UIKeyModifierCommand | UIKeyModifierShift)) {
		switch ([[keyCommand input] characterAtIndex:0]) {
		case '1':
			[self fn11Key];
			break;
		case '2':
			[self fn12Key];
			break;
		default:
			break;
		}
	} else {
		[softKeyboard handleKeyPress: [[keyCommand input] characterAtIndex:0] withModifierFlags:[self convertModifierFlags:keyCommand.modifierFlags]];
	}
}

- (void)handleFnKey:(UIKeyCommand *)keyCommand {
#ifdef __IPHONE_13_4
	if (@available(iOS 13.4, *)) {
		if (keyCommand.input == UIKeyInputF1) {
			[self fn1Key];
		} else if (keyCommand.input == UIKeyInputF2) {
			[self fn2Key];
		} else if (keyCommand.input == UIKeyInputF3) {
			[self fn3Key];
		} else if (keyCommand.input == UIKeyInputF4) {
			[self fn4Key];
		} else if (keyCommand.input == UIKeyInputF5) {
			[self fn5Key];
		} else if (keyCommand.input == UIKeyInputF6) {
			[self fn6Key];
		} else if (keyCommand.input == UIKeyInputF7) {
			[self fn7Key];
		} else if (keyCommand.input == UIKeyInputF8) {
			[self fn8Key];
		} else if (keyCommand.input == UIKeyInputF9) {
			[self fn9Key];
		} else if (keyCommand.input == UIKeyInputF10) {
			[self fn10Key];
		} else if (keyCommand.input == UIKeyInputF11) {
			[self fn11Key];
		} else if (keyCommand.input == UIKeyInputF12) {
			[self fn12Key];
		}
	}
#endif
}

- (void)handleSpecialKey:(UIKeyCommand *)keyCommand {
#ifdef __IPHONE_13_4
	if (keyCommand.input == UIKeyInputEscape) {
		[self escapeKey:keyCommand];
	} else if (keyCommand.input == UIKeyInputPageUp) {
		[softKeyboard handleKeyPress:Common::KEYCODE_PAGEUP withModifierFlags:[self convertModifierFlags:keyCommand.modifierFlags]];
	} else if (keyCommand.input == UIKeyInputPageDown) {
		[softKeyboard handleKeyPress:Common::KEYCODE_PAGEDOWN withModifierFlags:[self convertModifierFlags:keyCommand.modifierFlags]];
	}
	if (@available(iOS 13.4, *)) {
		if (keyCommand.input == UIKeyInputHome) {
			[softKeyboard handleKeyPress:Common::KEYCODE_HOME withModifierFlags:[self convertModifierFlags:keyCommand.modifierFlags]];
		} else if (keyCommand.input == UIKeyInputEnd) {
			[softKeyboard handleKeyPress:Common::KEYCODE_END withModifierFlags:[self convertModifierFlags:keyCommand.modifierFlags]];
		}
	}
#endif
}

- (NSArray *)keyCommands {
	NSMutableArray<UIKeyCommand *> *overloadedKeys = [[[NSMutableArray alloc] init] autorelease];
	// Arrows
	[overloadedKeys addObjectsFromArray:[self overloadArrowKeys]];
	// Roman letters
	[overloadedKeys addObjectsFromArray:[self overloadRomanLetters]];
	// Numbers
	[overloadedKeys addObjectsFromArray:[self overloadNumbers]];
	// FN keys
	[overloadedKeys addObjectsFromArray:[self overloadFnKeys]];
	// ESC, PAGE_UP, PAGE_DOWN, HOME, END
	[overloadedKeys addObjectsFromArray:[self overloadSpecialKeys]];

	return overloadedKeys;
}

- (void) upArrow: (UIKeyCommand *) keyCommand {
	if (keyCommand.modifierFlags == UIKeyModifierCommand) {
		[softKeyboard handleKeyPress:Common::KEYCODE_PAGEUP withModifierFlags:0];
	} else {
		[softKeyboard handleKeyPress:Common::KEYCODE_UP withModifierFlags:[self convertModifierFlags:keyCommand.modifierFlags]];
	}
}

- (void) downArrow: (UIKeyCommand *) keyCommand {
	if (keyCommand.modifierFlags == UIKeyModifierCommand) {
		[softKeyboard handleKeyPress:Common::KEYCODE_PAGEDOWN withModifierFlags:0];
	} else {
		[softKeyboard handleKeyPress:Common::KEYCODE_DOWN withModifierFlags:[self convertModifierFlags:keyCommand.modifierFlags]];
	}
}

- (void) leftArrow: (UIKeyCommand *) keyCommand {
	if (keyCommand.modifierFlags == UIKeyModifierCommand) {
		[softKeyboard handleKeyPress:Common::KEYCODE_HOME withModifierFlags:0];
	} else {
		[softKeyboard handleKeyPress:Common::KEYCODE_LEFT withModifierFlags:[self convertModifierFlags:keyCommand.modifierFlags]];
	}
}

- (void) rightArrow: (UIKeyCommand *) keyCommand {
	if (keyCommand.modifierFlags == UIKeyModifierCommand) {
		[softKeyboard handleKeyPress:Common::KEYCODE_END withModifierFlags:0];
	} else {
		[softKeyboard handleKeyPress:Common::KEYCODE_RIGHT withModifierFlags:[self convertModifierFlags:keyCommand.modifierFlags]];
	}
}

- (void) escapeKey: (UIKeyCommand *) keyCommand {
	[softKeyboard handleKeyPress:Common::KEYCODE_ESCAPE withModifierFlags:0];
}

- (void) mainMenuKey {
	[softKeyboard handleMainMenuKey];
}

- (void) escapeKey {
	[softKeyboard handleKeyPress:Common::KEYCODE_ESCAPE withModifierFlags:0];
}

- (void) tabKey {
	[softKeyboard handleKeyPress:Common::KEYCODE_TAB withModifierFlags:0];
}

- (void) fn1Key {
	[softKeyboard handleKeyPress:Common::KEYCODE_F1 withModifierFlags:0];
}

- (void) fn2Key {
	[softKeyboard handleKeyPress:Common::KEYCODE_F2 withModifierFlags:0];
}

- (void) fn3Key {
	[softKeyboard handleKeyPress:Common::KEYCODE_F3 withModifierFlags:0];
}

- (void) fn4Key {
	[softKeyboard handleKeyPress:Common::KEYCODE_F4 withModifierFlags:0];
}

- (void) fn5Key {
	[softKeyboard handleKeyPress:Common::KEYCODE_F5 withModifierFlags:0];
}

- (void) fn6Key {
	[softKeyboard handleKeyPress:Common::KEYCODE_F6 withModifierFlags:0];
}

- (void) fn7Key {
	[softKeyboard handleKeyPress:Common::KEYCODE_F7 withModifierFlags:0];
}

- (void) fn8Key {
	[softKeyboard handleKeyPress:Common::KEYCODE_F8 withModifierFlags:0];
}

- (void) fn9Key {
	[softKeyboard handleKeyPress:Common::KEYCODE_F9 withModifierFlags:0];
}

- (void) fn10Key {
	[softKeyboard handleKeyPress:Common::KEYCODE_F10 withModifierFlags:0];
}

- (void) fn11Key {
	[softKeyboard handleKeyPress:Common::KEYCODE_F11 withModifierFlags:0];
}

- (void) fn12Key {
	[softKeyboard handleKeyPress:Common::KEYCODE_F12 withModifierFlags:0];
}

- (void) leftArrowKey {
	[softKeyboard handleKeyPress:Common::KEYCODE_LEFT withModifierFlags:0];
}

- (void) upArrowKey {
	[softKeyboard handleKeyPress:Common::KEYCODE_UP withModifierFlags:0];
}

- (void) rightArrowKey {
	[softKeyboard handleKeyPress:Common::KEYCODE_RIGHT withModifierFlags:0];
}

- (void) downArrowKey {
	[softKeyboard handleKeyPress:Common::KEYCODE_DOWN withModifierFlags:0];
}

- (void) returnKey {
	[softKeyboard handleKeyPress:Common::KEYCODE_RETURN withModifierFlags:0];
}

- (void) switchKeyboardLayout {
	if ([self keyboardType] == UIKeyboardTypeDefault) {
		[self setKeyboardType:UIKeyboardTypeNumberPad];
		[[toolbar selectedItem] setTitle:@"abc"];
	} else {
		[self setKeyboardType:UIKeyboardTypeDefault];
		[[toolbar selectedItem] setTitle:@"123"];
	}

	[self reloadInputViews];
}
@end


@implementation SoftKeyboard {
	BOOL _keyboardVisible;
	CGFloat _inputAccessoryHeight;
}

#if TARGET_OS_IOS
- (void)resizeParentFrame:(NSNotification*)notification keyboardDidShow:(BOOL)didShow
{
	NSDictionary* userInfo = [notification userInfo];
	CGRect keyboardFrame = [[userInfo objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
	keyboardFrame = [self.superview convertRect:keyboardFrame fromView:nil];

	// Base the new frame size on the current parent frame size
	CGRect newFrame = self.superview.frame;
#ifdef __IPHONE_14_0
	if (@available(iOS 14.0, tvOS 14.0, *)) {
		if (GCKeyboard.coalescedKeyboard != nil) {
			if (didShow) {
				// The inputAccessoryView is hidden by setting it to nil. Then when
				// receiving the UIKeyboardDidHideNotification the height will be 0.
				// Remember the height of the inputAccessoryView when it's presented
				// so the main frame can be resized back to the proper size.
				_inputAccessoryHeight = inputView.inputAccessoryView.frame.size.height;
			}
			newFrame.size.height += (_inputAccessoryHeight) * (didShow ? -1 : 1);
		} else {
			newFrame.size.height += (keyboardFrame.size.height) * (didShow ? -1 : 1);
		}
	} else {
		newFrame.size.height += (keyboardFrame.size.height) * (didShow ? -1 : 1);
	}
#endif
	// Resize with a fancy animation
	NSNumber *rate = notification.userInfo[UIKeyboardAnimationDurationUserInfoKey];
	[UIView animateWithDuration:rate.floatValue animations:^{
		self.superview.frame = newFrame;
	}];
}

- (void)keyboardDidShow:(NSNotification*)notification
{
	// NotificationCenter might notify multiple times
	// when keyboard did show because the accessoryView
	// affect the keyboard height. However since we use
	// UIKeyboardFrameEndUserInfoKey to get the keyboard
	// it will always have the same value. Make sure to
	// only handle one notification.
	if (!_keyboardVisible) {
		[self resizeParentFrame:notification keyboardDidShow:YES];
		_keyboardVisible = YES;
	}
}

- (void)keyboardDidHide:(NSNotification*)notification
{
	// NotificationCenter will only call this once
	// when keyboard did hide.
	[self resizeParentFrame:notification keyboardDidShow:NO];
	_keyboardVisible = NO;
}

- (void)keyboardDidConnect:(NSNotification*)notification
{
	[inputView becomeFirstResponder];
}

- (void)keyboardDidDisconnect:(NSNotification*)notification
{
#ifdef __IPHONE_14_0
	if (@available(iOS 14.0, tvOS 14.0, *)) {
		if (GCKeyboard.coalescedKeyboard == nil) {
			[inputView endEditing:YES];
		}
	}
#endif
}
#endif

- (id)initWithFrame:(CGRect)frame {
	self = [super initWithFrame:frame];

#if TARGET_OS_IOS
	[[NSNotificationCenter defaultCenter] addObserver:self
	 selector:@selector(keyboardDidShow:)
	 name:UIKeyboardDidShowNotification
	 object:nil];

	[[NSNotificationCenter defaultCenter] addObserver:self
	 selector:@selector(keyboardDidHide:)
	 name:UIKeyboardDidHideNotification
	 object:nil];
#endif
#ifdef __IPHONE_14_0
	if (@available(iOS 14.0, tvOS 14.0, *)) {
		[[NSNotificationCenter defaultCenter] addObserver:self
		 selector:@selector(keyboardDidConnect:)
		 name:GCKeyboardDidConnectNotification
	     object:nil];

		[[NSNotificationCenter defaultCenter] addObserver:self
		 selector:@selector(keyboardDidDisconnect:)
		 name:GCKeyboardDidDisconnectNotification
	     object:nil];
	}
#endif

	inputDelegate = nil;
	inputView = [[TextInputHandler alloc] initWithKeyboard:self];
	inputView.delegate = self;
	inputView.clearsOnBeginEditing = YES;
	inputView.keyboardType = UIKeyboardTypeDefault;
	[inputView layoutIfNeeded];
	_keyboardVisible = NO;
	_inputAccessoryHeight = 0.0f;

#ifdef __IPHONE_14_0
	if (@available(iOS 14.0, tvOS 14.0, *)) {
		// If already connected to a HW keyboard, start
		// monitoring key presses
		if (GCKeyboard.coalescedKeyboard != nil) {
			[inputView becomeFirstResponder];
		}
	}
#endif
	return self;
}

- (void)dealloc {
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[super dealloc];
}

- (BOOL)textField:(UITextField *)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)text {
	if (text.length) {
		[inputDelegate handleKeyPress:[text characterAtIndex:0] withModifierFlags:0];
	}
	return YES;
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField {
	[inputView returnKey];
	return NO;
}

- (void)textFieldDidBeginEditing:(UITextField *)textField {
	if (ConfMan.getBool("keyboard_fn_bar"))
		[inputView attachAccessoryView];
}
- (void)textFieldDidEndEditing:(UITextField *)textField {
	[inputView detachAccessoryView];
}

-(UITextField *)inputView {
	return inputView;
}

- (void)setInputDelegate:(id)delegate {
	inputDelegate = delegate;
}

- (void)handleKeyPress:(unichar)c withModifierFlags:(int)f {
	[inputDelegate handleKeyPress:c withModifierFlags:f];
}

- (void)handleMainMenuKey {
	[inputDelegate handleMainMenuKey];
}

- (void)showKeyboard {
#ifdef __IPHONE_14_0
	if (@available(iOS 14.0, tvOS 14.0, *)) {
		if ([inputView isFirstResponder] &&
			GCKeyboard.coalescedKeyboard != nil) {
			if (ConfMan.getBool("keyboard_fn_bar")) {
				[inputView attachAccessoryView];
			}
			return;
		}
	}
#endif
	[inputView becomeFirstResponder];
}

- (void)hideKeyboard {
#ifdef __IPHONE_14_0
	if (@available(iOS 14.0, tvOS 14.0, *)) {
		if ([inputView isFirstResponder] &&
			GCKeyboard.coalescedKeyboard != nil) {
			if (!ConfMan.getBool("keyboard_fn_bar")) {
				[inputView detachAccessoryView];
			}
			return;
		}
	}
#endif
	[inputView endEditing:YES];
}

- (BOOL)isKeyboardShown {
	return [inputView isFirstResponder];
}
@end
