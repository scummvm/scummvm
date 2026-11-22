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

#include "ww/ww.h"
#include "ww/gamelogic.h"
#include "ww/graphics.h"
#include "ww/gxlarchive.h"

#include "audio/audiostream.h"
#include "common/config-manager.h"
#include "common/error.h"
#include "common/savefile.h"
#include "engines/advancedDetector.h"
#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/palette.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"
#include "graphics/thumbnail.h"
#include "graphics/scaler.h"

namespace WW {

bool WWEngine::readSavegameHeader(Common::InSaveFile *in, SavegameHeader &header, bool skipThumbnail) {
	header.version = 0;
	header.saveName.clear();
	header.thumbnail = nullptr;
	header.saveYear = 0;
	header.saveMonth = 0;
	header.saveDay = 0;
	header.saveHour = 0;
	header.saveMinutes = 0;
	header.playTime = 0;

	// Get the savegame version
	header.version = in->readByte();
	if (header.version > kWWSavegameVersion)
		return false;

	// Read in the string
	char ch;
	while ((ch = (char)in->readByte()) != '\0')
		header.saveName += ch;

	// Get the thumbnail
	if (!Graphics::loadThumbnail(*in, header.thumbnail, skipThumbnail)) {
		return false;
	}

	// Read in save date/time
	header.saveYear = in->readSint16LE();
	header.saveMonth = in->readSint16LE();
	header.saveDay = in->readSint16LE();
	header.saveHour = in->readSint16LE();
	header.saveMinutes = in->readSint16LE();
	header.playTime = in->readUint32LE();

	return true;
}

Common::Error WWEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	Common::Error result = Common::kNoError;
	if (!_logic->saveSavegame(slot, &desc))
		result = Common::kUnknownError;

	return result;
}

Common::Error WWEngine::loadGameState(int slot) {
	Common::Error result = Common::kNoError;
	if (!_logic->loadSavegame(slot))
		result = Common::kUnknownError;

	return result;
}

void WWEngine::writeSavegameHeader(Common::OutSaveFile *out, SavegameHeader &header) {
	// Write out a savegame header
	out->write(savegameStr, kWWSavegameStrSize + 1);

	out->writeByte(kWWSavegameVersion);

	// Write savegame name
	out->write(header.saveName.c_str(), header.saveName.size() + 1);

	// Get the active palette
	uint8 thumbPalette[256 * 3];
	g_system->getPaletteManager()->grabPalette(thumbPalette, 0, 256);

	// Create a thumbnail and save it
	Graphics::Surface *thumb = new Graphics::Surface();
	_screen->saveScreenshot();

	::createThumbnail(thumb, (const byte *)_screen->_screenCopy->getPixels(), 320, 200, thumbPalette);
	Graphics::saveThumbnail(*out, *thumb);
	thumb->free();
	delete thumb;

	// Write out the save date/time
	TimeDate td;
	g_system->getTimeAndDate(td);
	out->writeSint16LE(td.tm_year + 1900);
	out->writeSint16LE(td.tm_mon + 1);
	out->writeSint16LE(td.tm_mday);
	out->writeSint16LE(td.tm_hour);
	out->writeSint16LE(td.tm_min);

	out->writeUint32LE(g_engine->getTotalPlayTime() / 1000);
}

