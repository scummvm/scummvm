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

#include "common/events.h"
#include "graphics/paletteman.h"

#include "pelrock/cdplayer.h"
#include "pelrock/room.h"
#include "pelrock/util.h"

namespace Pelrock {

static const uint32 kCDPlayerTrackNamesOffset = 301203;  // JUEGO.EXE — track name strings
static const uint32 kCDPlayerControlsOffset   = 2214760; // ALFRED.7 — controls sprite

CDPlayer::CDPlayer(PelrockEventManager *eventMan, ResourceManager *res, SoundManager *sound)
	: _events(eventMan), _res(res), _sound(sound) {
	init();
}

CDPlayer::~CDPlayer() {
	cleanup();
}

void CDPlayer::run() {
	g_engine->changeCursor(DEFAULT);

	while (!g_engine->shouldQuit()) {
		_events->pollEvent();
		checkMouse(_events->_mouseX, _events->_mouseY);

		if (_events->_rightMouseClicked) {
			_events->_rightMouseClicked = false;
			break;
		}

		drawScreen();
		g_engine->_screen->markAllDirty();
		g_engine->_screen->update();
		g_system->delayMillis(10);
	}
	g_engine->_screen->clear(0);
	// Restore room palette
	g_system->getPaletteManager()->setPalette(g_engine->_room->_roomPalette, 0, 256);
	_sound->stopMusic();
	_sound->playMusicTrack(g_engine->_room->_musicTrack, true);
}

void CDPlayer::init() {
	_compositeScreen.create(640, 400, Graphics::PixelFormat::createFormatCLUT8());
	loadBackground();
	loadControls();
	loadTrackNames();
}

void CDPlayer::loadTrackNames() {
	Common::File juegoFile;
	if (!juegoFile.open("JUEGO.EXE")) {
		return;
	}
	juegoFile.seek(kCDPlayerTrackNamesOffset, SEEK_SET);

	for (int i = 0; i < 31; i++) {
		trackNames[i] = juegoFile.readString(0, 30);
	}

	juegoFile.close();
}

void CDPlayer::drawScreen() {
	_compositeScreen.blitFrom(_backgroundScreen);
	drawSpriteToBuffer(_compositeScreen, _controls, 1, 1, 213, 72, 207);

	drawButtons();

	g_engine->_screen->blitFrom(_compositeScreen);
	g_engine->_smallFont->drawString(g_engine->_screen, trackNames[_selectedTrack - 2], 26, 17, 640, 255, Graphics::kTextAlignLeft);
}

void CDPlayer::drawButtons() {
	for (int i = 0; i < 5; i++) {
		if (_selectedButton == i) {
			drawSpriteToBuffer(_compositeScreen, buttons[i][1], _buttonRects[i].left, _buttonRects[i].top, _buttonRects[i].width(), _buttonRects[i].height(), 207);
		} else {
			drawSpriteToBuffer(_compositeScreen, buttons[i][0], _buttonRects[i].left, _buttonRects[i].top, _buttonRects[i].width(), _buttonRects[i].height(), 207);
		}
	}
}

void CDPlayer::loadBackground() {
	_backgroundScreen.create(640, 400, Graphics::PixelFormat::createFormatCLUT8());
	_palette = new byte[768];
	_res->getExtraScreen(10, (byte *)_backgroundScreen.getPixels(), _palette);
	g_system->getPaletteManager()->setPalette(_palette, 0, 256);
}

void CDPlayer::cleanup() {
	_backgroundScreen.free();
	_compositeScreen.free();

	if (_palette) {
		delete[] _palette;
		_palette = nullptr;
	}

	for (int i = 0; i < 5; i++) {
		delete[] buttons[i][0];
		delete[] buttons[i][1];
	}

	g_engine->_screen->markAllDirty();
	g_engine->_screen->update();
}

void CDPlayer::checkMouse(int x, int y) {
	if (_events->_leftMouseClicked) {
		switch (_selectedButton) {
		case STOP_BUTTON:
			_sound->stopMusic();
			break;
		case PAUSE_BUTTON:
			_sound->pauseMusic();
			break;
		case PLAY_BUTTON:
			if (_sound->isPaused() && _sound->getCurrentMusicTrack() == _selectedTrack) {
				_sound->playMusicTrack(_selectedTrack, true);
			} else {
				_sound->stopMusic();
				_sound->playMusicTrack(_selectedTrack, true);
			}
			break;
		case PREVIOUS_BUTTON:
			if (_selectedTrack > 2) {
				_selectedTrack--;
			}
			break;
		case NEXT_BUTTON:
			if (_selectedTrack < 32) {
				_selectedTrack++;
			}
			break;
		default:
			break;
		}
		_selectedButton = NO_CDBUTTON;
		_events->_leftMouseClicked = false;
	}

	if (_events->_leftMouseButton != 0 && _selectedButton == NO_CDBUTTON) {
		_selectedButton = isButtonClicked(_events->_mouseX, _events->_mouseY);
		if (_selectedButton != NO_CDBUTTON) {
			_sound->playSound("11ZZZZZZ.SMP", 0);
		}
	}
}

CDPlayer::CDControls CDPlayer::isButtonClicked(int x, int y) {
	for (int i = 0; i < 5; i++) {
		if (_buttonRects[i].contains(x, y)) {
			return static_cast<CDControls>(i);
		}
	}
	return NO_CDBUTTON;
}

void CDPlayer::loadControls() {
	_controls = new byte[213 * 72];
	Common::File alfred7;
	if (!alfred7.open("ALFRED.7")) {
		return;
	}
	alfred7.seek(kCDPlayerControlsOffset, SEEK_SET);
	byte *compressedData = nullptr;
	size_t outSize = 0;
	readUntilBuda(&alfred7, kCDPlayerControlsOffset, compressedData, outSize);
	byte *rawData = nullptr;

	rleDecompress(compressedData, outSize, 0, 0, &rawData, true);

	uint32 pos = 213 * 72;
	Common::copy(rawData, rawData + pos, _controls);
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 2; j++) {
			int w = _buttonRects[i].width();
			int h = _buttonRects[i].height();
			buttons[i][j] = new byte[w * h];
			Common::copy(rawData + pos, rawData + pos + w * h, buttons[i][j]);
			pos += w * h;
		}
	}
	alfred7.close();
	delete[] compressedData;
	delete[] rawData;
}

} // End of namespace Pelrock
