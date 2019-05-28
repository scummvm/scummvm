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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/platform/sdl/macosx/appmenu_osx.h"
#include "common/translation.h"

#include "backends/platform/sdl/macosx/macosx-compat.h"
#include <Cocoa/Cocoa.h>
#include <AppKit/NSWorkspace.h>

#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_12
#define NSEventModifierFlagCommand NSCommandKeyMask
#define NSEventModifierFlagOption  NSAlternateKeyMask
#endif

#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_10
#define NSEventModifierFlags NSUInteger
#endif

#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_5
typedef unsigned long NSUInteger;
#endif

// Apple added setAppleMenu in 10.5 and removed it in 10.6.
// But as the method still exists we declare it ourselves here.
// Yes, this works :)
@interface NSApplication(MissingFunction)
- (void)setAppleMenu:(NSMenu *)menu;
@end
// However maybe we should conditionally use it depending on the system on which we run ScummVM (and not
// the one on which we compile) to only do it on OS X 10.5.
// Here is the relevant bit from the release notes for 10.6:
// In Leopard and earlier, apps that tried to construct a menu bar without a nib would get an undesirable
// stubby application menu that could not be removed. To work around this problem on Leopard, you can call
// the undocumented setAppleMenu: method and pass it the application menu, like so:
//   [NSApp setAppleMenu:[[[NSApp mainMenu] itemAtIndex:0] submenu]];
// In SnowLeopard, this workaround is unnecessary and should not be used. Under SnowLeopard, the first menu
// is always identified as the application menu.

static void openFromBundle(NSString *file) {
	NSString *path = [[NSBundle mainBundle] pathForResource:file ofType:@"rtf"];
	if (!path) {
		path = [[NSBundle mainBundle] pathForResource:file ofType:@"html"];
		if (!path) {
			path = [[NSBundle mainBundle] pathForResource:file ofType:@""];
			if (!path)
				path = [[NSBundle mainBundle] pathForResource:file ofType:@"md"];
		}
	}

	if (path) {
		// RTF and HTML files are widely recognized and we can rely on the default
		// file association working for those. For the other ones this might not be
		// the case so we explicitely indicate they should be open with TextEdit.
		if ([path hasSuffix:@".html"] || [path hasSuffix:@".rtf"])
			[[NSWorkspace sharedWorkspace] openFile:path];
		else
			[[NSWorkspace sharedWorkspace] openFile:path withApplication:@"TextEdit"];
	}
}

@interface ScummVMMenuHandler : NSObject {
}
- (void) openReadme;
- (void) openLicenseGPL;
- (void) openLicenseLGPL;
- (void) openLicenseFreefont;
- (void) openLicenseOFL;
- (void) openLicenseBSD;
- (void) openNews;
- (void) openUserManual;
- (void) openCredits;
@end

@implementation ScummVMMenuHandler : NSObject
- (void)openReadme {
	openFromBundle(@"README");
}

- (void)openLicenseGPL {
	openFromBundle(@"COPYING");
}

- (void)openLicenseLGPL {
	openFromBundle(@"COPYING-LGPL");
}

- (void)openLicenseFreefont {
	openFromBundle(@"COPYING-FREEFONT");
}

- (void)openLicenseOFL {
	openFromBundle(@"COPYING-OFL");
}

- (void)openLicenseBSD {
	openFromBundle(@"COPYING-BSD");
}

- (void)openNews {
	openFromBundle(@"NEWS");
}

- (void)openUserManual {
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"https://www.scummvm.org/documentation"]];
}

- (void)openCredits {
	openFromBundle(@"AUTHORS");
}
@end

NSString *constructNSStringFromCString(const char *rawCString, CFStringEncoding stringEncoding) {
	return (NSString *)CFStringCreateWithCString(NULL, rawCString, stringEncoding);
}

static NSMenu *addMenu(const char *title, CFStringEncoding encoding, NSString *key, SEL setAs) {
	NSString *str = constructNSStringFromCString(title, encoding);
	NSMenu *menu = [[NSMenu alloc] initWithTitle:str];

	NSMenuItem *menuItem = [[NSMenuItem alloc] initWithTitle:str action:nil keyEquivalent:key];
	[menuItem setSubmenu:menu];
	[[NSApp mainMenu] addItem:menuItem];

	if (setAs) {
		[NSApp performSelector:setAs withObject:menu];
	}

	[str release];
	[menuItem release];

	return menu;
}

