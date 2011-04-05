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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#if defined(OPENPANDORA)

#include "backends/graphics/openpandora/op-graphics.h"
#include "backends/events/openpandora/op-events.h"
#include "backends/platform/openpandora/op-sdl.h"
#include "common/mutex.h"
#include "common/translation.h"
#include "common/util.h"

#include "graphics/scaler/aspect.h"
#include "graphics/surface.h"

OPGraphicsManager::OPGraphicsManager(SdlEventSource *boss)
	: SdlGraphicsManager(boss) {
}

bool OPGraphicsManager::loadGFXMode() {
	/* FIXME: For now we just cheat and set the overlay to 640*480 not 800*480 and let SDL
	   deal with the boarders (it saves cleaning up the overlay when the game screen is
	   smaller than the overlay ;)
	*/
	_videoMode.overlayWidth = 640;
	_videoMode.overlayHeight = 480;
	_videoMode.fullscreen = true;

	if (_videoMode.screenHeight != 200 && _videoMode.screenHeight != 400)
		_videoMode.aspectRatioCorrection = false;

	return SdlGraphicsManager::loadGFXMode();
}

#endif
