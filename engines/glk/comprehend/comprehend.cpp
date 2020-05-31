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
#include "common/config-manager.h"
#include "common/translation.h"
#include "glk/comprehend/debugger.h"
#include "glk/comprehend/draw_surface.h"
#include "glk/comprehend/game.h"
#include "glk/comprehend/game_cc.h"
#include "glk/comprehend/game_data.h"
#include "glk/comprehend/game_oo.h"
#include "glk/comprehend/game_tm.h"
#include "glk/comprehend/game_tr.h"
#include "glk/quetzal.h"

namespace Glk {
namespace Comprehend {

Comprehend *g_comprehend;

Comprehend::Comprehend(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc), _topWindow(nullptr), _bottomWindow(nullptr),
                                                                            _drawSurface(nullptr), _game(nullptr), _saveSlot(-1),
                                                                            _graphicsEnabled(true), _drawFlags(0) {
	g_comprehend = this;
}

Comprehend::~Comprehend() {
	delete _drawSurface;
	delete _game;

	g_comprehend = nullptr;
}

void Comprehend::runGame() {
	initialize();

	// Lookup game
	createGame();

	comprehend_load_game(_game);
	comprehend_play_game(_game);

	deinitialize();
}
void Comprehend::initialize() {
	// Set up the GLK windows
	g_conf->_wMarginX = 0;
	g_conf->_wMarginY = 0;

	_bottomWindow = (TextBufferWindow *)glk_window_open(0, 0, 0, wintype_TextBuffer, 1);
	_topWindow = (GraphicsWindow *)glk_window_open(_bottomWindow,
	                                               winmethod_Above | winmethod_Fixed,
	                                               400, wintype_Graphics, 2);

	glk_set_window(_bottomWindow);
	_topWindow->fillRect(0, Rect(0, 0, _topWindow->_w, _topWindow->_h));

	// Initialize drawing
	_drawSurface = new DrawSurface();
	_drawSurface->setColorTable(0);
	_drawSurface->_renderColor = 0;
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
	if (_gameDescription._gameId == "ootopis")
		_game = new OOToposGame();
	if (_gameDescription._gameId == "talisman")
		_game = new OOToposGame();
	if (_gameDescription._gameId == "transylvania")
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

	_drawSurface->renderIfDirty();
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
	_drawSurface->renderIfDirty();

	glk_request_char_event(_bottomWindow);

	event_t ev;
	while (ev.type != evtype_CharInput) {
		glk_select(&ev);

		if (ev.type == evtype_Quit) {
			glk_cancel_char_event(_bottomWindow);
			return -1;
		}
	}

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

} // namespace Comprehend
} // namespace Glk
