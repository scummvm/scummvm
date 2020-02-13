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
#include "ultima/shared/early/font_resources.h"
#include "ultima/shared/maps/map.h"
#include "ultima/shared/early/ultima_early.h"
#include "ultima/shared/gfx/font.h"
#include "ultima/shared/gfx/screen.h"

namespace Ultima {
namespace Shared {

BEGIN_MESSAGE_MAP(Game, GameBase)
	ON_MESSAGE(EndOfTurnMsg)
END_MESSAGE_MAP()

Game::Game() : GameBase(), _randomSeed(0), _gameView(nullptr), _map(nullptr), _party(nullptr),
		_edgeColor(0), _borderColor(0), _highlightColor(0), _textColor(0), _color1(0), _bgColor(0), _whiteColor(0) {
	_fontResources = new FontResources();
	_fontResources->load();
	setFont(new Gfx::Font((const byte *)&_fontResources->_font8x8[0][0]));
}

Game::~Game() {
	delete _fontResources;
}

void Game::setCGAPalette() {
	static const byte PALETTE[4][3] = { { 0, 0, 0 },{ 0xAA, 0xAA, 0 },{ 0xAA, 0, 0xAA },{ 0xAA, 0xAA, 0xAA } };
	g_vm->_screen->setPalette(&PALETTE[0][0], 0, 4);

	_edgeColor = 3;
	_borderColor = 3;
	_highlightColor = 1;
	_textColor = 3;
	_color1 = 6;
	_whiteColor = 3;
}

void Game::setEGAPalette() {
	static const byte PALETTE[16][3] = {
		{ 0, 0, 0 },{ 0x00, 0x00, 0x80 },{ 0x00, 0x80, 0x00 },{ 0x00, 0x80, 0x80 },
		{ 0x80, 0x00, 0x00 },{ 0x80, 0x00, 0x80 },{ 0x80, 0x80, 0x00 },{ 0xC0, 0xC0, 0xC0 },
		{ 0x80, 0x80, 0x80 },{ 0x00, 0x00, 0xFF },{ 0x00, 0xFF, 0x00 },{ 0x00, 0xFF, 0xFF },
		{ 0xFF, 0x40, 0x40 },{ 0xFF, 0x00, 0xFF },{ 0xFF, 0xFF, 0x00 },{ 0xFF, 0xFF, 0xFF }
	};
	g_vm->_screen->setPalette(&PALETTE[0][0], 0, 16);

	_edgeColor = 15;
	_borderColor = 1;
	_highlightColor = 12;
	_textColor = 11;
	_color1 = 7;
	_bgColor = 0;
	_whiteColor = 15;
}

void Game::setEGAPalette(const byte *palette) {
	// Build up the EGA palette
	byte egaPalette[64 * 3];

	byte *p = &egaPalette[0];
	for (int i = 0; i < 64; ++i) {
		*p++ = (i >> 2 & 1) * 0xaa + (i >> 5 & 1) * 0x55;
		*p++ = (i >> 1 & 1) * 0xaa + (i >> 4 & 1) * 0x55;
		*p++ = (i & 1) * 0xaa + (i >> 3 & 1) * 0x55;
	}

	// Loop through setting palette colors based on the passed indexes
	for (int idx = 0; idx < 16; ++idx) {
		int palIndex = palette[idx];
		assert(palIndex < 64);

		const byte *pRgb = (const byte *)&egaPalette[palIndex * 3];
		g_vm->_screen->setPalette(pRgb, idx, 1);
	}
}

void Game::loadU6Palette() {
	// Read in the palette
	File f("u6pal");
	byte palette[PALETTE_SIZE];
	f.read(palette, PALETTE_SIZE);
	f.close();

	// Adjust the palette values from 0-63 to 0-255, and set the palette
	for (int idx = 0; idx < PALETTE_SIZE; ++idx)
		palette[idx] = VGA_COLOR_TRANS(palette[idx]);
	g_vm->_screen->setPalette(&palette[0], 0, PALETTE_COUNT);

	// TODO: Set appropriate indexes
	_edgeColor = 15;
	_borderColor = 1;
	_highlightColor = 12;
	_textColor = 72;
	_color1 = 7;
	_bgColor = 49;
}

void Game::playFX(uint effectId) {
	warning("TODO: playFX");
}

void Game::synchronize(Common::Serializer &s) {
	_party->synchronize(s);
	_map->synchronize(s);
}

bool Game::EndOfTurnMsg(CEndOfTurnMsg &msg) {
	// Update things on the map
	_map->update();
	return false;
}

void Game::endOfTurn() {
	CEndOfTurnMsg turnMsg;
	turnMsg.execute(this);
}

} // End of namespace Shared
} // End of namespace Ultima
