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

#ifdef __EMSCRIPTEN__


#define FORBIDDEN_SYMBOL_EXCEPTION_FILE
#include <emscripten.h>

#include "backends/platform/sdl/emscripten/emscripten.h"

// Inline JavaScript, see https://emscripten.org/docs/api_reference/emscripten.h.html#inline-assembly-javascript for details
EM_JS(bool, isFullscreen, (), {
	return !!document.fullscreenElement;
});

EM_JS(void, toggleFullscreen, (bool enable), {
	let canvas = document.getElementById('canvas');
	if (enable && !document.fullscreenElement) {
		canvas.requestFullscreen();
	}
	if (!enable && document.fullscreenElement) {
		document.exitFullscreen();
	}
});


// Overridden functions
bool OSystem_Emscripten::hasFeature(Feature f) {
	if (f == kFeatureFullscreenMode)
		return true;
	if (f == kFeatureNoQuit)
		return true;
	return OSystem_POSIX::hasFeature(f);
}

bool OSystem_Emscripten::getFeatureState(Feature f) {
	if (f == kFeatureFullscreenMode) {
		return isFullscreen();
	} else {
		return OSystem_POSIX::getFeatureState(f);
	}
}

void OSystem_Emscripten::setFeatureState(Feature f, bool enable) {
	if (f == kFeatureFullscreenMode) {
		toggleFullscreen(enable);
	} else {
		OSystem_POSIX::setFeatureState(f, enable);
	}
}

#endif
