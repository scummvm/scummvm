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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/platform/sdl/macosx/appmenu_osx.h"

#include "backends/platform/sdl/macosx/macosx-compat.h"

#include "common/system.h"
#include "common/events.h"

#include <Cocoa/Cocoa.h>
#include <AppKit/NSWorkspace.h>

#if defined(MAC_OS_X_VERSION_10_12_2) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12_2

@interface ScummVMlTouchbarDelegate : NSResponder <NSTouchBarDelegate>
@end

@implementation ScummVMlTouchbarDelegate
NSColor *font_color;
NSColor *back_color;
NSButton *tbButton;

- (instancetype)init {
	if (self = [super init]) {
		font_color = [NSColor whiteColor];
		back_color = [NSColor colorWithCalibratedRed:0 green:0.8 blue:0.2 alpha:1.0f];

		tbButton = [[NSButton alloc] init];
		[[tbButton cell] setBackgroundColor:back_color];
		[tbButton setAction:@selector(actionKey:)];
		[tbButton setTarget:self];

		[self setButton:nil];
	}
	return self;
}

- (NSTouchBar *)makeTouchBar {
	NSTouchBar *bar = [[NSTouchBar alloc] init];

	bar.delegate = self;
	bar.customizationIdentifier = @"org.ScummVM.Touchbar.Customization";
	bar.defaultItemIdentifiers = @[@"textButton"];

	return bar;
}

- (nullable NSTouchBarItem *)touchBar:(NSTouchBar *)touchBar makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier {
	NSCustomTouchBarItem *customItem = [[NSCustomTouchBarItem alloc] initWithIdentifier:@"textButton"];
	customItem.view = tbButton;
	customItem.visibilityPriority = NSTouchBarItemPriorityHigh;

	return customItem;
}

- (void)setButton : (const char *)title {
	NSString *ns_title = nil;
	if (title) {
		ns_title = [NSString stringWithUTF8String:title];
	} else {
		ns_title = [NSString stringWithUTF8String:"ScummVM"];
	}

	NSMutableAttributedString *att_title = [[NSMutableAttributedString alloc] initWithString:ns_title];

	[tbButton setAttributedTitle:att_title];
	[tbButton invalidateIntrinsicContentSize];
}

- (IBAction) actionKey : (id) sender {
	Common::Event event;

	event.type = Common::EVENT_MAINMENU;
	g_system->getEventManager()->pushEvent(event);
}

@end

static ScummVMlTouchbarDelegate *g_tb_delegate = nil;

void macOSTouchbarUpdate(const char *message) {
	[g_tb_delegate setButton:message];
}

void macOSTouchbarCreate() {
	if (g_tb_delegate)
		return;

	if (NSAppKitVersionNumber < NSAppKitVersionNumber10_12_2)
		return;

	NSApplication *app = [NSApplication sharedApplication];
	if (!app)
		return;

	g_tb_delegate = [[ScummVMlTouchbarDelegate alloc] init];

	if (g_tb_delegate) {
		g_tb_delegate.nextResponder = app.nextResponder;
		app.nextResponder = g_tb_delegate;
	}
}

void macOSTouchbarDestroy() {
}

#else

void macOSTouchbarCreate() {}
void macOSTouchbarDestroy() {}
void macOSTouchbarUpdate(const char *message) {}

#endif
