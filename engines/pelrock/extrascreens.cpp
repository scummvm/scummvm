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

#include "extrascreens.h"
#include "pelrock/extrascreens.h"
#include "pelrock/graphics.h"
#include "pelrock/util.h"

namespace Pelrock {

SpellBook::SpellBook(PelrockEventManager *eventMan, ResourceManager *res)
	: _palette(nullptr),
	  _events(eventMan),
	  _res(res),
	  _spell(nullptr) {
	init();
}

SpellBook::~SpellBook() {
	cleanup();
}

Spell *SpellBook::run() {
	loadBackground();
	g_engine->changeCursor(DEFAULT);
	bool exit = false;
	while (!g_engine->shouldQuit() && !exit) {
		_events->pollEvent();
		drawScreen();
		if (_events->_leftMouseClicked) {
			_events->_leftMouseClicked = false;
			exit = checkMouse(_events->_mouseClickX, _events->_mouseClickY);
		}
		g_engine->_screen->markAllDirty();
		g_engine->_screen->update();
		g_system->delayMillis(10);
	}
	g_engine->_screen->clear(0);
	// Restore room palette
	g_system->getPaletteManager()->setPalette(g_engine->_room->_roomPalette, 0, 256);
	return _selectedSpell;
}

void SpellBook::init() {
	_compositeScreen.create(640, 400, Graphics::PixelFormat::createFormatCLUT8());

	// selectPage(0);
}

void SpellBook::selectPage(int page) {
	debug("Selected spell page: %d", page);
	_spell = new Spell();
	_spell->page = page;
	Common::File alfred7;
	if (!alfred7.open("ALFRED.7")) {
		return;
	}

	Common::File juegoFile;
	if (!juegoFile.open("JUEGO.EXE")) {
		return;
	}

	alfred7.seek(1268719, SEEK_SET);
	int w = 119;
	int h = 99;
	int nFrames = 13;
	byte *compressedData = nullptr;
	byte *spriteData = nullptr;
	size_t outSize = 0;
	readUntilBuda(&alfred7, 1268723, compressedData, outSize);
	rleDecompress(compressedData, outSize, 0, w * h * nFrames, &spriteData, false);
	_spell->image = new byte[w * h];
	extractSingleFrame(spriteData, _spell->image, page, w, h);

	juegoFile.seek(0x0004661C, SEEK_SET);
	byte *textData = new byte[2861];
	juegoFile.read(textData, 2861);

	for (int i = 0; i < 2861; ++i) {
		if (textData[i] == 0x0D)
			textData[i] = 23;
	}

	Common::Array<Common::StringArray> spells = _res->processTextData(textData, 2861, true);

	_spell->text = spells[page];
	delete[] compressedData;
	delete[] spriteData;
	alfred7.close();
	juegoFile.close();
}

void SpellBook::drawScreen() {
	_compositeScreen.blitFrom(_backgroundScreen);

	int textY = 83;
	int textX = 317;

	if (_spell != nullptr) {
		drawSpriteToBuffer(_compositeScreen, _spell->image, 168, 143, 119, 99, 207);
		g_engine->_graphics->drawColoredTexts(_compositeScreen, _spell->text, textX, textY, 640, 0, g_engine->_smallFont);
	}

	g_engine->_screen->blitFrom(_compositeScreen);
	if (_spell != nullptr) {
		g_engine->_graphics->drawColoredTexts(g_engine->_screen, _spell->text, textX, textY, 640, 0, g_engine->_smallFont);
	}
}

void SpellBook::loadBackground() {
	_backgroundScreen.create(640, 400, Graphics::PixelFormat::createFormatCLUT8());
	_palette = new byte[768];
	_res->getExtraScreen(8, (byte *)_backgroundScreen.getPixels(), _palette);
	g_system->getPaletteManager()->setPalette(_palette, 0, 256);
}

void SpellBook::cleanup() {
	_backgroundScreen.free();
	_compositeScreen.free();
	if (_palette) {
		delete[] _palette;
		_palette = nullptr;
	}
	if (_spell) {
		delete _spell;
		_spell = nullptr;
	}
	g_engine->_screen->markAllDirty();
	g_engine->_screen->update();
}

bool SpellBook::checkMouse(int x, int y) {
	// Check bookmarks
	for (int i = 0; i < 13; i++) {
		Common::Rect r = Common::Rect(_bookmarks[i].x, _bookmarks[i].y, _bookmarks[i].x + _bookmarks[i].w, _bookmarks[i].y + _bookmarks[i].h);
		if (r.contains(x, y)) {
			selectPage(_bookmarks[i].page);
			return false;
		}
	}

	// Check text area
	if (_spell == nullptr) {
		return true;
	}

	Common::Rect textArea = Common::Rect(321, 81, 321 + 140, 81 + (_spell->text.size() * 10));
	if (textArea.contains(x, y)) {
		_selectedSpell = _spell;
		return true;
	}
	return false;
}

CDPlayer::CDPlayer(PelrockEventManager *eventMan, ResourceManager *res, SoundManager *sound) : _events(eventMan), _res(res), _sound(sound) {
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
	juegoFile.seek(0x049893, SEEK_SET);

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
			if(_sound->_isPaused && _sound->_currentMusicTrack == _selectedTrack) {
				_sound->playMusicTrack(_selectedTrack, true);
			}
			else {
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
		if(_selectedButton != NO_CDBUTTON) {
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
	alfred7.seek(2214760, SEEK_SET);
	byte *compressedData = nullptr;
	size_t outSize = 0;
	readUntilBuda(&alfred7, 2214760, compressedData, outSize);
	byte *rawData = nullptr;

	rleDecompress(compressedData, outSize, 0, 0, &rawData, true);

	// debug("Decompressed CD player controls: %d bytes", decompressedSize);
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

BackgroundBook::BackgroundBook(PelrockEventManager *eventMan, ResourceManager *res) : _events(eventMan), _res(res) {
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
	// Restore room palette
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
			if ((_selectedPage + 1) * kItemsPerPage < _roomNames.size()) {
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
			if (firstItem + itemIndex < _roomNames.size()) {
				Common::String roomName = _roomNames[firstItem + itemIndex];
				debug("Selected room: %s", roomName.c_str());
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

	size_t namesSize = 1335;
	juegoExe.seek(0x49315, SEEK_SET);
	byte *namesData = new byte[namesSize];
	juegoExe.read(namesData, namesSize);
	uint32 pos = 0;
	Common::String currentName = "";
	while (pos < namesSize) {
		if (namesData[pos] == 0xFD &&
			namesData[pos + 1] == 0x00 &&
			namesData[pos + 2] == 0x08 &&
			namesData[pos + 3] == 0x02) {
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
	g_engine->_screen->blitFrom(_compositeScreen);


	int firstItem = _selectedPage * kItemsPerPage;
	for(int i = 0; i < kItemsPerPage; i++) {
		if(firstItem + i >= _roomNames.size()) {
			break;
		}
		// g_engine->_graphics->drawColoredTexts(_compositeScreen, _roomNames[i], 37, 72 + (i * g_engine->_smallFont->getFontHeight()), 640, 0, Graphics::kTextAlignLeft);
	// g_engine->_graphics->drawColoredTexts(_compositeScreen, _spell->text, textX, textY, 640, 0, g_engine->_smallFont);
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
	alfred7.seek(3188448, SEEK_SET);
	for(int i = 0; i < 2; i++) {
		for(int j = 0; j < 2; j++) {
			int w = _buttonRects[i].width();
			int h = _buttonRects[i].height();
			_buttons[i][j] = new byte[w * h];
			alfred7.read(_buttons[i][j], w * h);
		}
	}

	alfred7.close();
}

void BackgroundBook::cleanup() {
	_compositeScreen.free();
	_backgroundScreen.free();
	if (_palette) {
		delete[] _palette;
		_palette = nullptr;
	}
}

} // End of namespace Pelrock
