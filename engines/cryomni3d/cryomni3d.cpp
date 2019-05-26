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
#include "common/error.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "common/debug-channels.h"

#include "common/events.h"
#include "common/file.h"

#include "audio/mixer.h"
#include "graphics/palette.h"

#include "cryomni3d/cryomni3d.h"

#include "cryomni3d/image/hlz.h"
#include "cryomni3d/video/hnm_decoder.h"

namespace CryOmni3D {

CryOmni3DEngine::CryOmni3DEngine(OSystem *syst,
                                 const CryOmni3DGameDescription *gamedesc) : Engine(syst), _gameDescription(gamedesc),
	_fontManager(), _sprites(), _dragStatus(kDragStatus_NoDrag), _autoRepeatNextEvent(-1u) {
	if (!_mixer->isReady()) {
		error("Sound initialization failed");
	}

	// Setup mixer
	syncSoundSettings();

	unlockPalette();

	DebugMan.addDebugChannel(kDebugFile, "File", "Track File Accesses");
	DebugMan.addDebugChannel(kDebugVariable, "Variable", "Track Variable Accesses");
	DebugMan.addDebugChannel(kDebugSaveLoad, "SaveLoad", "Track Save/Load Function");
}

CryOmni3DEngine::~CryOmni3DEngine() {
	DebugMan.clearAllDebugChannels();
}

Common::Error CryOmni3DEngine::run() {
	return Common::kNoError;
}

void CryOmni3DEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);

	/*
	if (pause) {
	    _video->pauseVideos();
	} else {
	    _video->resumeVideos();
	    _system->updateScreen();
	}
	*/
}

Common::String CryOmni3DEngine::prepareFileName(const Common::String &baseName,
        const char *const *extensions) const {
	Common::String fname(baseName);

	int lastDotPos = fname.size() - 1;
	for (; lastDotPos >= 0; --lastDotPos) {
		if (fname[lastDotPos] == '.') {
			break;
		}
	}

	int extBegin;
	if (lastDotPos > -1) {
		extBegin = lastDotPos + 1;
		fname.erase(extBegin);
	} else {
		fname += ".";
		extBegin = fname.size();
	}

	while (*extensions != nullptr) {
		fname += *extensions;
		debug("Trying file %s", fname.c_str());
		if (Common::File::exists(fname)) {
			return fname;
		}
		fname.erase(extBegin);
		extensions++;
	}
	fname.deleteLastChar();
	warning("Failed to find file %s/%s", baseName.c_str(), fname.c_str());
	return baseName;
}

void CryOmni3DEngine::playHNM(const Common::String &filename, Audio::Mixer::SoundType soundType,
                              HNMCallback beforeDraw, HNMCallback afterDraw) {
	const char *const extensions[] = { "hns", "hnm", nullptr };
	Common::String fname(prepareFileName(filename, extensions));

	byte *currentPalette = new byte[256 * 3];
	g_system->getPaletteManager()->grabPalette(currentPalette, 0, 256);

	// Pass the ownership of currentPalette to HNMDecoder
	Video::VideoDecoder *videoDecoder = new Video::HNMDecoder(false, currentPalette);
	videoDecoder->setSoundType(soundType);

	if (!videoDecoder->loadFile(fname)) {
		warning("Failed to open movie file %s/%s", filename.c_str(), fname.c_str());
		delete videoDecoder;
		return;
	}

	videoDecoder->start();

	uint16 width = videoDecoder->getWidth();
	uint16 height = videoDecoder->getHeight();

	bool skipVideo = false;
	unsigned int frameNum = 0;
	while (!g_engine->shouldQuit() && !videoDecoder->endOfVideo() && !skipVideo) {
		if (videoDecoder->needsUpdate()) {
			const Graphics::Surface *frame = videoDecoder->decodeNextFrame();

			if (frame) {
				if (videoDecoder->hasDirtyPalette()) {
					const byte *palette = videoDecoder->getPalette();
					setPalette(palette, 0, 256);
				}

				if (beforeDraw) {
					(this->*beforeDraw)(frameNum);
				}
				g_system->copyRectToScreen(frame->getPixels(), frame->pitch, 0, 0, width, height);
				if (afterDraw) {
					(this->*afterDraw)(frameNum);
				}

				frameNum++;
			}
		}
		g_system->updateScreen();

		if (pollEvents() && checkKeysPressed()) {
			skipVideo = true;
		}
	}

	delete videoDecoder;
}

Image::ImageDecoder *CryOmni3DEngine::loadHLZ(const Common::String &filename) {
	Common::String fname(prepareFileName(filename, "hlz"));

	Image::ImageDecoder *imageDecoder = new Image::HLZFileDecoder();

	Common::File file;

	if (!file.open(fname)) {
		warning("Failed to open hlz file %s/%s", filename.c_str(), fname.c_str());
		return nullptr;
	}

	if (!imageDecoder->loadStream(file)) {
		warning("Failed to open hlz file %s", fname.c_str());
		delete imageDecoder;
		imageDecoder = 0;
		return nullptr;
	}

	return imageDecoder;
}

void CryOmni3DEngine::displayHLZ(const Common::String &filename) {
	Image::ImageDecoder *imageDecoder = loadHLZ(filename);

	if (!imageDecoder) {
		return;
	}

	if (imageDecoder->hasPalette()) {
		const byte *palette = imageDecoder->getPalette();
		setPalette(palette, imageDecoder->getPaletteStartIndex(), imageDecoder->getPaletteColorCount());
	}

	const Graphics::Surface *frame = imageDecoder->getSurface();
	g_system->copyRectToScreen(frame->getPixels(), frame->pitch, 0, 0, frame->w, frame->h);
	g_system->updateScreen();

	bool exitImg = false;
	while (!g_engine->shouldQuit() && !exitImg) {
		if (pollEvents()) {
			if (checkKeysPressed(1, Common::KEYCODE_ESCAPE) || getCurrentMouseButton() == 1) {
				exitImg = true;
			}
		}
		g_system->updateScreen();
	}

	delete imageDecoder;
}

void CryOmni3DEngine::setCursor(const Graphics::Cursor &cursor) const {
	g_system->setMouseCursor(cursor.getSurface(), cursor.getWidth(), cursor.getHeight(),
	                         cursor.getHotspotX(), cursor.getHotspotY(), cursor.getKeyColor());
}

void CryOmni3DEngine::setCursor(unsigned int cursorId) const {
	const Graphics::Cursor &cursor = _sprites.getCursor(cursorId);
	g_system->setMouseCursor(cursor.getSurface(), cursor.getWidth(), cursor.getHeight(),
	                         cursor.getHotspotX(), cursor.getHotspotY(), cursor.getKeyColor());
}

bool CryOmni3DEngine::pollEvents() {
	Common::Event event;
	bool hasEvents = false;

	unsigned int oldMouseButton = getCurrentMouseButton();

	while (g_system->getEventManager()->pollEvent(event)) {
		if (event.type == Common::EVENT_KEYDOWN) {
			_keysPressed.push(event.kbd);
		}
		hasEvents = true;
	}
	g_system->delayMillis(10);

	_dragStatus = kDragStatus_NoDrag;
	unsigned int currentMouseButton = getCurrentMouseButton();
	if (!oldMouseButton && currentMouseButton == 1) {
		// Starting the drag
		_dragStatus = kDragStatus_Pressed;
		_dragStart = getMousePos();
	} else if (oldMouseButton == 1) {
		// We were already pressing
		if (currentMouseButton == 1) {
			// We are still pressing
			Common::Point delta = _dragStart - getMousePos();
			if (ABS(delta.x) > 2 || ABS(delta.y) > 2) {
				// We moved from the start point
				_dragStatus = kDragStatus_Dragging;
			} else if (_autoRepeatNextEvent != -1u) {
				// Check for auto repeat duration
				if (_autoRepeatNextEvent < g_system->getMillis()) {
					_dragStatus = kDragStatus_Pressed;
				}
			}
		} else {
			// We just finished dragging
			_dragStatus = kDragStatus_Finished;
			// Cancel auto repeat
			_autoRepeatNextEvent = -1;
		}
	}
	// Else we weren't dragging and still aren't

	return hasEvents;
}

void CryOmni3DEngine::setAutoRepeatClick(unsigned int millis) {
	_autoRepeatNextEvent = g_system->getMillis() + millis;
}

unsigned int CryOmni3DEngine::getCurrentMouseButton() {
	int mask = g_system->getEventManager()->getButtonState();
	if (mask & 0x1) {
		return 1;
	} else if (mask & 0x2) {
		return 2;
	} else {
		return 0;
	}
}

void CryOmni3DEngine::waitMouseRelease() {
	while (g_system->getEventManager()->getButtonState() != 0 && !g_engine->shouldQuit()) {
		pollEvents();
	}
}

void CryOmni3DEngine::setMousePos(const Common::Point &point) {
	g_system->warpMouse(point.x, point.y);
	// Ensure to update mouse position in event manager
	pollEvents();
}

