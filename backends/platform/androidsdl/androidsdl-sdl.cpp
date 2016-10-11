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

#include "backends/platform/androidsdl/androidsdl-sdl.h"
#include "backends/events/androidsdl/androidsdl-events.h"
#include "backends/graphics/androidsdl/androidsdl-graphics.h"

void OSystem_ANDROIDSDL::initBackend() {
	// Create the backend custom managers

	if (_eventSource == 0)
		_eventSource = new AndroidSdlEventSource();

	if (_graphicsManager == 0)
		_graphicsManager = new AndroidSdlGraphicsManager(_eventSource, _window);

	if (!ConfMan.hasKey("browser_lastpath"))
		ConfMan.set("browser_lastpath", "/storage");

	if (!ConfMan.hasKey("gfx_mode"))
		ConfMan.set("gfx_mode", "2x");

	// Call parent implementation of this method
	OSystem_POSIX::initBackend();
}
