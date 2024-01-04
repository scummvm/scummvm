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
#define FORBIDDEN_SYMBOL_EXCEPTION_getenv
#include <emscripten.h>

#include "backends/platform/sdl/emscripten/emscripten.h"
#include "common/file.h"

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

EM_JS(void, downloadFile, (const char *filenamePtr, char *dataPtr, int dataSize), {
	const view = new Uint8Array(Module.HEAPU8.buffer, dataPtr, dataSize);
	const blob = new Blob([view], {
			type:
				'octet/stream'
		});
	const filename = UTF8ToString(filenamePtr);
	setTimeout(() => {
		const a = document.createElement('a');
		a.style = 'display:none';
		document.body.appendChild(a);
		const url = window.URL.createObjectURL(blob);
		a.href = url;
		a.download = filename;
		a.click();
		window.URL.revokeObjectURL(url);
		document.body.removeChild(a);
	}, 0);
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

Common::Path OSystem_Emscripten::getDefaultLogFileName() {
	return Common::Path("/tmp/scummvm.log");
}

Common::Path OSystem_Emscripten::getDefaultConfigFileName() {
	return Common::Path(Common::String::format("%s/scummvm.ini", getenv("HOME")));
}

Common::Path OSystem_Emscripten::getScreenshotsPath() {
	return Common::Path("/tmp/");
}

Common::Path OSystem_Emscripten::getDefaultIconsPath() {
	return Common::Path(DATA_PATH"/gui-icons/");
}

bool OSystem_Emscripten::displayLogFile() {
	if (_logFilePath.empty())
		return false;

	exportFile(_logFilePath);
	return true;
}

#ifdef USE_OPENGL
OSystem_SDL::GraphicsManagerType OSystem_Emscripten::getDefaultGraphicsManager() const {
	return GraphicsManagerOpenGL;
}
#endif

void OSystem_Emscripten::exportFile(const Common::Path &filename) {
	Common::File file;
	Common::FSNode node(filename);
	file.open(node);
	if (!file.isOpen()) {
		warning("Could not open file %s!", filename.toString(Common::Path::kNativeSeparator).c_str());
		return;
	}
	Common::String exportName = filename.getLastComponent().toString(Common::Path::kNativeSeparator);
	const int32 size = file.size();
	char *bytes = new char[size + 1];
	file.read(bytes, size);
	file.close();
	downloadFile(exportName.c_str(), bytes, size);
	delete[] bytes;
}
#endif