Common::Point CryOmni3DEngine::getMousePos() {
	return g_system->getEventManager()->getMousePos();
}

Common::KeyState CryOmni3DEngine::getNextKey() {
	if (_keysPressed.empty()) {
		return Common::KeyState();
	} else {
		return _keysPressed.pop();
	}
}

bool CryOmni3DEngine::checkKeysPressed() {
	Common::KeyCode kc = getNextKey().keycode;
	if (kc != Common::KEYCODE_INVALID) {
		clearKeys();
		return true;
	} else {
		return false;
	}
}

bool CryOmni3DEngine::checkKeysPressed(unsigned int numKeys, ...) {
	bool found = false;
	Common::KeyCode kc = getNextKey().keycode;
	while (!found && kc != Common::KEYCODE_INVALID) {
		va_list va;
		va_start(va, numKeys);
		for (unsigned int i = 0; i < numKeys; i++) {
			// Compiler says that KeyCode is promoted to int, so we need this ugly cast
			Common::KeyCode match = (Common::KeyCode) va_arg(va, int);
			if (match == kc) {
				found = true;
				break;
			}
		}
		va_end(va);
		kc = getNextKey().keycode;
	}
	clearKeys();
	return found;
}

void CryOmni3DEngine::copySubPalette(byte *dst, const byte *src, uint start, uint num) {
	assert(start < 256);
	assert(start + num < 256);
	memcpy(&dst[3 * start], &src[3 * start], 3 * num * sizeof(*dst));
}

void CryOmni3DEngine::setPalette(const byte *colors, uint start, uint num) {
	if (start < _lockPaletteStartRW) {
		colors = colors + 3 * (_lockPaletteStartRW - start);
		start = _lockPaletteStartRW;
	}
	uint end = start + num - 1;
	if (end > _lockPaletteEndRW) {
		num = num - (end - _lockPaletteEndRW);
		end = _lockPaletteEndRW;
	}
	g_system->getPaletteManager()->setPalette(colors, start, num);
	// Don't update screen there: palette will be updated with next updateScreen call
}

void CryOmni3DEngine::fadeOutPalette() {
	byte palOut[256 * 3];
	uint16 palWork[256 * 3];
	uint16 delta[256 * 3];

	g_system->getPaletteManager()->grabPalette(palOut, 0, 256);
	for (unsigned int i = 0; i < 256 * 3; i++) {
		palWork[i] = palOut[i] << 8;
		delta[i] = palWork[i] / 25;
	}

	for (unsigned int step = 0; step < 25 && !g_engine->shouldQuit(); step++) {
		for (unsigned int i = 0; i < 256 * 3; i++) {
			palWork[i] -= delta[i];
			palOut[i] = palWork[i] >> 8;
		}
		setPalette(palOut, 0, 256);
		// Wait 50ms between each steps but refresh screen every 10ms
		for (unsigned int i = 0; i < 5; i++) {
			g_system->updateScreen();
			g_system->delayMillis(10);
		}
	}
	setBlackPalette();
}

void CryOmni3DEngine::fadeInPalette(const byte *palette) {
	byte palOut[256 * 3];
	uint16 palWork[256 * 3];
	uint16 delta[256 * 3];

	memset(palOut, 0, sizeof(palOut));
	memset(palWork, 0, sizeof(palWork));
	for (unsigned int i = 0; i < 256 * 3; i++) {
		delta[i] = (palette[i] << 8) / 25;
	}

	setBlackPalette();
	for (unsigned int step = 0; step < 25 && !g_engine->shouldQuit(); step++) {
		for (unsigned int i = 0; i < 256 * 3; i++) {
			palWork[i] += delta[i];
			palOut[i] = palWork[i] >> 8;
		}
		setPalette(palOut, 0, 256);
		// Wait 50ms between each steps but refresh screen every 10ms
		for (unsigned int i = 0; i < 5; i++) {
			g_system->updateScreen();
			g_system->delayMillis(10);
		}
	}
	setPalette(palette, 0, 256);
	g_system->updateScreen();
}

void CryOmni3DEngine::setBlackPalette() {
	byte pal[256 * 3];
	memset(pal, 0, 256 * 3);
	g_system->getPaletteManager()->setPalette(pal, 0, 256);
	g_system->updateScreen();
}

void CryOmni3DEngine::fillSurface(byte color) {
	g_system->fillScreen(color);
	g_system->updateScreen();
}
} // End of namespace CryOmni3D