void WWEngine::updateEvents() {
	Common::Event event;
	Common::Point clickPt;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			_keyCode = event.kbd.keycode;
			if (_keyCode == Common::KEYCODE_ESCAPE && _introOngoing) {
				_escPressed = true;
			}
			break;
		case Common::EVENT_KEYUP:
			_keyCode = Common::KEYCODE_INVALID;
			break;
		case Common::EVENT_MOUSEMOVE:
			// Restrict mouse move to the mouse allowed zone
			_mouseX = CLIP<int>(event.mouse.x, _mouseZone.left, _mouseZone.right - 1);
			_mouseY = CLIP<int>(event.mouse.y, _mouseZone.top, _mouseZone.bottom - 1);
			g_system->warpMouse(_mouseX, _mouseY);
			break;
		case Common::EVENT_LBUTTONDOWN:
			// Only consider the event if the mouse is in the mouse zone
			clickPt = event.mouse;
			if (_mouseZone.contains(clickPt)) {
				_mouseClickButtons |= kLeftButtonClicked;
				_mouseClickX = clickPt.x;
				_mouseClickY = clickPt.y;
				//_mouseClickButtons |= kLeftButtonDown;
			}
			break;
		case Common::EVENT_LBUTTONUP:
			//_mouseClickButtons &= ~kLeftButtonDown;
			break;
		case Common::EVENT_RBUTTONDOWN:
			// Only consider the event if the mouse is in the mouse zone
			clickPt = event.mouse;
			if(_mouseZone.contains(clickPt)) {
				_mouseClickButtons |= kRightButtonClicked;
				_mouseClickX = event.mouse.x;
				_mouseClickY = event.mouse.y;
				//_mouseClickButtons |= kRightButtonDown;
			}
			break;
		case Common::EVENT_RBUTTONUP:
			//_mouseClickButtons &= ~kRightButtonDown;
			break;
		case Common::EVENT_QUIT:
			_escPressed = true;
			quitGame();
			break;
		default:
			break;
		}
	}
}

int WWEngine::getRandom(int max) {
	return max <= 1 ? 0 : _random->getRandomNumber(max - 1);
}

void WWEngine::waitMillis(uint millis) {
	const uint32 waitTime = _system->getMillis() + millis;
	while (_system->getMillis() < waitTime && !shouldQuit() && !(_introOngoing && _escPressed)) {
		updateEvents();
		_system->updateScreen();
		_system->delayMillis(10);
	}
}

void WWEngine::waitSeconds(uint seconds) {
	waitMillis(seconds * 1000);
}

void WWEngine::initMouseCursor() {
	const uint kCursorWidth = 9;
	const uint kCursorHeight = 9;
	const uint kCursorHotspotX = 4;
	const uint kCursorHotspotY = 4;
	static const byte kCursorData[kCursorWidth * kCursorHeight] = {
		0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x0F,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00
	};
	CursorMan.replaceCursor(kCursorData, kCursorWidth, kCursorHeight, kCursorHotspotX, kCursorHotspotY, 0);
}

void WWEngine::handleMapPalette() {
	if (_gameMapHasPaletteHandler) {
		const uint32 curTicks = _system->getMillis();
		if (curTicks - _gameMapLastTicks > 55) {
			_gameMapLastTicks = curTicks;
			_gameMapCtr = (_gameMapCtr + 1) % 16;
			if (_gameMapCtr == 0) {
				paletteFadeColor(109, 1, 1, 1, 64);
			} else if (_gameMapCtr == 8) {
				paletteFadeColor(109, 56, 8, 5, 64);
			}
		}
	}
}

void WWEngine::handleMouseClick() {
	if (_mouseClickButtons & kLeftButtonClicked) {
		handleMouseLeftClick();
	}
	if (_mouseClickButtons & kRightButtonClicked) {
		handleMouseRightClick();
	}
	if (_mouseClickButtons & kKeyPressed) {
		warning("handleMouseClick - STUB handleKeyInput");
	}
}

void WWEngine::loadPalette(GxlArchive* lib, const char *filename) {
	Image::PCXDecoder *imageDecoder = lib->loadImage(filename);
	if (imageDecoder->getPalette().empty()) {
		warning("loadPalette() Could not load palette from '%s'", filename);
	} else {
		memcpy(_palette2, imageDecoder->getPalette().data(), Graphics::PALETTE_SIZE);
		g_system->getPaletteManager()->setPalette(_palette2, 0, 256);
	}
	delete imageDecoder;
}

