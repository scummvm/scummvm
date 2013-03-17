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
#include "common/algorithm.h"
#include "common/translation.h"

#include <AppKit/NSOpenPanel.h>
#include <AppKit/NSButton.h>
#include <Foundation/NSString.h>
#include <Foundation/NSURL.h>

@interface ShowHiddenFilesControler : NSObject {
	NSOpenPanel* _panel;
}

- (id) init;
- (void) dealloc;
- (void) setOpenPanel : (NSOpenPanel*) panel;
- (IBAction) showHiddenFiles : (id) sender;

@end

@implementation ShowHiddenFilesControler

- (id) init {
	self = [super init];
	_panel = 0;
	
	return self;
}

- (void) dealloc {
	[_panel release];
	[super dealloc];
}

- (void) setOpenPanel : (NSOpenPanel*) panel {
	_panel = panel;
	[_panel retain];
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
	_hiddenFilesRef = CFStringCreateWithCString(0, _("Show hidden files"), stringEncoding);
}

BrowserDialog::~BrowserDialog() {
	CFRelease(_titleRef);
	CFRelease(_chooseRef);
	CFRelease(_hiddenFilesRef);
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

	NSOpenPanel *panel = [NSOpenPanel openPanel];
	[panel setCanChooseFiles:!_isDirBrowser];
	[panel setCanChooseDirectories:_isDirBrowser];
	[panel setTitle:(NSString *)_titleRef];
	[panel setPrompt:(NSString *)_chooseRef];

	NSButton *shoHiddenFilesButton = 0;
	ShowHiddenFilesControler *shoHiddenFilesControler = 0;
	if ([panel respondsToSelector:@selector(setShowsHiddenFiles:)]) {
		shoHiddenFilesButton = [[NSButton alloc] init];
		[shoHiddenFilesButton retain];
		[shoHiddenFilesButton setButtonType:NSSwitchButton];
		[shoHiddenFilesButton setTitle:(NSString *)_hiddenFilesRef];
		[shoHiddenFilesButton sizeToFit];
		if (ConfMan.getBool("gui_browser_show_hidden", Common::ConfigManager::kApplicationDomain)) {
			[shoHiddenFilesButton setState:NSOnState];
			[panel setShowsHiddenFiles: YES];
		} else {
			[shoHiddenFilesButton setState:NSOffState];
			[panel setShowsHiddenFiles: NO];
		}
		[panel setAccessoryView:shoHiddenFilesButton];

		shoHiddenFilesControler = [[ShowHiddenFilesControler alloc] init];
		[shoHiddenFilesControler retain];
		[shoHiddenFilesControler setOpenPanel:panel];
		[shoHiddenFilesButton setTarget:shoHiddenFilesControler];
		[shoHiddenFilesButton setAction:@selector(showHiddenFiles:)];
	}

	if ([panel runModal] == NSOKButton) {
		NSURL *url = [panel URL];
		if ([url isFileURL]) {
			const char *filename = [[url path] UTF8String];
			_choice = Common::FSNode(filename);
			choiceMade = true;
		}
	}

	if (shoHiddenFilesButton != 0)
		[shoHiddenFilesButton release];
	if (shoHiddenFilesControler != 0)
		[shoHiddenFilesControler release];

	// If we were in fullscreen mode, switch back
	if (wasFullscreen) {
		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureFullscreenMode, true);
		g_system->endGFXTransaction();
	}

	return choiceMade;
}

} // End of namespace GUI
