
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

#include "ultima/ultima1/views/interactions/king.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

// The four dungeon monsters kings with an even castle index send the
// player to kill, indexed by (castleIndex - 2) / 2
static const char *const QUEST_MONSTER_NAMES[4] = {
	"Gelatinous Cube", "Carrion Creeper", "Lich", "Balron"
};

// The four landmarks kings with an odd castle index send the player to
// find, as indexes into Data::LOCATION_NAMES, indexed by (castleIndex - 1) / 2
static const int QUEST_LOCATION_INDEXES[4] = { 47, 45, 43, 41 };

// Reward text shown for a completed "kill a monster" quest, indexed by the
// continent the player is currently standing on
static const char *const GEM_REWARD_MESSAGES[4] = {
	"that it requires all four gems\nto launch a time machine!\n\nThe king gives thee a red gem.\n",
	"that the time machine must be\nused in order to win.\n\nThe king gives thee a green gem!\n",
	"the princess will help a\nspace ace through time!\n\nThe king gives thee a blue gem!\n",
	"to take nine items from my\nstorerooms - but only nine!\n\nThe king also gives thee a white gem!\n"
};

King::King() : Interaction("King") {
}

bool King::msgFocus(const FocusMessage &msg) {
	_mode = KING_PROMPT;
	setBounds(Common::Rect(0, 0, 0, 0));

	if (checkQuestReward()) {
		finish();
		return true;
	}

	writeString("Dost thou offer pence\n");
	writeString("or service: ");
	showCursor();
	return true;
}

bool King::msgAction(const ActionMessage &msg) {
	writeString("neither\n");
	finish();
	return true;
}

bool King::msgKeypress(const KeypressMessage &msg) {
	switch (_mode) {
	case KING_PROMPT:
		if (msg.keycode == Common::KEYCODE_p) {
			_mode = KING_PENCE;
			_amount.clear();
			writeString("pence\n");
			resetLine();
			writeString("\x10""How much? ");
			showCursor();

		} else if (msg.keycode == Common::KEYCODE_s) {
			int castleIdx = _G(map)._castleNum2;

			if (_G(savegame)._questStatus[castleIdx] == -1) {
				writeString("\n");
				writeString("Thou art on a quest\n");
				writeString("for me already!\n");
			} else {
				giveQuest();
			}
			finish();

		} else {
			writeString("neither\n");
			finish();
		}
		break;

	case KING_PENCE:
		penceKeypress(msg);
		break;
	}

	return true;
}

bool King::checkQuestReward() {
	auto &sg = _G(savegame);
	int castleIdx = _G(map)._castleNum2;

	if (sg._questStatus[castleIdx] != 1)
		return false;

	sg._questStatus[castleIdx] = 0;

	writeString(Common::String::format("Ah, %s!  Thou hast\n", sg._name));
	writeString("completed thy quest.\n");

	if (castleIdx % 2 != 0) {
		// Odd castle index - a "find a location" quest rewards strength points
		int amount = (99 - sg._strength) / 8;
		if (amount == 0 && sg._strength != 99)
			amount = 1;

		writeString(Common::String::format("For this I will give thee %d point%s of strength.\n",
			amount, amount == 1 ? "" : "s"));
		sg._strength += amount;

	} else {
		// Even castle index - a "kill a monster" quest rewards a gem, based
		// on the continent the castle is on. WORKAROUND: the original reuses
		// a single position field for both overworld and in-location
		// coordinates, so this ends up passing meaningless in-castle
		// coordinates to the continent lookup; using the real overworld
		// position instead
		int continent = _G(map).getContinentAt(sg._overworldPos);
		writeString("For this I shall tell thee\n");
		writeString(GEM_REWARD_MESSAGES[continent]);

		switch (continent) {
		case 0:
			++sg._redGems;
			break;
		case 1:
			++sg._greenGems;
			break;
		case 2:
			++sg._blueGem;
			break;
		case 3:
			++sg._whiteGem;
			sg._castleItemAllowance = 9;
			break;
		default:
			break;
		}
	}

	redrawStats();
	return true;
}

void King::giveQuest() {
	auto &sg = _G(savegame);
	int castleIdx = _G(map)._castleNum2;
	sg._questStatus[castleIdx] = -1;

	if (castleIdx % 2 == 0) {
		writeString("Go now and kill a\n");

		int monsterIdx = (castleIdx - 2) / 2;
		if (monsterIdx >= 0 && monsterIdx < 4)
			writeString(Common::String::format("%s\n", QUEST_MONSTER_NAMES[monsterIdx]));
	} else {
		writeString("Go forth and find\n");

		int locIdx = (castleIdx - 1) / 2;
		if (locIdx >= 0 && locIdx < 4)
			writeString(Common::String::format("%s\n", Data::LOCATION_NAMES[QUEST_LOCATION_INDEXES[locIdx]]));
	}

	writeString("Do not return until\n");
	writeString("thy quest is done!\n");
}

void King::penceKeypress(const KeypressMessage &msg) {
	if (msg.ascii >= '0' && msg.ascii <= '9') {
		if ((_amount.empty() && msg.ascii == '0') || _amount.size() >= 4)
			return;

		_amount += (char)msg.ascii;
		writeString(Common::String(1, (char)msg.ascii));

	} else if (msg.keycode == Common::KEYCODE_BACKSPACE) {
		if (!_amount.empty()) {
			_amount.deleteLastChar();
			resetLine();
			writeString(Common::String::format("\x10""How much? %s", _amount.c_str()));
			showCursor();
		}

	} else {
		auto &sg = _G(savegame);
		int amount = _amount.empty() ? 0 : atoi(_amount.c_str());

		writeString("\n");
		if (amount == 0) {
			writeString("none\n");
			finish();
		} else if (amount > sg._coins) {
			writeString("Thou hast not that much!\n");
			g_engine->playFX(1);
			finish();
		} else {
			sg._coins -= amount;
			int hits = amount * 3 / 2;
			sg._hits += hits;

			writeString("In return I give unto\n");
			writeString(Common::String::format("thee %d hit points\n", hits));
			redrawStats();
			finish();
		}
	}
}

void King::finish() {
	writeString("\n");
	prompt();
	close();
}

void King::redrawStats() {
	g_engine->findView("Stats")->draw();
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima
