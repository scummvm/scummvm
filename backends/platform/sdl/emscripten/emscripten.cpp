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

#include "backends/events/emscriptensdl/emscriptensdl-events.h"
#include "backends/fs/emscripten/emscripten-fs-factory.h"
#include "backends/mutex/null/null-mutex.h"
#include "backends/fs/emscripten/emscripten-fs-factory.h"
#include "backends/platform/sdl/emscripten/emscripten.h"
#include "backends/timer/default/default-timer.h"
#include "common/file.h"
#ifdef USE_TTS
#include "backends/text-to-speech/emscripten/emscripten-text-to-speech.h"
#endif

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

#ifdef USE_CLOUD
/* Listener to feed the activation JSON from the wizard at cloud.scummvm.org back 
 * Usage: Run the following on the final page of the activation flow:
 * 		  window.opener.postMessage(document.getElementById("json").value,"*")
 */
EM_JS(bool, cloud_connection_open_oauth_window, (char const *url), {
	oauth_window = window.open(UTF8ToString(url));
	window.addEventListener("message", (event) => {
		Module._cloud_connection_json_callback(stringToNewUTF8( JSON.stringify(event.data)));
		oauth_window.close()
	}, {once : true});
	return true;
});
#endif

extern "C" {
#ifdef USE_CLOUD
void EMSCRIPTEN_KEEPALIVE cloud_connection_json_callback(char *str) {
	warning("cloud_connection_callback: %s", str);
	OSystem_Emscripten *emscripten_g_system = dynamic_cast<OSystem_Emscripten *>(g_system);
	if (emscripten_g_system->_cloudConnectionCallback) {
		(*emscripten_g_system->_cloudConnectionCallback)(new Common::String(str));
	} else {
		warning("No Storage Connection Callback Registered!");
	}
}
#endif
}

// Overridden functions

void OSystem_Emscripten::initBackend() {
#ifdef USE_TTS
	// Initialize Text to Speech manager
	_textToSpeechManager = new EmscriptenTextToSpeechManager();
#endif

	// SDL Timers don't work in Emscripten unless threads are enabled or Asyncify is disabled.
	// We can do neither, so we use the DefaultTimerManager instead.
	_timerManager = new DefaultTimerManager();

	// Event source
	_eventSource = new EmscriptenSdlEventSource();

	// Invoke parent implementation of this method
	OSystem_POSIX::initBackend();
}

void OSystem_Emscripten::init() {
	// Initialze File System Factory
	EmscriptenFilesystemFactory *fsFactory = new EmscriptenFilesystemFactory();
	_fsFactory = fsFactory;

	// Invoke parent implementation of this method
	OSystem_SDL::init();
}

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

void OSystem_Emscripten::updateTimers() {
	// avoid a recursion loop if a timer callback decides to call OSystem::delayMillis()
	static bool inTimer = false;

	if (!inTimer) {
		inTimer = true;
		((DefaultTimerManager *)_timerManager)->checkTimers();
		inTimer = false;
	} else {
		const Common::ConfigManager::Domain *activeDomain = ConfMan.getActiveDomain();
		assert(activeDomain);

		warning("%s/%s calls update() from timer",
				activeDomain->getValOrDefault("engineid").c_str(),
				activeDomain->getValOrDefault("gameid").c_str());
	}
}

Common::MutexInternal *OSystem_Emscripten::createMutex() {
	return new NullMutexInternal();
}

void OSystem_Emscripten::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {
	// Add the global DATA_PATH (and some sub-folders) to the directory search list 
	// Note: gui-icons folder is added in GuiManager::initIconsSet 
	Common::FSNode dataNode(DATA_PATH);
	if (dataNode.exists() && dataNode.isDirectory()) {
		s.addDirectory(dataNode, priority, 2, false);
	}
}

void OSystem_Emscripten::delayMillis(uint msecs) {
	static uint32 lastThreshold = 0;
	const uint32 threshold = getMillis() + msecs;
	if (msecs == 0 && threshold - lastThreshold < 10) {
		return;
	}
	uint32 pause = 0;
	do {
#ifdef ENABLE_EVENTRECORDER
		if (!g_eventRec.processDelayMillis())
#endif
		SDL_Delay(pause);
		updateTimers();
		pause = getMillis() > threshold ? 0 : threshold - getMillis(); // Avoid negative values
		pause = pause > 10 ? 10 : pause; // ensure we don't pause for too long
	} while (pause > 0);
	lastThreshold = threshold;
}

#ifdef USE_CLOUD
bool OSystem_Emscripten::openUrl(const Common::String &url) {
	if(url.hasPrefix("https://cloud.scummvm.org/")){
		return cloud_connection_open_oauth_window(url.c_str());
	}
	return	OSystem_SDL::openUrl(url);
}
#endif

#endif
