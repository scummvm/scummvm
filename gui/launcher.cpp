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

#include "common/config-manager.h"
#include "gui/gui-manager.h"
#include "gui/ThemeEval.h"

#include "gui/launcher.h"
#include "gui/launcher-dialog.h"

using Common::ConfigManager;

namespace GUI {

LauncherChooser::LauncherChooser() : _impl(nullptr) {}

LauncherChooser::~LauncherChooser() {
	delete _impl;
	_impl = nullptr;
}

#ifndef DISABLE_LAUNCHERDISPLAY_GRID
LauncherDisplayType getRequestedLauncherType() {
	const Common::String &userConfig = ConfMan.get("gui_launcher_chooser", Common::ConfigManager::kApplicationDomain);
	// If grid needs to be disabled on certain resolutions,
	// those conditions need to be added here
	if (userConfig.equalsIgnoreCase("grid") && g_gui.xmlEval()->getVar("Globals.GridSupported", 0)) {
		return kLauncherDisplayGrid;
	} else {
		return kLauncherDisplayList;
	}
}
#endif // !DISABLE_LAUNCHERDISPLAY_GRID

void LauncherChooser::selectLauncher() {
#ifndef DISABLE_LAUNCHERDISPLAY_GRID
	LauncherDisplayType requestedType = getRequestedLauncherType();
	if (!_impl || _impl->getType() != requestedType) {
		delete _impl;
		_impl = nullptr;

		switch (requestedType)
		{
		case kLauncherDisplayGrid:
			_impl = new LauncherGrid(Common::U32String("LauncherGrid"));
			break;

		default:
			// fallthrough intended
		case kLauncherDisplayList:
#endif // !DISABLE_LAUNCHERDISPLAY_GRID
			_impl = new LauncherSimple(Common::U32String("Launcher"));
#ifndef DISABLE_LAUNCHERDISPLAY_GRID
			break;
		}
	}
#endif // !DISABLE_LAUNCHERDISPLAY_GRID
}

int LauncherChooser::runModal() {
	if (!_impl)
		return -1;

	int ret;
	do {
		ret = _impl->run();
		if (ret == kSwitchLauncherDialog) {
			selectLauncher();
		}
	} while (ret < -1);
	return ret;
}

} // End of namespace GUI
