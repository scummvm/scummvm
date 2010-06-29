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
 * $URL$
 * $Id$
 *
 */

#include "backends/platform/samsungtv/samsungtv.h"
#include "backends/events/samsungtvsdl/samsungtvsdl-events.h"

OSystem_SDL_SamsungTV::OSystem_SDL_SamsungTV()
	:
	OSystem_POSIX("/dtv/usb/sda1/.scummvmrc") {
}

bool OSystem_SDL_SamsungTV::hasFeature(Feature f) {
	return
		(f == OSystem::kFeatureAspectRatioCorrection) ||
		(f == OSystem::kFeatureCursorHasPalette);
}

void OSystem_SDL_SamsungTV::initBackend() {
	if (_eventManager == 0)
		_eventManager = new SamsungTVSdlEventManager(this);

	// Call parent implementation of this method
	OSystem_SDL::initBackend();
}

void OSystem_SDL_SamsungTV::setFeatureState(Feature f, bool enable) {
	switch (f) {
	case OSystem::kFeatureAspectRatioCorrection:
		_graphicsManager->setFeatureState(f, enable);
		break;
	default:
		break;
	}
}

bool OSystem_SDL_SamsungTV::getFeatureState(Feature f) {
	switch (f) {
	case OSystem::kFeatureAspectRatioCorrection:
		return _graphicsManager->getFeatureState(f);
	default:
		return false;
	}
}

void OSystem_SDL_SamsungTV::quit() {
	deinit();
}
