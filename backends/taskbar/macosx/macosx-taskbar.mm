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

#if defined(MACOSX) && defined(USE_TASKBAR)

#include "backends/taskbar/macosx/macosx-taskbar.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "backends/platform/sdl/macosx/macosx-compat.h"
#include <AppKit/NSApplication.h>
#include <AppKit/NSImage.h>
#include <Foundation/NSString.h>
#include <Foundation/NSDictionary.h>
#include <Foundation/NSArray.h>
#include <Foundation/NSUserDefaults.h>
#include <AppKit/NSImageView.h>
#include <AppKit/NSColor.h>
#include <AppKit/NSBezierPath.h>
#include <CoreFoundation/CFString.h>

// NSDockTile was introduced with Mac OS X 10.5.
// Try provide backward compatibility by avoiding NSDockTile symbols.
#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_5
typedef id NSDockTilePtr;
#else
#include <AppKit/NSDockTile.h>
typedef NSDockTile * NSDockTilePtr;
#endif

NSDockTilePtr _dockTile;
NSImageView *_applicationIconView;
NSImageView *_overlayIconView;

// Using a NSProgressIndicator as a sub-view of the NSDockTile view does not work properly.
// The progress indicator is grayed out and display no progress. So instead the bar is drawn
// manually, which is a bit more work :(

MacOSXTaskbarManager::MacOSXTaskbarManager() : _progress(-1.0) {
	if ([NSApp respondsToSelector:@selector(dockTile)])
		_dockTile = [NSApp dockTile];
	_applicationIconView = nil;
	_overlayIconView = nil;
}

MacOSXTaskbarManager::~MacOSXTaskbarManager() {
	clearApplicationIconView();
}

void MacOSXTaskbarManager::initApplicationIconView() {
	if (_dockTile == nil)
		return;
	if (_applicationIconView == nil) {
		_applicationIconView = [[NSImageView alloc] init];
		[_applicationIconView setImage:[NSApp applicationIconImage]];
		[_dockTile performSelector:@selector(setContentView:) withObject:_applicationIconView];
	}
}

void MacOSXTaskbarManager::clearApplicationIconView() {
	if (_dockTile == nil)
		return;
	[_dockTile performSelector:@selector(setContentView:) withObject:nil];
	[_applicationIconView release];
	_applicationIconView = nil;
}

void MacOSXTaskbarManager::initOverlayIconView() {
	if (_dockTile == nil)
		return;
	if (_overlayIconView == nil) {
		const double overlaySize = 0.75;
		initApplicationIconView();
		NSSize size = [_applicationIconView frame].size;
		_overlayIconView = [[NSImageView alloc] initWithFrame:NSMakeRect(size.width * (1.0-overlaySize), 0.0f, size.width * overlaySize, size.height * overlaySize)];
		[_overlayIconView setImageAlignment:NSImageAlignBottomRight];
		[_applicationIconView addSubview:_overlayIconView];
		[_overlayIconView release];
	}
}

void MacOSXTaskbarManager::clearOverlayIconView() {
	if (_progress < 0.0)
		clearApplicationIconView();
	else
		[_overlayIconView removeFromSuperview];
	_overlayIconView = nil;
}

void MacOSXTaskbarManager::setOverlayIcon(const Common::String &name, const Common::String &description) {
	if (_dockTile == nil)
		return;

    if (name.empty()) {
		clearOverlayIconView();
		[_dockTile performSelector:@selector(display)];
		return;
	}

	Common::String path = getIconPath(name);
	if (path.empty())
		return;

	initOverlayIconView();

	CFStringRef imageFile = CFStringCreateWithCString(0, path.c_str(), kCFStringEncodingASCII);
	NSImage *image = [[NSImage alloc] initWithContentsOfFile:(NSString *)imageFile];
	[_overlayIconView setImage:image];
	[image release];
	CFRelease(imageFile);

	[_dockTile performSelector:@selector(display)];
}

void MacOSXTaskbarManager::setProgressValue(int completed, int total) {
	if (_dockTile == nil)
		return;

	if (total > 0)
		_progress = (double)completed / (double)total;
	else if (_progress < 0)
		_progress = 0.0;

	 NSImage *mainIcon = [[NSApp applicationIconImage] copy];
	double barSize = [mainIcon size].width;
	double progressSize = barSize * _progress;
	[mainIcon lockFocus];
	[[NSColor colorWithDeviceRed:(40.0/255.0) green:(120.0/255.0) blue:(255.0/255.0) alpha:0.78] set];
	[NSBezierPath fillRect:NSMakeRect(0, 0, progressSize, 11)];
	[[NSColor colorWithDeviceRed:(241.0/255.0) green:(241.0/255.0) blue:(241.0/255.0) alpha:0.78] set];
	[NSBezierPath fillRect:NSMakeRect(progressSize, 0, barSize-progressSize, 11)];
	[mainIcon unlockFocus];

	initApplicationIconView();
	[_applicationIconView setImage:mainIcon];
	[mainIcon release];

	[_dockTile performSelector:@selector(display)];
}

