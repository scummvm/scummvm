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

#if defined(DINGUX)

#include "backends/platform/dingux/dingux.h"
#include "backends/events/dinguxsdl/dinguxsdl-events.h"

void OSystem_SDL_Dingux::initBackend() {
	ConfMan.registerDefault("fullscreen", true);

	// Create the events manager
	if (_eventSource == 0)
		_eventSource = new DINGUXSdlEventSource();

	// Call parent implementation of this method
	OSystem_POSIX::initBackend();
}

bool OSystem_SDL_Dingux::hasFeature(Feature f) {
	if (f == kFeatureFullscreenMode)
		return false;

	return OSystem_SDL::hasFeature(f);
}

#endif
