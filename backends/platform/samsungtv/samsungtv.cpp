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

#include "common/scummsys.h"

#if defined(SAMSUNGTV)

#include "backends/platform/samsungtv/samsungtv.h"
#include "backends/events/samsungtvsdl/samsungtvsdl-events.h"
#include "backends/saves/default/default-saves.h"
#include "backends/fs/posix/posix-fs.h"
#include "common/textconsole.h"

void OSystem_SDL_SamsungTV::initBackend() {
	// Create the savefile manager
	if (_savefileManager == 0) {
		_savefileManager = new DefaultSaveFileManager("/mtd_wiselink/scummvm savegames");
	}

	// Create the events manager
	if (_eventSource == 0)
		_eventSource = new SamsungTVSdlEventSource();

	// Call parent implementation of this method
	OSystem_POSIX::initBackend();
}

void OSystem_SDL_SamsungTV::quit() {
	delete this;
}

void OSystem_SDL_SamsungTV::fatalError() {
	delete this;
	warning("ScummVM: Fatal internal error.");
	for (;;) {}
}

Common::String OSystem_SDL_SamsungTV::getDefaultConfigFileName() {
	return "/mtd_rwarea/.scummvmrc";
}

Common::String OSystem_SDL_SamsungTV::getDefaultLogFileName() {
	if (!Posix::assureDirectoryExists("/mtd_ram", nullptr)) {
		return Common::String();
	}

	return "/mtd_ram/scummvm.log";
}

bool OSystem_SDL_SamsungTV::hasFeature(Feature f) {
	if (f == kFeatureFullscreenMode)
		return false;

	return OSystem_SDL::hasFeature(f);
}

#endif
