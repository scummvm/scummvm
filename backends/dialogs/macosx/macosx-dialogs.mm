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

#include "gui/browser.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "common/events.h"
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

#ifdef USE_TRANSLATION
		CFStringRef encStr = CFStringCreateWithCString(NULL, TransMan.getCurrentCharset().c_str(), kCFStringEncodingASCII);
		CFStringEncoding stringEncoding = CFStringConvertIANACharSetNameToEncoding(encStr);
		CFRelease(encStr);
#else
		CFStringEncoding stringEncoding = kCFStringEncodingASCII;
#endif
		CFStringRef hiddenFilesString = CFStringCreateWithCString(0, _("Show hidden files"), stringEncoding);
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

namespace GUI {

BrowserDialog::BrowserDialog(const char *title, bool dirBrowser)
	: Dialog("Browser") {

	// remember whether this is a file browser or a directory browser.
	_isDirBrowser = dirBrowser;

	// Get current encoding
#ifdef USE_TRANSLATION
	CFStringRef encStr = CFStringCreateWithCString(NULL, TransMan.getCurrentCharset().c_str(), kCFStringEncodingASCII);
	CFStringEncoding stringEncoding = CFStringConvertIANACharSetNameToEncoding(encStr);
	CFRelease(encStr);
#else
	CFStringEncoding stringEncoding = kCFStringEncodingASCII;
#endif

	// Convert title to NSString
	_titleRef = CFStringCreateWithCString(0, title, stringEncoding);

	// Convert button text to NSString
	_chooseRef = CFStringCreateWithCString(0, _("Choose"), stringEncoding);
}

BrowserDialog::~BrowserDialog() {
	CFRelease(_titleRef);
	CFRelease(_chooseRef);
}

int BrowserDialog::runModal() {
	bool choiceMade = false;

	// If in fullscreen mode, switch to windowed mode
	bool wasFullscreen = g_system->getFeatureState(OSystem::kFeatureFullscreenMode);
	if (wasFullscreen) {
		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureFullscreenMode, false);
		g_system->endGFXTransaction();
	}

	// Temporarily show the real mouse
	CGDisplayShowCursor(kCGDirectMainDisplay);

	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSWindow *keyWindow = [[NSApplication sharedApplication] keyWindow];


	NSOpenPanel *panel = [NSOpenPanel openPanel];
	[panel setCanChooseFiles:!_isDirBrowser];
	[panel setCanChooseDirectories:_isDirBrowser];
	if (_isDirBrowser)
		[panel setTreatsFilePackagesAsDirectories:true];
	[panel setTitle:(NSString *)_titleRef];
	[panel setPrompt:(NSString *)_chooseRef];

	BrowserDialogPresenter* presenter = [[BrowserDialogPresenter alloc] init];
	[presenter performSelectorOnMainThread:@selector(showOpenPanel:) withObject:panel waitUntilDone:YES];
	if (presenter->_url) {
		Common::String filename = [[presenter->_url path] UTF8String];
		_choice = Common::FSNode(filename);
		choiceMade = true;
	}
	[presenter release];

	[pool release];
	[keyWindow makeKeyAndOrderFront:nil];

	// While the native macOS file browser is open, any input events (e.g. keypresses) are
	// still received by the NSApplication. With SDL backend for example this results in the
	// events beeing queued and processed after we return, thus dispatching events that were
	// intended for the native file browser. For example: pressing Esc to cancel the native
	// macOS file browser would cause the application to quit in addition to closing the
	// file browser. To avoid this happening clear all pending events.
	g_system->getEventManager()->getEventDispatcher()->clearEvents();

	// If we were in fullscreen mode, switch back
	if (wasFullscreen) {
		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureFullscreenMode, true);
		g_system->endGFXTransaction();
	}

	return choiceMade;
}

} // End of namespace GUI