void MacOSXTaskbarManager::setProgressState(TaskbarProgressState state) {
	if (_dockTile == nil)
		return;

	// Only support two states: visible and not visible.
	if (state == kTaskbarNoProgress) {
		_progress = -1.0;
		if (_overlayIconView == nil)
			clearApplicationIconView();
		else if (_applicationIconView != nil)
			[_applicationIconView setImage:[NSApp applicationIconImage]];
		return;
	}

	setProgressValue(-1, -1);
}

void MacOSXTaskbarManager::setCount(int count) {
	if (_dockTile == nil)
		return;

	if (count > 0)
		[_dockTile performSelector:@selector(setBadgeLabel:) withObject:[NSString stringWithFormat:@"%d", count]];
	else
		[_dockTile performSelector:@selector(setBadgeLabel:) withObject:nil];
}

void MacOSXTaskbarManager::notifyError() {
	if (_dockTile == nil)
		return;

	// NSImageNameCaution was introduced in 10.6.
	// For compatibility with older systems we should use something else (e.g. overlay label
	// or our own icon).
	//initOverlayIconView();
	//[_overlayIconView setImage:[NSImage imageNamed:NSImageNameCaution]];
	//[_dockTile performSelector:@selector(display)];
}

void MacOSXTaskbarManager::clearError() {
	if (_dockTile == nil)
		return;

    clearOverlayIconView();
	[_dockTile performSelector:@selector(display)];
	return;
}

Common::String MacOSXTaskbarManager::getIconPath(const Common::String& target) {
	// We first try to look for a iconspath configuration variable then
	// fallback to the extra path
	//
	// Icons can be either in a subfolder named "icons" or directly in the path

	Common::String iconsPath = ConfMan.get("iconspath");
	Common::String extraPath = ConfMan.get("extrapath");

#define TRY_ICON_PATH(path) { \
Common::FSNode node((path)); \
if (node.exists()) \
return (path); \
}

	if (!iconsPath.empty()) {
		TRY_ICON_PATH(iconsPath + "/" + target + ".png");
		TRY_ICON_PATH(iconsPath + "/" + ConfMan.get("gameid") + ".png");
		TRY_ICON_PATH(iconsPath + "/icons/" + target + ".png");
		TRY_ICON_PATH(iconsPath + "/icons/" + ConfMan.get("gameid") + ".png");
	}

	if (!extraPath.empty()) {
		TRY_ICON_PATH(extraPath + "/" + target + ".png");
		TRY_ICON_PATH(extraPath + "/" + ConfMan.get("gameid") + ".png");
		TRY_ICON_PATH(extraPath + "/icons/" + target + ".png");
		TRY_ICON_PATH(extraPath + "/icons/" + ConfMan.get("gameid") + ".png");
	}

	return "";
}

void MacOSXTaskbarManager::addRecent(const Common::String &name, const Common::String &description) {
	//warning("[MacOSXTaskbarManager::addRecent] Adding recent list entry: %s (%s)", name.c_str(), description.c_str());

	if (_dockTile == nil)
		return;

	// Store the game, description and icon in user preferences.
	// The NSDockTilePlugin will retrieve them there to list them in the dock tile menu.

	CFStringRef gameName = CFStringCreateWithCString(0, name.c_str(), kCFStringEncodingASCII);
	CFStringRef desc = CFStringCreateWithCString(0, description.c_str(), kCFStringEncodingASCII);

	// First build the dictionary for this game.
	NSMutableDictionary *dict = [[NSMutableDictionary  alloc] init];
	[dict setObject:(NSString *)gameName forKey:@"game"];
	[dict setObject:(NSString *)desc forKey:@"description"];

	// Icon
	Common::String iconPath = getIconPath(name);
	if (!iconPath.empty()) {
		CFStringRef icon = CFStringCreateWithCString(0, iconPath.c_str(), kCFStringEncodingASCII);
		[dict setObject:(NSString *)icon forKey:@"icon"];
		CFRelease(icon);
	}

	// Retrieve the current list of recent items and update it.
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	NSArray *oldArray = [defaults arrayForKey:@"recentGames"];
	if (oldArray == nil) {
		[defaults setObject:[NSArray arrayWithObject:dict] forKey:@"recentGames"];
	} else {
		NSMutableArray *newArray = [[NSMutableArray alloc] initWithArray:oldArray];
		// Insert the new game at the start
		[newArray insertObject:dict atIndex:0];
		// If the game was already present in the array, remove it
		for (unsigned int i = 1 ; i < [newArray count] ; ++i) {
			NSDictionary *oldDict = [newArray objectAtIndex:i];
			if (oldDict == nil)
				continue;
			NSString *oldGame = [oldDict valueForKey:@"game"];
			if (oldGame != nil && [oldGame isEqualToString:(NSString*)gameName]) {
				[newArray removeObjectAtIndex:i];
				break;
			}
		}
		// And make sure we limit the size of the array to 5 games
		if ([newArray count] > 5)
			[newArray removeLastObject];
		[defaults setObject:newArray forKey:@"recentGames"];
		[newArray release];
	}

	// Finally release the dictionary
	[dict release];
	CFRelease(gameName);
	CFRelease(desc);
}



#endif
