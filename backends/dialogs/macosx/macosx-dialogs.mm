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

// Disable symbol overrides so that we can use system headers
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/scummsys.h"

#if defined(MACOSX) && defined(USE_SYSDIALOGS)

#include "backends/dialogs/macosx/macosx-dialogs.h"

#include "common/config-manager.h"
#include "common/algorithm.h"
#include "common/translation.h"

#include <AppKit/NSNibDeclarations.h>
#include <AppKit/NSOpenPanel.h>
#include <AppKit/NSApplication.h>
#include <AppKit/NSButton.h>
#include <Foundation/NSString.h>
#include <Foundation/NSURL.h>
#include <Foundation/NSAutoreleasePool.h>


@interface BrowserDialogPresenter : NSObject {
@public
	NSURL *_url;
@private
	NSOpenPanel *_panel;
}
- (id) init;
- (void) dealloc;
- (void) showOpenPanel: (NSOpenPanel*) panel;
- (IBAction) showHiddenFiles : (id) sender;
@end

@implementation BrowserDialogPresenter

- (id) init {
	self = [super init];
	_url = 0;
	_panel = 0;
	return self;
}

- (void) dealloc {
	[_url release];
	[super dealloc];
}

- (void) showOpenPanel: (NSOpenPanel*) panel {
	_panel = panel;

	NSButton *showHiddenFilesButton = 0;
	if ([panel respondsToSelector:@selector(setShowsHiddenFiles:)]) {
		showHiddenFilesButton = [[NSButton alloc] init];
		[showHiddenFilesButton setButtonType:NSSwitchButton];

		CFStringRef hiddenFilesString = CFStringCreateWithCString(0, _("Show hidden files").encode().c_str(), kCFStringEncodingUTF8);
		[showHiddenFilesButton setTitle:(NSString*)hiddenFilesString];
		CFRelease(hiddenFilesString);

		[showHiddenFilesButton sizeToFit];
		if (ConfMan.getBool("gui_browser_show_hidden", Common::ConfigManager::kApplicationDomain)) {
			[showHiddenFilesButton setState:NSOnState];
			[panel setShowsHiddenFiles: YES];
		} else {
			[showHiddenFilesButton setState:NSOffState];
			[panel setShowsHiddenFiles: NO];
		}
		[panel setAccessoryView:showHiddenFilesButton];

		[showHiddenFilesButton setTarget:self];
		[showHiddenFilesButton setAction:@selector(showHiddenFiles:)];
	}

#if MAC_OS_X_VERSION_MAX_ALLOWED <= 1090
	if ([panel runModal] == NSOKButton) {
#else
	if ([panel runModal] == NSModalResponseOK) {
#endif
		NSURL *url = [panel URL];
		if ([url isFileURL]) {
			_url = url;
			[_url retain];
		}
	}

	[showHiddenFilesButton release];
	_panel = 0;
}

- (IBAction) showHiddenFiles : (id) sender {
	if ([sender state] == NSOnState) {
		[_panel setShowsHiddenFiles: YES];
		ConfMan.setBool("gui_browser_show_hidden", true, Common::ConfigManager::kApplicationDomain);
	} else {
		[_panel setShowsHiddenFiles: NO];
		ConfMan.setBool("gui_browser_show_hidden", false, Common::ConfigManager::kApplicationDomain);
	}
}

@end

Common::DialogManager::DialogResult MacOSXDialogManager::showFileBrowser(const Common::U32String &title, Common::FSNode &choice, bool isDirBrowser) {
	DialogResult result = kDialogCancel;

	// Get current encoding
	CFStringEncoding stringEncoding = kCFStringEncodingUTF8;

	// Convert labels to NSString
	CFStringRef titleRef = CFStringCreateWithCString(0, title.encode().c_str(), stringEncoding);
	CFStringRef chooseRef = CFStringCreateWithCString(0, _("Choose").encode().c_str(), stringEncoding);

	beginDialog();

	// Temporarily show the real mouse
	CGDisplayShowCursor(kCGDirectMainDisplay);

	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSWindow *keyWindow = [[NSApplication sharedApplication] keyWindow];


	NSOpenPanel *panel = [NSOpenPanel openPanel];
	[panel setCanChooseFiles:!isDirBrowser];
	[panel setCanChooseDirectories:isDirBrowser];
	if (isDirBrowser)
		[panel setTreatsFilePackagesAsDirectories:true];
	[panel setTitle:(NSString *)titleRef];
	[panel setPrompt:(NSString *)chooseRef];

	BrowserDialogPresenter* presenter = [[BrowserDialogPresenter alloc] init];
	[presenter performSelectorOnMainThread:@selector(showOpenPanel:) withObject:panel waitUntilDone:YES];
	if (presenter->_url) {
		Common::String filename = [[presenter->_url path] UTF8String];
		choice = Common::FSNode(filename);
		result = kDialogOk;
	}
	[presenter release];

	[pool release];
	[keyWindow makeKeyAndOrderFront:nil];

	CFRelease(titleRef);
	CFRelease(chooseRef);

	endDialog();

	return result;
}

#endif
