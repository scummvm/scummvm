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

#include "mm/mm1/views_enh/protect.h"
#include "mm/mm1/data/text.h"
#include "mm/mm1/game/combat.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

Protect::Protect() : ScrollView("Protect") {
	setBounds(Common::Rect(0, 0, 320, 200));
	addButton(&g_globals->_escSprites, Common::Point(134, 170), 0, KEYBIND_ESCAPE, true);
}

void Protect::draw() {
	ScrollView::draw();

	setReduced(false);
	writeLine(0, "Protect:  Spells currently active", ALIGN_MIDDLE);
	writeString(152, 172, "Go back");

	setReduced(true);
	int yp = 4;

	// Protection spells
	for (int i = 0; i < 7; ++i) {
		if (g_globals->_activeSpells._arr[i]) {
			Common::String line = Common::String::format("Protection from %s + %d%%",
				PROTECTION_NAMES[i],
				g_globals->_activeSpells._arr[i]);
			writeString(0, yp += 8, line, ALIGN_MIDDLE);
		}
	}

	// Light spell
	if (g_globals->_activeSpells._arr[7]) {
		writeString(0, yp += 8, Common::String::format("Light (%d)",
			g_globals->_activeSpells._arr[7]), ALIGN_MIDDLE);
	}

	// The rest
	const int slowCasts = g_globals->_combat ?
		g_globals->_combat->getSlowCasts() : 0;
	for (int i = 8; i < 18; ++i) {
		if (i == 13) {
			if (slowCasts == 1) {
				writeString(0, yp += 8,
					"Slow - enemy Speed halved", ALIGN_MIDDLE);
			} else if (slowCasts > 1) {
				writeString(0, yp += 8, Common::String::format(
					"Slow x%d - Speed halved per cast", slowCasts),
					ALIGN_MIDDLE);
			} else {
				yp += 8;
			}
		}

		if (g_globals->_activeSpells._arr[i]) {
			Common::String line = PROTECTION_NAMES[i];
			if (i == 8) {
				line += Common::String::format(" +%d AC",
					g_globals->_activeSpells._arr[i]);
			} else if (i == 9) {
				line += " - prevents falls";
			} else if (i == 10) {
				line += " - allows water travel";
			} else if (i == 11) {
				line += " - prevents surprise";
			} else if (i == 12) {
				line += " 100%";
			} else if (i == 13) {
				line += " +1 hit/damage";
			} else if (i == 14) {
				line += " +3 AC vs attacks";
			} else if (i == 15) {
				line += " -8 missile damage";
			} else if (i == 16) {
				line += " -50% damage";
			} else if (i == 17) {
				line += Common::String::format("%d to attacks",
					g_globals->_activeSpells._arr[17]);
			}

			writeString(0, yp += 8, line, ALIGN_MIDDLE);
		}
	}

	const int weakenCasts = g_globals->_combat ?
		g_globals->_combat->getWeakenCasts() : 0;
	if (weakenCasts == 1) {
		writeString(0, yp += 8, "Weaken - enemy AC -1", ALIGN_MIDDLE);
	} else if (weakenCasts > 1) {
		writeString(0, yp += 8, Common::String::format(
			"Weaken x%d - enemy AC -%d", weakenCasts, weakenCasts),
			ALIGN_MIDDLE);
	}

	// Character-specific combat boosts. Multiple boosts on one character are
	// combined into one compact row to preserve room in the Protect window.
	if (g_globals->_combat) {
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Common::String effects;
			const int heroism = g_globals->_combat->getHeroismBonus(i);
			const int power = g_globals->_combat->getPowerBonus(i);
			const int quickness = g_globals->_combat->getQuicknessBonus(i);

			if (heroism) {
				effects = Common::String::format(
					g_globals->_combat->isSuperHeroism(i) ?
					"Super Heroism +%d Level" : "Heroism +%d Level",
					heroism);
			}
			if (power >= 0) {
				if (!effects.empty())
					effects += ", ";
				effects += Common::String::format("Power +%d Might", power);
			}
			if (quickness >= 0) {
				if (!effects.empty())
					effects += ", ";
				effects += Common::String::format(
					"Quickness +%d Speed", quickness);
			}

			if (!effects.empty()) {
				writeString(0, yp += 8, Common::String::format("%s: %s",
					g_globals->_party[i]._name, effects.c_str()),
					ALIGN_MIDDLE);
			}
		}
	}

}

bool Protect::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_ESCAPE:
		close();
		return true;
	default:
		break;
	}

	return false;
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