void WWEngine::paletteFadeIn(int index, int count, int stepsSize) {
	byte fadePalette[768];
	const int fadeStartIndex = index * 3;
	const int fadeEndIndex = (index + count) * 3;
	bool isDone = false;
	// CHECKME Assume we're always fading in from black for now
	memset(fadePalette, 0, 768);
	while (!isDone) {
		isDone = true;
		for (int fadeIndex = fadeStartIndex; fadeIndex < fadeEndIndex; fadeIndex++) {
			const byte destValue = _palette2[fadeIndex];
			if (fadePalette[fadeIndex] < destValue) {
				fadePalette[fadeIndex] = MIN<int>(fadePalette[fadeIndex] + stepsSize, destValue);
				isDone = false;
			}
		}
		g_system->getPaletteManager()->setPalette(fadePalette, 0, 256);
		g_system->updateScreen();
		g_system->delayMillis(20);
	}
}

void WWEngine::paletteFadeOut(int index, int count, int stepsSize) {
	byte fadePalette[768];
	const int fadeStartIndex = index * 3;
	const int fadeEndIndex = (index + count) * 3;
	bool isDone = false;
	memcpy(fadePalette, _palette2, 768);
	while (!isDone) {
		isDone = true;
		for (int fadeIndex = fadeStartIndex; fadeIndex < fadeEndIndex; fadeIndex++) {
			if (fadePalette[fadeIndex] > 0) {
				fadePalette[fadeIndex] = MAX<int>(fadePalette[fadeIndex] - stepsSize, 0);
				isDone = false;
			}
		}
		g_system->getPaletteManager()->setPalette(fadePalette, 0, 256);
		g_system->updateScreen();
		g_system->delayMillis(20);
	}
}

void WWEngine::paletteFadeColor(int index, byte r, byte g, byte b, int steps) {
	byte fadePalette[768];
	g_system->getPaletteManager()->grabPalette(fadePalette, 0, 256);

	const int destR = r << 2;
	const int destG = g << 2;
	const int destB = b << 2;
	
	const int deltaR = fadePalette[index * 3] >= destR ? 1 : -1;
	const int deltaG = fadePalette[(index * 3) + 1] >= destG ? 1 : -1;
	const int deltaB = fadePalette[(index * 3) + 2] >= destB ? 1 : -1;
	
	while (fadePalette[index * 3] != destR && fadePalette[(index * 3) + 1] != destG && fadePalette[(index * 3) + 2] != destB) {
		fadePalette[index * 3] = ABS(fadePalette[index * 3] - destR) < steps ? destR : fadePalette[index * 3] - (steps * deltaR);
		fadePalette[(index * 3) + 1] = ABS(fadePalette[(index * 3) + 1] - destG) < steps ? destG : fadePalette[(index * 3) + 1] - (steps * deltaG);
		fadePalette[(index * 3) + 2] = ABS(fadePalette[(index * 3) + 2] - destB) < steps ? destB : fadePalette[(index * 3) + 2] - (steps * deltaB);

		g_system->getPaletteManager()->setPalette(fadePalette, 0, 256);
		g_system->updateScreen();
		g_system->delayMillis(20);
	}
	
}

void WWEngine::drawImageToSurfaceIntern(GxlArchive *lib, const char *filename, WWSurface *destSurface, int x, int y, bool transparent) {
	Image::PCXDecoder *imageDecoder = lib->loadImage(filename);
	if (transparent) {
		destSurface->drawSurfaceTransparent(imageDecoder->getSurface(), x, y);
	} else {
		destSurface->drawSurface(imageDecoder->getSurface(), x, y);
	}
	delete imageDecoder;
}

void WWEngine::drawImageToScreenIntern(GxlArchive *lib, const char *filename, int x, int y, bool transparent) {
	Image::PCXDecoder *imageDecoder = lib->loadImage(filename);
	if (transparent) {
		_screen->drawSurfaceTransparent(imageDecoder->getSurface(), x, y);
	} else {
		_screen->drawSurface(imageDecoder->getSurface(), x, y);
	}
	delete imageDecoder;
}

void WWEngine::drawImageToScreen(GxlArchive* lib, const char *filename, int x, int y, bool transparent) {
	drawImageToScreenIntern(lib, filename, x, y, transparent);
}

