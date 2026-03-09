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

#include "pelrock/backgroundbook.h"
#include "pelrock/room.h"
#include "pelrock/util.h"

namespace Pelrock {

static const uint32 kBgBookButtonsOffset = 3188448; // ALFRED.7 — UI buttons
static const uint32 kRoomNamesOffset     = 299797;  // JUEGO.EXE — room name strings
static const uint32 kRoomNamesSize       = 1297;

BackgroundBook::BackgroundBook(PelrockEventManager *eventMan, ResourceManager *res, RoomManager *room)
	: _events(eventMan), _res(res), _room(room) {
	init();
}

BackgroundBook::~BackgroundBook() {
	cleanup();
}

void BackgroundBook::run() {
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
	g_system->getPaletteManager()->setPalette(g_engine->_room->_roomPalette, 0, 256);
}

void BackgroundBook::init() {
	_compositeScreen.create(640, 400, Graphics::PixelFormat::createFormatCLUT8());
	loadBackground();
	loadButtons();
	loadRoomNames();
}

void BackgroundBook::checkMouse(int x, int y) {
	if (_events->_leftMouseClicked) {
		switch (_selectedButton) {
		case PREVIOUS_BUTTON:
			if (_selectedPage > 0) {
				_selectedPage--;
			}
			break;
		case NEXT_BUTTON:
			if ((_selectedPage + 1) * kItemsPerPage < (int)_roomNames.size()) {
				_selectedPage++;
			}
			break;
		default:
			break;
		}
		_selectedButton = NO_BG_BUTTON;

		int firstItem = _selectedPage * kItemsPerPage;
		if (y >= 72 && y < 72 + (kItemsPerPage * g_engine->_smallFont->getFontHeight()) && x >= 37 && x <= 37 + 200) {
			int itemIndex = (y - 72) / g_engine->_smallFont->getFontHeight();
			int roomIndex = firstItem + itemIndex;
			if (roomIndex < (int)_roomNames.size()) {
				_events->_leftMouseClicked = false;
				int finalRoomIndex = roomIndex < 10 ? roomIndex : roomIndex + 2;
				showRoom(finalRoomIndex);
			}
		}

		_events->_leftMouseClicked = false;
	}

	if (_events->_leftMouseButton != 0 && _selectedButton == NO_BG_BUTTON) {
		_selectedButton = isButtonClicked(_events->_mouseX, _events->_mouseY);
	}
}

BackgroundBook::Buttons BackgroundBook::isButtonClicked(int x, int y) {
	for (int i = 0; i < 2; i++) {
		if (_buttonRects[i].contains(x, y)) {
			return static_cast<Buttons>(i);
		}
	}
	return NO_BG_BUTTON;
}

void BackgroundBook::loadRoomNames() {
	Common::StringArray roomNames;
	Common::File juegoExe;
	if (!juegoExe.open(Common::Path("JUEGO.EXE"))) {
		error("Couldnt find file JUEGO.EXE");
	}

	size_t namesSize = kRoomNamesSize;
	juegoExe.seek(kRoomNamesOffset, SEEK_SET);
	byte *namesData = new byte[namesSize];
	juegoExe.read(namesData, namesSize);
	uint32 pos = 0;
	Common::String currentName = "";
	while (pos < namesSize) {
		if (namesData[pos] == 0xFD) {
			if (currentName.size() > 0) {
				roomNames.push_back(currentName);
			}
			currentName = "";
			pos += 4;
			continue;
		}
		currentName += (char)namesData[pos];
		pos++;
	}
	delete[] namesData;
	juegoExe.close();
	_roomNames = roomNames;
}

void BackgroundBook::drawScreen() {
	_compositeScreen.blitFrom(_backgroundScreen);
	drawButtons();

	if (thumbSurface) {
		_compositeScreen.blitFrom(*thumbSurface, Common::Point(338, 120));
	}
	g_engine->_screen->blitFrom(_compositeScreen);

	int firstItem = _selectedPage * kItemsPerPage;
	for (int i = 0; i < kItemsPerPage; i++) {
		if (firstItem + i >= (int)_roomNames.size()) {
			break;
		}
		g_engine->_smallFont->drawString(g_engine->_screen, _roomNames[firstItem + i], 37, 72 + (i * g_engine->_smallFont->getFontHeight()), 640, 2, Graphics::kTextAlignLeft);
	}
}

void BackgroundBook::drawButtons() {
	for (int i = 0; i < 2; i++) {
		if (_selectedButton == i) {
			drawSpriteToBuffer(_compositeScreen, _buttons[i][0], _buttonRects[i].left, _buttonRects[i].top, _buttonRects[i].width(), _buttonRects[i].height(), 207);
		} else {
			drawSpriteToBuffer(_compositeScreen, _buttons[i][1], _buttonRects[i].left, _buttonRects[i].top, _buttonRects[i].width(), _buttonRects[i].height(), 207);
		}
	}
}

void BackgroundBook::loadBackground() {
	_backgroundScreen.create(640, 400, Graphics::PixelFormat::createFormatCLUT8());
	_palette = new byte[768];
	_res->getExtraScreen(13, (byte *)_backgroundScreen.getPixels(), _palette);
	g_system->getPaletteManager()->setPalette(_palette, 0, 256);
}

void BackgroundBook::loadButtons() {
	Common::File alfred7;
	if (!alfred7.open("ALFRED.7")) {
		return;
	}
	alfred7.seek(kBgBookButtonsOffset, SEEK_SET);
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			int w = _buttonRects[i].width();
			int h = _buttonRects[i].height();
			_buttons[i][j] = new byte[w * h];
			alfred7.read(_buttons[i][j], w * h);
		}
	}
	alfred7.close();
}

void BackgroundBook::showRoom(int roomIndex) {
	Common::File roomFile;
	if (!roomFile.open(Common::Path("ALFRED.1"))) {
		warning("BackgroundBook: Could not open ALFRED.1");
		return;
	}

	int roomOffset = roomIndex * kRoomStructSize;
	Graphics::ManagedSurface bgSurface(640, 400, Graphics::PixelFormat::createFormatCLUT8());

	byte *roomPalette = new byte[256 * 3];
	_room->getPalette(&roomFile, roomOffset, roomPalette);
	_room->getBackground(&roomFile, roomOffset, (byte *)bgSurface.getPixels());
	roomFile.close();

	thumbSurface = bgSurface.scale(160, 100);
	// Set room palette and display the background
	g_system->getPaletteManager()->setPalette(roomPalette, 0, 256);

	bgSurface.free();
}

void BackgroundBook::cleanup() {
	_compositeScreen.free();
	_backgroundScreen.free();
	if (_palette) {
		delete[] _palette;
		_palette = nullptr;
	}
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			delete[] _buttons[i][j];
		}
	}
	if (thumbSurface) {
		thumbSurface->free();
		thumbSurface = nullptr;
	}
}

} // End of namespace Pelrock
