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

#include "glk/comprehend/comprehend.h"
#include "glk/comprehend/debugger.h"
#include "glk/comprehend/draw_surface.h"
#include "glk/comprehend/game.h"
#include "glk/comprehend/game_cc.h"
#include "glk/comprehend/game_data.h"
#include "glk/comprehend/game_oo.h"
#include "glk/comprehend/game_tm.h"
#include "glk/comprehend/game_tr.h"
#include "glk/comprehend/pics.h"
#include "glk/quetzal.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "engines/util.h"

namespace Glk {
namespace Comprehend {

// Even with no ScummVM scaling, internally we do a 2x scaling to
// render on a 640x480 window, to allow for better looking text
#define SCALE_FACTOR 2

Comprehend *g_comprehend;

Comprehend::Comprehend(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc), _topWindow(nullptr), _bottomWindow(nullptr),
	_drawSurface(nullptr), _game(nullptr), _pics(nullptr), _saveSlot(-1),
	_graphicsEnabled(true), _drawFlags(0), _disableSaves(false) {
	g_comprehend = this;
}

Comprehend::~Comprehend() {
	delete _drawSurface;
	delete _game;
	SearchMan.remove("Pics");   // This also deletes it

	g_comprehend = nullptr;
}

void Comprehend::initGraphicsMode() {
	Graphics::PixelFormat pixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
	initGraphics(640, 400, &pixelFormat);
}

void Comprehend::runGame() {
	initialize();

	// Lookup game
	createGame();

	_game->loadGame();
	_game->playGame();

	deinitialize();
}

void Comprehend::initialize() {
	// Set up the GLK windows
	g_conf->_wMarginX = 0;
	g_conf->_wMarginY = 0;
	g_conf->_tMarginY = 4;

	_bottomWindow = (TextBufferWindow *)glk_window_open(0, 0, 0, wintype_TextBuffer, 1);
	glk_set_window(_bottomWindow);

	showGraphics();
	_topWindow->fillRect(0, Rect(0, 0, _topWindow->_w, _topWindow->_h));

	const Graphics::PixelFormat pixelFormat = g_system->getScreenFormat();
	_bottomWindow->_stream->setZColors(
		pixelFormat.RGBToColor(0xff, 0xff, 0xff),
		pixelFormat.RGBToColor(0, 0, 0)
	);

	// Initialize drawing surface, and the archive that abstracts
	// the room and item graphics as as individual files
	_drawSurface = new DrawSurface();
	_pics = new Pics();
	SearchMan.add("Pics", _pics, 99, true);

	// Check for savegame to load
	_saveSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;
}

void Comprehend::deinitialize() {
	glk_window_close(_topWindow);
	glk_window_close(_bottomWindow);
}

void Comprehend::createDebugger() {
	setDebugger(new Debugger());
}

void Comprehend::createGame() {
	if (_gameDescription._gameId == "crimsoncrown")
		_game = new CrimsonCrownGame();
	else if (_gameDescription._gameId == "ootopis")
		_game = new OOToposGame();
	else if (_gameDescription._gameId == "talisman")
		_game = new OOToposGame();
	else if (_gameDescription._gameId == "transylvania")
		_game = new TransylvaniaGame();
	else
		error("Unknown game");
}

void Comprehend::print(const char *fmt, ...) {
	va_list argp;
	va_start(argp, fmt);
	Common::String msg = Common::String::vformat(fmt, argp);
	va_end(argp);

	glk_put_string_stream(glk_window_get_stream(_bottomWindow),
	                      msg.c_str());
}

void Comprehend::readLine(char *buffer, size_t maxLen) {
	event_t ev;

	glk_request_line_event(_bottomWindow, buffer, maxLen - 1, 0);

	for (;;) {
		glk_select(&ev);
		if (ev.type == evtype_Quit) {
			glk_cancel_line_event(_bottomWindow, &ev);
			return;
		} else if (ev.type == evtype_LineInput)
			break;
	}

	buffer[ev.val1] = 0;
}

int Comprehend::readChar() {
	glk_request_char_event(_bottomWindow);
	setDisableSaves(true);

	event_t ev;
	while (ev.type != evtype_CharInput) {
		glk_select(&ev);

		if (ev.type == evtype_Quit) {
			glk_cancel_char_event(_bottomWindow);
			return -1;
		}
	}

	setDisableSaves(false);
	return ev.val1;
}

Common::Error Comprehend::readSaveData(Common::SeekableReadStream *rs) {
	Common::Serializer s(rs, nullptr);
	_game->synchronizeSave(s);

	_game->_updateFlags = UPDATE_ALL;
	return Common::kNoError;
}

Common::Error Comprehend::writeGameData(Common::WriteStream *ws) {
	Common::Serializer s(nullptr, ws);
	_game->synchronizeSave(s);

	return Common::kNoError;
}

bool Comprehend::loadLauncherSavegameIfNeeded() {
	if (_saveSlot != -1) {
		return loadGameState(_saveSlot).getCode() == Common::kNoError;
	}

	return false;
}


void Comprehend::drawPicture(uint pictureNum) {
	if (_topWindow)
		glk_image_draw_scaled(_topWindow, pictureNum,
			20 * SCALE_FACTOR, 0, G_RENDER_WIDTH * SCALE_FACTOR, G_RENDER_HEIGHT * SCALE_FACTOR);
}

void Comprehend::drawLocationPicture(int pictureNum, bool clearBg) {
	drawPicture(pictureNum + (clearBg ? LOCATIONS_OFFSET : LOCATIONS_NO_BG_OFFSET));
}

void Comprehend::drawItemPicture(int pictureNum) {
	drawPicture(pictureNum + ITEMS_OFFSET);
}

void Comprehend::clearScreen(bool isBright) {
	drawPicture(isBright ? BRIGHT_ROOM : DARK_ROOM);
}

void Comprehend::toggleGraphics() {
	if (_topWindow) {
		// Remove the picture window
		glk_window_close(_topWindow);
		_topWindow = nullptr;
		_graphicsEnabled = false;
	} else {
		// Create the window again
		showGraphics();
	}
}

void Comprehend::showGraphics() {
	if (!_topWindow) {
		_topWindow = (GraphicsWindow *)glk_window_open(_bottomWindow,
			winmethod_Above | winmethod_Fixed,
			160 * SCALE_FACTOR, wintype_Graphics, 2);
		_graphicsEnabled = true;
	}
}

} // namespace Comprehend
} // namespace Glk