void WWEngine::drawImageToSurface(GxlArchive *lib, const char *filename, WWSurface *destSurface, int x, int y) {
	drawImageToSurfaceIntern(lib, filename, destSurface, x, y, false);
}

void WWEngine::drawRoomImageToBackground(const char *filename, int x, int y) {
	drawImageToSurfaceIntern(_roomGxl, filename, _backgroundSurface, x, y, false);
}

void WWEngine::drawRoomImageToBackgroundTransparent(const char *filename, int x, int y) {
	drawImageToSurfaceIntern(_roomGxl, filename, _backgroundSurface, x, y, true);
}

void WWEngine::drawRoomImageToScreen(const char *filename, int x, int y) {
	drawImageToScreenIntern(_roomGxl, filename, x, y, false);
}

void WWEngine::drawRoomImageToSurface(const char *filename, WWSurface *destSurface, int x, int y) {
	drawImageToSurfaceIntern(_roomGxl, filename, destSurface, x, y, false);
}

void WWEngine::drawSpiralEffect(Graphics::Surface *surface, int x, int y, int grainWidth, int grainHeight) {
	ScreenEffect screenEffect(this, surface, x, y, grainWidth, grainHeight);
	screenEffect.drawSpiralEffect();
}

void WWEngine::drawRandomEffect(Graphics::Surface *surface, int x, int y, int grainWidth, int grainHeight) {
	if (surface) {
		ScreenEffect screenEffect(this, surface, x, y, grainWidth, grainHeight);
		screenEffect.drawRandomEffect();
	} else {
		warning("%s() x:%d y:%d missing surface!", __func__, x, y);
	}
}

void WWEngine::drawWeaveEffect(Graphics::Surface* surface, int x, int y, int grainWidth, int grainHeight, int delay) {
	if (surface) {
		ScreenEffect screenEffect(this, surface, x, y, grainWidth, grainHeight);
		screenEffect.drawWeaveEffect();

	} else {
		warning("%s() x:%d y:%d missing surface!", __func__, x, y);
	}
}

void WWEngine::drawSlideEffect(Graphics::Surface* surface, int x, int y, int grainWidth, int grainHeight, int delay) {
	if (surface) {
		ScreenEffect screenEffect(this, surface, x, y, grainWidth, grainHeight);
		screenEffect.drawSlideEffect();
	} else {
		warning("%s() x:%d y:%d missing surface!", __func__, x, y);
	}
}

void WWEngine::drawCurrentTextToSurface(WWSurface *destSurface, int x, int y) {
	drawCurrentText(x, y, destSurface);
}

void WWEngine::displayText(const char *filename, int index, int flag, int x, int y, int drawToVirtual) {
	_currentText = loadString(filename, index, flag);
	drawCurrentText(x, y, nullptr);
}

void WWEngine::displayTextLines(const char *filename, int baseIndex, int x, int y, int count) {
	int ticks = 3000;
	if (count < 0) {
		ticks = -count;
		count = 1;
	}
	for (int textIndex = 0; textIndex < count; textIndex++) {
		displayText(filename, baseIndex + textIndex, 0, x, y, 0);
		_isTextVisible = false;
		waitMillis(ticks);
		refreshActors();
	}
}

void WWEngine::playSound(const char *filename, bool flag, int freq) {
	if (_isSoundEnabled)
		_sound->playSound(filename, flag, freq);
}

void WWEngine::changeMusic() {
	if (!_isMusicEnabled)
		return;

	_midi->stopSong();

	switch (_musicIndex) {
	case 0:
		_midi->playMusic("pop-a.xmi");
		break;
	case 1:
		_midi->playMusic("metal1.xmi");
		break;
	case 2:
		_midi->playMusic("metal2.xmi");
		break;
	default:
		_midi->playMusic("metal3.xmi");
		break;
	}
}

void WWEngine::changeMusic(const char *filename) {
	if (!_isMusicEnabled)
		return;

	_midi->stopSong();
	_midi->playMusic(filename);
}

void WWEngine::stopMusic() {
	_midi->stopSong();
}

