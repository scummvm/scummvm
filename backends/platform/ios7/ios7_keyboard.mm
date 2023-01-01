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

@interface UITextInputTraits
- (void)setAutocorrectionType:(int)type;
- (void)setAutocapitalizationType:(int)type;
- (void)setEnablesReturnKeyAutomatically:(BOOL)val;
@end

@interface TextInputHandler : UITextField<UITabBarDelegate> {
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
	[self setAutocapitalizationType:UITextAutocapitalizationTypeNone];
	[self setEnablesReturnKeyAutomatically:NO];
#if TARGET_OS_IOS
	// Hide the input assistent bar. The API is only available since IOS 9.0.
	// The code only compils with the iOS 9.0+ SDK, and only works on iOS 9.0
	// or above.
#ifdef __IPHONE_9_0
#if __has_builtin(__builtin_available)
	if ( @available(iOS 9,*) ) {
#else
	if ( [self respondsToSelector:@selector(inputAssistantItem)] ) {
#endif
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
	toolbar.tintColor = keyboard.tintColor;
	toolbar.translucent = NO;
	toolbar.delegate = self;

	toolbar.items = @[
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
	self.inputAccessoryView = scrollView;

#endif
	return self;
}

-(void)dealloc {
	[toolbar release];
	[scrollView release];
	[super dealloc];
}

-(void)selectUITabBarItem:(UITapGestureRecognizer *)recognizer {
	switch ([[toolbar selectedItem] tag]) {
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
	self.inputAccessoryView.hidden = NO;
	// Alternatively we could add/remove instead of show/hide the inpute accessory view
//	self.inputAccessoryView = scrollView;
//	[self reloadInputViews];
	// We need at least a width of 1024 pt for the toolbar. If we add more buttons this may need to be increased.
	toolbar.frame = CGRectMake(0, 0, MAX(1024, [[UIScreen mainScreen] bounds].size.width), toolbar.frame.size.height);
	toolbar.bounds = toolbar.frame;
	toolbar.selectedItem = nil;
#if TARGET_OS_IOS
	scrollView.contentSize = toolbar.frame.size;
#endif
}

- (void)detachAccessoryView {
	self.inputAccessoryView.hidden = YES;
	// Alternatively we could add/remove instead of show/hide the inpute accessory view
//	self.inputAccessoryView = nil;
//	[self reloadInputViews];
}

- (void) setWantsPriority: (UIKeyCommand*) keyCommand {
	// In iOS 15 the UIKeyCommand has a new property wantsPriorityOverSystemBehavior that is needed to
	// receive some keys (such as the arrow keys).
	if ([keyCommand respondsToSelector:@selector(setWantsPriorityOverSystemBehavior:)]) {
		[keyCommand setValue:[NSNumber numberWithBool:YES] forKey:@"wantsPriorityOverSystemBehavior"];
	}
}

- (NSArray *)keyCommands {
	UIKeyCommand *upArrow = [UIKeyCommand keyCommandWithInput: UIKeyInputUpArrow modifierFlags: 0 action: @selector(upArrow:)];
	[self setWantsPriority: upArrow];
	UIKeyCommand *downArrow = [UIKeyCommand keyCommandWithInput: UIKeyInputDownArrow modifierFlags: 0 action: @selector(downArrow:)];
	[self setWantsPriority: downArrow];
	UIKeyCommand *leftArrow = [UIKeyCommand keyCommandWithInput: UIKeyInputLeftArrow modifierFlags: 0 action: @selector(leftArrow:)];
	[self setWantsPriority: leftArrow];
	UIKeyCommand *rightArrow = [UIKeyCommand keyCommandWithInput: UIKeyInputRightArrow modifierFlags: 0 action: @selector(rightArrow:)];
	[self setWantsPriority: rightArrow];
	UIKeyCommand *escapeKey = [UIKeyCommand keyCommandWithInput: UIKeyInputEscape modifierFlags: 0 action: @selector(escapeKey:)];
	return [[NSArray alloc] initWithObjects: upArrow, downArrow, leftArrow, rightArrow, escapeKey, nil];
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

- (void) escapeKey: (UIKeyCommand *) keyCommand {
	[softKeyboard handleKeyPress:Common::KEYCODE_ESCAPE];
}

- (void) mainMenuKey {
	[softKeyboard handleMainMenuKey];
}

- (void) escapeKey {
	[softKeyboard handleKeyPress:Common::KEYCODE_ESCAPE];
}

- (void) tabKey {
	[softKeyboard handleKeyPress:Common::KEYCODE_TAB];
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
	inputView.clearsOnBeginEditing = YES;
	[inputView layoutIfNeeded];

#if TARGET_OS_IOS
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(prepareKeyboard:) name:UIKeyboardWillShowNotification object:nil];
#endif
	return self;
}

- (void)dealloc {
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[super dealloc];
}

- (BOOL)textField:(UITextField *)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)text {
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

- (void)textFieldDidBeginEditing:(UITextField *)textField {
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

- (void)handleKeyPress:(unichar)c {
	[inputDelegate handleKeyPress:c];
}

- (void)handleMainMenuKey {
	[inputDelegate handleMainMenuKey];
}

- (void)prepareKeyboard:(NSNotification *)notification {
#if TARGET_OS_IOS
	// Check if a hardware keyboard is connected, and only show the accessory view if there isn't one.
	// If there is a hardware keyboard, the software one will only contains the text assistance bar
	// and will be small (less than 100 pt, but use a bit more in case it changes with future iOS versions).
	// This only works with iOS 9 and above. For ealier version the keyboard size is fixed and instead it
	// only shows part of the keyboard (which could be detected with the origin position, but that would
	// depend on the current orientation and screen resolution).
	NSDictionary* info = [notification userInfo];
	CGRect keyboardEndFrame = [[info objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
	if (keyboardEndFrame.size.height < 140)
		[inputView detachAccessoryView];
	else
		[inputView attachAccessoryView];
#endif
}

- (void)showKeyboard {
	[inputView becomeFirstResponder];
}

- (void)hideKeyboard {
	[inputView endEditing:YES];
}

@end
