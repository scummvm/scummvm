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

#include "common/algorithm.h"
#include "common/fs.h"
#include "common/ustr.h"
#include "graphics/font.h"
#include "graphics/screen.h"
#include "graphics/sjis.h"
#include "graphics/surface.h"
#include "video/avi_decoder.h"

#include "harold/bcb/bcb.h"
#include "harold/bcb/script.h"
#include "harold/bcb/static_data.h"

namespace BCB {

static const uint32 kScriptSecondaryStateByteOffset = 0x98;
bool BCBEngine::showMapScreen() {
	Graphics::Surface *map = loadBitmap("16mmap.bmp");
	if (!map)
		return false;

	applyPendingClockTickDelta();
	renderClockAndChipToSurface(*map, 0, stateDword(kStateGameClockTicks));
	if (_mapControlsActive)
		composeMapControlsToSurface(*map, *map);
	blitScreenWithLineReveal(*map, false);
	map->free();
	delete map;
	_currentDisplayState = 2;
	_lastTimerTick = g_system->getMillis();
	_screen->update();
	return true;
}

bool BCBEngine::openMapMoveMovie() {
	if (_mapMoveVideo)
		return true;

	Common::SeekableReadStream *stream = createActiveDiscReadStream("bgm/move.avi");
	if (!stream)
		return false;

	_mapMoveVideo = new Video::AVIDecoder();
	_mapMoveVideo->setSoundType(Audio::Mixer::kMusicSoundType);
	if (!_mapMoveVideo->loadStream(stream)) {
		delete _mapMoveVideo;
		_mapMoveVideo = nullptr;
		return false;
	}

	_mapMoveVideo->setOutputPixelFormat(g_system->getScreenFormat());
	return true;
}

void BCBEngine::startMapMoveMovie() {
	if (!_mapMoveVideo && !openMapMoveMovie())
		return;

	_mapMoveVideo->start();
}

void BCBEngine::stopMapMoveMovie() {
	if (!_mapMoveVideo)
		return;

	_mapMoveVideo->stop();
	delete _mapMoveVideo;
	_mapMoveVideo = nullptr;
}

bool BCBEngine::isMapButtonEnabled(uint buttonIndex) const {
	if (buttonIndex == 0 || buttonIndex >= ARRAYSIZE(kMapButtonTable))
		return false;

	const int secondaryStateIndex = kMapButtonSecondaryStateIndexTable[buttonIndex];
	return secondaryStateIndex == 0 || stateDword(kScriptSecondaryStateByteOffset + secondaryStateIndex * 4) != 0;
}

void BCBEngine::drawMapButtonPressed(uint buttonIndex, bool present) {
	if (buttonIndex == 0 || buttonIndex >= ARRAYSIZE(kMapButtonTable))
		return;

	const Graphics::Surface *map = getUiBitmap("16mmap.bmp");
	if (!map)
		return;

	const ButtonControlRecord &button = kMapButtonTable[buttonIndex];
	if (0x1c8 + button.width <= map->w && 0x1e0 + button.height <= map->h)
		_screen->copyRectToSurface(map->getBasePtr(0x1c8, 0x1e0), map->pitch, button.x, button.y, button.width, button.height);

	if (present)
		_screen->update();
}

void BCBEngine::composeMapControlsToSurface(Graphics::Surface &surface, const Graphics::Surface &mapArt) {
	for (uint i = 1; i < ARRAYSIZE(kMapButtonTable); ++i) {
		if (!isMapButtonEnabled(i))
			continue;

		const ButtonControlRecord &button = kMapButtonTable[i];
		if (0x1d8 + button.width <= mapArt.w && 0x1e0 + button.height <= mapArt.h)
			surface.copyRectToSurface(mapArt, button.x, button.y, Common::Rect(0x1d8, 0x1e0, 0x1d8 + button.width, 0x1e0 + button.height));
	}

	const uint32 shadowColor = surface.format.RGBToColor(0, 0, 0);
	const uint32 textColor = surface.format.RGBToColor(255, 255, 255);
	for (uint i = 1; i < ARRAYSIZE(kMapButtonTable); ++i) {
		if (!isMapButtonEnabled(i))
			continue;
		if (i >= _mapButtonTextSjis.size() || _mapButtonTextSjis[i].empty())
			continue;

		const ButtonControlRecord &button = kMapButtonTable[i];
		const int textX = button.x + 0x14;
		const int textY = button.y - (_gameDescription->desc.language == Common::KO_KOR ? 5 : 4);
		const int textWidth = MAX<int>(0, surface.w - textX - 2);
		if (_gameDescription->desc.language == Common::JA_JPN && _sjisFont) {
			int x = textX + 1;
			for (const byte *text = reinterpret_cast<const byte *>(_mapButtonTextSjis[i].c_str()); *text;) {
				const byte firstByte = *text++;
				uint16 sjisChar = firstByte;
				if (((firstByte >= 0x81 && firstByte <= 0x9f) || (firstByte >= 0xe0 && firstByte <= 0xef)) && *text)
					sjisChar |= (uint16)(*text++) << 8;
				_sjisFont->drawChar(surface, sjisChar, x, textY + 1, shadowColor, 0);
				x += _sjisFont->getCharWidth(sjisChar);
			}
			x = textX;
			for (const byte *text = reinterpret_cast<const byte *>(_mapButtonTextSjis[i].c_str()); *text;) {
				const byte firstByte = *text++;
				uint16 sjisChar = firstByte;
				if (((firstByte >= 0x81 && firstByte <= 0x9f) || (firstByte >= 0xe0 && firstByte <= 0xef)) && *text)
					sjisChar |= (uint16)(*text++) << 8;
				_sjisFont->drawChar(surface, sjisChar, x, textY, textColor, 0);
				x += _sjisFont->getCharWidth(sjisChar);
			}
		} else if (_ownerDrawUiTextFont && i < _mapButtonText.size()) {
			_ownerDrawUiTextFont->drawString(&surface, _mapButtonText[i], textX + 1, textY + 1, textWidth, shadowColor, Graphics::kTextAlignLeft);
			_ownerDrawUiTextFont->drawString(&surface, _mapButtonText[i], textX, textY, textWidth, textColor, Graphics::kTextAlignLeft);
		}
	}
}

void BCBEngine::drawMapControls(bool present) {
	const Graphics::Surface *map = getUiBitmap("16mmap.bmp");
	if (!map)
		return;

	Graphics::Surface *screen = _screen->surfacePtr();
	if (screen) {
		composeMapControlsToSurface(*screen, *map);
		_screen->makeAllDirty();
	}

	if (present)
		_screen->update();
}

bool BCBEngine::handleMapControlClick(const Common::Point &mousePosition) {
	if (_currentDisplayState != 2 || !_mapControlsActive)
		return false;

	for (uint i = 1; i < ARRAYSIZE(kMapButtonTable); ++i) {
		if (!isMapButtonEnabled(i))
			continue;

		const ButtonControlRecord &button = kMapButtonTable[i];
		if (!Common::Rect(button.x, button.y, button.x + button.width, button.y + button.height).contains(mousePosition))
			continue;

		drawMapButtonPressed(i);
		stateDword(kStateSelectedSceneChoiceValue) = (button.controlId - 0xc2) & 0xff;
		_mapControlsActive = false;
		stateDword(kStateMovieHistoryCount) = 0;
		stopMapMoveMovie();
		playUiSound();
		showMainSceneScreen();
		processGameStateDispatcher(true);
		return true;
	}

	return false;
}

bool BCBEngine::dispatchMapMoveControls() {
	freeSceneChoiceBacking();
	_sceneChoiceActive = false;
	stopSceneIdleMovie();
	openMapMoveMovie();
	_mapControlsActive = true;
	const bool showNewMapScreen = (_currentDisplayState != 2 && _currentDisplayState != 8);
	if (showNewMapScreen)
		showMapScreen();
	_currentDisplayState = 2;
	writeContinueState();
	if (!showNewMapScreen)
		drawMapControls();
	startMapMoveMovie();
	return true;
}

} // End of namespace BCB
