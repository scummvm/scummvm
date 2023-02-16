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

#include "mm/mm1/views_enh/game_party.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

static const byte CONDITION_COLORS[17] = {
	9, 9, 9, 9, 9, 9, 9, 9, 32, 32, 32, 32, 6, 6, 6, 6, 15
};

const byte FACE_CONDITION_FRAMES[17] = {
	2, 2, 2, 1, 1, 4, 4, 4, 3, 2, 4, 3, 3, 5, 6, 7, 0
};

const byte CHAR_FACES_X[6] = { 10, 45, 81, 117, 153, 189 };

const byte HP_BARS_X[6] = { 13, 50, 86, 122, 158, 194 };

GameParty::GameParty(UIElement *owner) : TextView("GameParty", owner),
		_restoreSprites("restorex.icn"),
		_hpSprites("hpbars.icn"),
		_dseFace("dse.fac") {
	setBounds(Common::Rect(0, 144, 234, 200));
}

void GameParty::draw() {
	Graphics::ManagedSurface s = getSurface();
	_restoreSprites.draw(&s, 0, Common::Point(8, 5));

	// Handle drawing the party faces
	bool inCombat = g_events->isInCombat();

	// Draw character frames
	for (uint idx = 0; idx < g_globals->_party.size(); ++idx) {
		Character &c = inCombat ? *g_globals->_combatParty[idx] : g_globals->_party[idx];
		ConditionEnum charCondition = c.worstCondition();
		int charFrame = FACE_CONDITION_FRAMES[charCondition];

		Shared::Xeen::SpriteResource *sprites = (charFrame > 4) ? &_dseFace : &c._faceSprites;
		assert(sprites);
		if (charFrame > 4)
			charFrame -= 5;

		sprites->draw(&s, charFrame, Common::Point(CHAR_FACES_X[idx], 6));
	}

	for (uint idx = 0; idx < g_globals->_party.size(); ++idx) {
		const Character &c = inCombat ? *g_globals->_combatParty[idx] : g_globals->_party[idx];

		// Draw the Hp bar
		int maxHp = c._hpMax;
		int frame;
		if (c._hpCurrent < 1)
			frame = 4;
		else if (c._hpCurrent > maxHp)
			frame = 3;
		else if (c._hpCurrent == maxHp)
			frame = 0;
		else if (c._hpCurrent < (maxHp / 4))
			frame = 2;
		else
			frame = 1;

		_hpSprites.draw(&s, frame, Common::Point(HP_BARS_X[idx], 38));

		// Also draw the highlight if character is selected
		if (_highlightOn && g_globals->_currCharacter == &c)
			g_globals->_globalSprites.draw(&s, 8, Common::Point(CHAR_FACES_X[idx] - 1, 5));
	}

	// Sprite drawing doesn't automatically mark the drawn areas,
	// so manually flag the entire area as dirty
	s.markAllDirty();
}

bool GameParty::msgGame(const GameMessage &msg) {
	if (msg._name == "CHAR_HIGHLIGHT") {
		_highlightOn = msg._value != 0;
		draw();
		return true;
	}

	return false;
}


} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
