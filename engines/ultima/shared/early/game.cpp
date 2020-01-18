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

#include "ultima/shared/early/game.h"
#include "ultima/shared/core/game_state.h"
#include "ultima/shared/engine/resources.h"
#include "ultima/shared/early/font_resources.h"
#include "ultima/shared/early/ultima_early.h"
#include "ultima/shared/gfx/font.h"
#include "ultima/shared/gfx/screen.h"

namespace Ultima {
namespace Shared {

EMPTY_MESSAGE_MAP(Game, GameBase);

Game::Game() : GameBase() {
	_gameState = new GameState();
	_fontResources = new FontResources();
	_fontResources->load();
	setFont(new Gfx::Font((const byte *)&_fontResources->_font8x8[0][0]));

	setPalette();
}

Game::~Game() {
	delete _fontResources;
	delete _gameState;
}

void Game::setPalette() {
	switch (_gameState->_videoMode) {
	case CGA: {
		static const byte PALETTE[4][3] = { { 0, 0, 0 }, { 0xAA, 0xAA, 0 }, {0xAA, 0, 0xAA }, {0xAA, 0xAA, 0xAA } };
		g_vm->_screen->setPalette(&PALETTE[0][0], 0, 4);

		_edgeColor = 3;
		_borderColor = 3;
		_highlightColor = 1;
		_textColor = 3;
		_color1 = 6;
		break;
	}

	case TGA:
	case EGA: {
		static const byte PALETTE[16][3] = {
			{ 0, 0, 0 }, { 0x00, 0x00, 0x80 }, { 0x00, 0x80, 0x00 }, { 0x00, 0x80, 0x80 },
			{ 0x80, 0x00, 0x00 }, { 0x80, 0x00, 0x80 }, { 0x80, 0x80, 0x00 }, { 0xC0, 0xC0, 0xC0 },
			{ 0x80, 0x80, 0x80 }, { 0x00, 0x00, 0xFF }, { 0x00, 0xFF, 0x00 }, { 0x00, 0xFF, 0xFF },
			{ 0xFF, 0x40, 0x40 }, { 0xFF, 0x00, 0xFF }, { 0xFF, 0xFF, 0x00 }, {0xFF, 0xFF, 0xFF }
		};
		g_vm->_screen->setPalette(&PALETTE[0][0], 0, 16);

		_edgeColor = 15;
		_borderColor = 1;
		_highlightColor = 12;
		_textColor = 11;
		_color1 = 7;
		break;
	}

	default:
		error("TODO: Palette setup for other video mode");
		break;
	}

}

} // End of namespace Shared
} // End of namespace Ultima