void WWEngine::redrawInventory() {
	// NOTE This seems to hide the inventory
	_inventoryItemsCount = 1;
	drawInventory();
	refreshActors();
}

void WWEngine::refreshInventory(bool doRefresh) {
	// NOTE This seems to show the inventory
	if (_inventoryItemsCount != 0) {
		drawInventory();
		drawInventory();
	}
	if (doRefresh) {
		refreshActors();
	}
}

int WWEngine::getActorScaleFromY(int actorY) {
	return _logic->getActorScaleFromY(actorY);
}

void WWEngine::playAnimation(const char *prefix, int startIndex, int count, int x, int y, int flag, uint ticks) {
	Common::String filename;
	// sysMouseDriver(2);
	if (count > 0) {
		for (int index = startIndex; index < startIndex + count; index++) {
			updateRoomAnimations();
			filename = Common::String::format("%s%d.pcx", prefix, index);
			drawRoomImageToScreen(filename.c_str(), x, y);
			drawRoomImageToBackground(filename.c_str(), x, y);
			waitMillis(ticks);
		}
	} else {
		for (int index = startIndex; index > startIndex + count; index--) {
			updateRoomAnimations();
			filename = Common::String::format("%s%d.pcx", prefix, index);
			drawRoomImageToScreen(filename.c_str(), x, y);
			drawRoomImageToBackground(filename.c_str(), x, y);
			waitMillis(ticks);
		}
	}
	if (flag) {
		filename = Common::String::format("%s%d.pcx", prefix, startIndex);
		drawRoomImageToScreen(filename.c_str(), x, y);
		drawRoomImageToBackground(filename.c_str(), x, y);
	}
	// sysMouseDriver(1)
}

void WWEngine::playAnimationLoops(const char *prefix, int startIndex, int count, int x, int y, int flag, uint ticks, int loopCount) {
	for (int loop = 0; loop < loopCount; loop++) {
		playAnimation(prefix, startIndex, count, x, y, flag, ticks);
	}
}

void WWEngine::openAlternateRoomLibrary(const char *name) {
	_roomName = Common::String(name);
	delete _roomGxl;

	_roomGxl = new GxlArchive(_roomName);
}

void WWEngine::loadRoomBackground() {
	const int wayneX = _wayneSpriteX, wayneY = _wayneSpriteY;
	const int garthX = _garthSpriteX, garthY = _garthSpriteY;

	loadPalette(_roomGxl, "backg.pcx");
	g_system->getPaletteManager()->setPalette(_palette2, 0, 256);

	drawRoomImageToSurface("backg.pcx", _backgroundSurface, 0, 0);
	refreshRoomBackground(_currentRoomNumber);

	if (_isRestoringRoom) {
		_isRestoringRoom = false;
		// Undo what the refresh did for a room entry
		setWaynePosition(wayneX, wayneY);
		setGarthPosition(garthX, garthY);

		// room 0 must keep reopening the menu until the Pizzathon starts
		if (_currentRoomNumber != 0)
			_roomEventNum = 0;
	}

	refreshActors();
}

void WWEngine::changeRoom(int roomNum) {
	if (_currentRoomNumber != -1) {
		// GxlCloseLib is included in openRoomLibrary, no need to call it here
		unloadStaticRoomObjects();
	}
	openRoomLibrary(roomNum);
	if (_wayneSpriteX != -1) {
		loadRoomMask(roomNum);
		loadStaticRoomObjects(roomNum);
	}
	_actorSpriteValue = 0;
	_currentRoomNumber = roomNum;
	_roomChangeCtr++;
	loadRoomBackground();
}

void WWEngine::refreshRoomBackground(int roomNum) {
	_logic->refreshRoomBackground(roomNum);
}

void WWEngine::handleRoomEvent() {
	if (_roomEventNum != 0) {
		const int eventNum = _roomEventNum;
		_roomEventNum = 0;
		_logic->handleRoomEvent(eventNum);
	}
}