static void addMenuItem(const char *title, CFStringEncoding encoding, id target, SEL selector, NSString *key, NSMenu *parent, NSEventModifierFlags flags = 0) {
	NSString *nsString = constructNSStringFromCString(title, encoding);
	NSMenuItem *menuItem = [[NSMenuItem alloc]
							initWithTitle:nsString
							action:selector
							keyEquivalent:key];
	if (target) {
		[menuItem setTarget:target];
	}
	if (flags) {
		[menuItem setKeyEquivalentModifierMask:flags];
	}
	[parent addItem:menuItem];
	[nsString release];
}

static ScummVMMenuHandler *delegate = nullptr;

void releaseMenu() {
	[delegate release];
	delegate = nullptr;
}

void replaceApplicationMenuItems() {
	if (!delegate) {
		delegate = [[ScummVMMenuHandler alloc] init];
	}

	// We cannot use [[NSApp mainMenu] removeAllItems] as removeAllItems was added in OS X 10.6
	// So remove the SDL generated menus one by one instead.
	while ([[NSApp mainMenu] numberOfItems] > 0) {
		[[NSApp mainMenu] removeItemAtIndex:0];
	}

	NSString *nsString = NULL;

	// Get current encoding
#ifdef USE_TRANSLATION
	nsString = constructNSStringFromCString(TransMan.getCurrentCharset().c_str(), NSASCIIStringEncoding);
	CFStringEncoding stringEncoding = CFStringConvertIANACharSetNameToEncoding((CFStringRef)nsString);
	[nsString release];
#else
	CFStringEncoding stringEncoding = kCFStringEncodingASCII;
#endif

	NSMenu *appleMenu = addMenu("ScummVM", kCFStringEncodingASCII, @"", @selector(setAppleMenu:));
	addMenuItem(_("About ScummVM"), stringEncoding, nil, @selector(orderFrontStandardAboutPanel:), @"", appleMenu);
	[appleMenu addItem:[NSMenuItem separatorItem]];
	addMenuItem(_("Hide ScummVM"), stringEncoding, nil, @selector(hide:), @"h", appleMenu);
	addMenuItem(_("Hide Others"), stringEncoding, nil, @selector(hideOtherApplications:), @"h", appleMenu, (NSEventModifierFlagOption|NSEventModifierFlagCommand));
	addMenuItem(_("Show All"), stringEncoding, nil, @selector(unhideAllApplications:), @"", appleMenu);
	[appleMenu addItem:[NSMenuItem separatorItem]];
	addMenuItem(_("Quit ScummVM"), stringEncoding, nil, @selector(terminate:), @"q", appleMenu);

	NSMenu *windowMenu = addMenu(_("Window"), stringEncoding, @"", @selector(setWindowsMenu:));
	addMenuItem(_("Minimize"), stringEncoding, nil, @selector(performMiniaturize:), @"m", windowMenu);

	NSMenu *helpMenu = addMenu(_("Help"), stringEncoding, @"", @selector(setHelpMenu:));
	addMenuItem(_("User Manual"), stringEncoding, delegate, @selector(openUserManual), @"", helpMenu);
	[helpMenu addItem:[NSMenuItem separatorItem]];
	addMenuItem(_("General Information"), stringEncoding, delegate, @selector(openReadme), @"", helpMenu);
	addMenuItem(_("What's New in ScummVM"), stringEncoding, delegate, @selector(openNews), @"", helpMenu);
	[helpMenu addItem:[NSMenuItem separatorItem]];
	addMenuItem(_("Credits"), stringEncoding, delegate, @selector(openCredits), @"", helpMenu);
	addMenuItem(_("GPL License"), stringEncoding, delegate, @selector(openLicenseGPL), @"", helpMenu);
	addMenuItem(_("LGPL License"), stringEncoding, delegate, @selector(openLicenseLGPL), @"", helpMenu);
	addMenuItem(_("Freefont License"), stringEncoding, delegate, @selector(openLicenseFreefont), @"", helpMenu);
	addMenuItem(_("OFL License"), stringEncoding, delegate, @selector(openLicenseOFL), @"", helpMenu);
	addMenuItem(_("BSD License"), stringEncoding, delegate, @selector(openLicenseBSD), @"", helpMenu);


	[appleMenu release];
	[windowMenu release];
	[helpMenu release];
}
