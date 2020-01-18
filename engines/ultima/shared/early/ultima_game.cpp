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

#include "ultima/shared/early/ultima_game.h"
#include "ultima/shared/core/game_state.h"
#include "ultima/shared/gfx/game_view.h"
#include "ultima/shared/engine/resources.h"
#include "ultima/shared/early/ultima_early.h"
#include "ultima/shared/early/font_resources.h"

namespace Ultima {
namespace Shared {

EMPTY_MESSAGE_MAP(UltimaGame, Game);

UltimaGame::UltimaGame() : Game() {
	_fontResources = new FontResources();
	_gameState = new GameState();
	_gameView = new GameView();
	_gameView->addUnder(this);

	setPalette();
}

UltimaGame::~UltimaGame() {
	delete _fontResources;
	delete _gameState;
	delete _gameView;
}

void UltimaGame::setPalette() {
	switch (_gameState->_videoMode) {
	case CGA: {
		static const byte PALETTE[4][3] = { { 0, 0, 0 }, { 0xAA, 0xAA, 0 }, {0xAA, 0, 0xAA }, {0xAA, 0xAA, 0xAA } };
		g_vm->_screen->setPalette(&PALETTE[0][0], 0, 4);
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
		break;
	}

	default:
		break;
	}
}

} // End of namespace Shared
} // End of namespace Ultima