void WWEngine::fillRoomMaskArea(int x1, int y1, int x2, int y2, bool blocked) {
	for (int y = y1; y <= y2; y++) {
		for (int x = x1; x <= x2; x++) {
			const int offset = (y * 40) + (x / 8);
			const byte value = 0x80 >> (x % 8);
			if (blocked) {
				_walkMap[offset] &= ~value;
			} else {
				_walkMap[offset] |= value;
			}
		}
	}
}

void WWEngine::loadAnimationSpriteRange(int baseIndex, const char *filename, int count) {
	for (int index = 0; index < count; index++) {
		Common::String tempFilename = Common::String::format("%s%d.pcx", filename, index);
		loadAnimationSprite(baseIndex + index, tempFilename.c_str());
	}
}

void WWEngine::loadAnimationSprite(int index, const char *filename) {
	delete _roomAnimations[index];
	_roomAnimations[index] = _roomGxl->loadRoomSurface(filename);
}

void WWEngine::drawAnimationSprite(int index, int x, int y) {
	_animationsRedrawBackground = true;
	_backgroundSurface->drawSurface(_roomAnimations[index], x, y);
}

void WWEngine::drawAnimationSpriteTransparent(int index, int x, int y) {
	_animationsRedrawBackground = true;
	_backgroundSurface->drawSurfaceTransparent(_roomAnimations[index], x, y);
}

void WWEngine::updateRoomAnimations() {
	if (_hasRoomAnimationCallback) {
		_animationsRedrawBackground = false;
		updateAnimationTimers();
		_logic->updateRoomAnimations();
		if (_animationsRedrawBackground) {
			_hasRoomAnimationCallback = false;
			refreshActors();
			_hasRoomAnimationCallback = true;
		}
	}
}

void WWEngine::startRoomAnimations() {
	_animationsCtr = 0;
	_hasRoomAnimationCallback = true;
}

void WWEngine::stopRoomAnimations() {
	for (uint i = 0; i < kRoomAnimationsCount; i++) {
		delete _roomAnimations[i];
		_roomAnimations[i] = nullptr;
	}
	_hasRoomAnimationCallback = false;
}

void WWEngine::updateAnimationTimers() {
	const uint32 currentTicks = _system->getMillis();
	for (uint i = 0; i < kAnimationTimersCount; i++) {
		AnimationTimer &animationTimer = _animationTimers[i];
		animationTimer.expired = animationTimer.nextUpdateTicks == 0 || currentTicks >= animationTimer.nextUpdateTicks;
		if (animationTimer.expired) {
			if (animationTimer.nextUpdateTicks > 0) {
				++animationTimer.counter;
			}
			animationTimer.nextUpdateTicks = currentTicks + animationTimer.delay;
		}
	}
}

void WWEngine::setAnimationTimer(uint index, uint32 delay, int initialCounter) {
	AnimationTimer &animationTimer = _animationTimers[index];
	animationTimer.nextUpdateTicks = 0;
	animationTimer.delay = delay;
	animationTimer.counter = initialCounter;
	animationTimer.expired = false;
}

bool WWEngine::isAnimationTimerExpired(uint index) {
	return _animationTimers[index].expired;
}

int WWEngine::getAnimationTimerCounter(uint index) {
	return _animationTimers[index].counter;
}

void WWEngine::initStaticRoomObjects() {
	for (uint i = 0; i < kStaticRoomObjectsCount; i++)
		_staticRoomObjects[i] = kStaticRoomObjects[i];
}

void WWEngine::loadStaticRoomObjects(int roomNum) {
	const int startIndex = kStaticRoomObjectsMap[roomNum].index;
	const int count = kStaticRoomObjectsMap[roomNum].count;
	for (int index = 0; index < count; index++) {
		const StaticRoomObject &roomObject = _staticRoomObjects[startIndex + index];
		if (roomObject.x1 != -1) {
			debug("%s", roomObject.name);
			Common::String objectName = Common::String::format("%s.pcx", roomObject.name);
			_staticRoomObjectSprites[index] = _roomGxl->loadRoomSurface(objectName.c_str());
			_backgroundSurface->drawSurfaceTransparent(_staticRoomObjectSprites[index], roomObject.x1, roomObject.y1);
		}
	}
}

