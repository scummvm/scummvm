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
 * $URL$
 * $Id$
 */

#include "gui/browser.h"
#include "gui/GuiManager.h"
#include "gui/ListWidget.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "common/algorithm.h"

#import <AppKit/NSOpenPanel.h>
#import <Foundation/NSString.h>

namespace GUI {

BrowserDialog::BrowserDialog(const char *title, bool dirBrowser)
	: Dialog("Browser") {
	_titleRef = CFStringCreateWithCString(0, title, CFStringGetSystemEncoding());
	_isDirBrowser = dirBrowser;
}

BrowserDialog::~BrowserDialog() {
	CFRelease(_titleRef);
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


	NSOpenPanel * panel = [NSOpenPanel openPanel];
	[panel setCanChooseDirectories:YES];
	if ([panel runModalForTypes:nil] == NSOKButton) {
#ifdef __POWERPC__
		const char *filename = [[panel filename] cString];
#else
		const char *filename = [[panel filename] cStringUsingEncoding:NSUTF8StringEncoding];
#endif
		_choice = Common::FSNode(filename);
		choiceMade = true;		
	}


	// If we were in fullscreen mode, switch back
	if (wasFullscreen) {
		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureFullscreenMode, true);
		g_system->endGFXTransaction();
	}

	return choiceMade;
}

}