void WWEngine::unloadStaticRoomObjects() {
	for (uint i = 0; i < kStaticRoomObjectSpritesCount; i++) {
		delete _staticRoomObjectSprites[i];
		_staticRoomObjectSprites[i] = nullptr;
	}
}

void WWEngine::setStaticRoomObjectPosition(int roomNum, int fromIndex, int toIndex, int x, int y) {
	const int startIndex = kStaticRoomObjectsMap[roomNum].index + fromIndex;
	const int endIndex = kStaticRoomObjectsMap[roomNum].index + toIndex;
	unloadStaticRoomObjects();
	for (int index = startIndex; index <= endIndex; index++) {
		StaticRoomObject &roomObject = _staticRoomObjects[index];
		roomObject.x1 = x;
		roomObject.y1 = y;
	}
	loadStaticRoomObjects(roomNum);
}

void WWEngine::drawStaticRoomObjects(int roomNum, int x, int y, int actorHeight, int actorWidth, WWSurface *surface) {
	const int x1 = x - actorWidth;
	const int x2 = x + actorWidth;
	const int y2 = y + actorHeight - 48;
	const int startIndex = kStaticRoomObjectsMap[roomNum].index;
	const int count = kStaticRoomObjectsMap[roomNum].count;
	for (int index = 0; index < count; index++) {
		const StaticRoomObject &roomObject = _staticRoomObjects[startIndex + index];
		if (roomObject.x1 != -1 && roomObject.y2 > y &&
			((roomObject.x1 <= x1 && roomObject.x2 >= x1) || (roomObject.x1 <= x2 && roomObject.x2 >= x2) || (roomObject.x1 >= x1 && roomObject.x2 <= x2))) {
			surface->drawSurfaceTransparent(_staticRoomObjectSprites[index], roomObject.x1 - x1, roomObject.y1 - y2);
		}
	}
}

void WWEngine::initRoomObjects() {
	for (int i = 0; i < kRoomObjectsCount; i++)
		_roomObjects[i] = kRoomObjects[i];
}

const RoomObject *WWEngine::getRoomObject(int objectId) {
	return &_roomObjects[objectId];
}

const char *WWEngine::getRoomObjectName(int objectId) {
	return _roomObjects[objectId].name;
}

int WWEngine::getObjectRoom(int objectId) {
	return _roomObjects[objectId].roomNumber;
}

int WWEngine::getObjectDirection(int objectId) {
	return _roomObjects[objectId].direction;
}

int WWEngine::findRoomObjectIdAtPoint(int x, int y) {
	for (int index = 0; index < kRoomObjectsCount; index++) {
		const RoomObject *roomObject = getRoomObject(index);
		if (roomObject->roomNumber == _currentRoomNumber &&
			x >= roomObject->x1 && x <= roomObject->x2 &&
			y >= roomObject->y1 && y <= roomObject->y2) {
			return index;
		}
	}
	return -1;
}

void WWEngine::gxCloseLib(GxlArchive *lib) {
	delete lib;
	lib = nullptr;
}

void WWEngine::setMouseBounds(int x1, int x2, int y1, int y2) {
	// Add one as rect.contains() use a strict < comparison for x2 and y2
	_mouseZone = Common::Rect(x1, y1, x2 + 1, y2 + 1);
}

uint WWEngine::updateGame() {
	updateEvents();
	updateMouseMove();

	// Take the clicks collected so far and reset them before handling them: a click made
	// while they're being handled is kept for the next call
	const uint clickButtons = _mouseClickButtons;
	_mouseClickButtons = 0;

	if (clickButtons & kLeftButtonClicked)
		handleMouseLeftClick();
	if (clickButtons & kRightButtonClicked)
		handleMouseRightClick();

	if (_roomEventNum != 0)
		handleRoomEvent();
	if (_gameState == 1 && _currentRoomNumber < 100)
		gameMapOpen();

	return clickButtons;
}
} // End of namespace WW
