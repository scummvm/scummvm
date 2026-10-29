
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

// Same generic dialog box geometry the Lose and Win views use
constexpr int DIALOG_X1 = 31, DIALOG_Y1 = 23, DIALOG_X2 = 287, DIALOG_Y2 = 127;
constexpr int BORDER_X1 = 34, BORDER_Y1 = 26, BORDER_X2 = 284, BORDER_Y2 = 124;

// The four dungeon monsters kings with an even castle index send the
// player to kill, indexed by (castleIndex - 2) / 2. They're padded to
// roughly center under the heading
static const char *const QUEST_MONSTER_NAMES[4] = {
	" Gelatinous Cube", " Carrion Creeper", "     Lich", "    Balron"
};

// The four landmarks kings with an odd castle index send the player to
// find, as indexes into Data::LOCATION_NAMES, indexed by (castleIndex - 1) / 2
static const int QUEST_LOCATION_INDEXES[4] = { 47, 45, 43, 41 };

// Text column of each of those landmark names within the quest dialog
static const int QUEST_LOCATION_X[4] = { 8, 10, 9, 9 };

// Reward text shown for a completed "kill a monster" quest, indexed by the
// continent the player is currently standing on. These are the lines for
// rows 9, 10, 12 and 13 of the dialog
static const char *const GEM_REWARD_LINES[4][4] = {
	{ "that it requires all four gems", "  to launch a time machine!", "The king gives thee a red gem.", nullptr },
	{ "that the time machine must be", "     used in order to win.", "     The king gives thee a", "           green gem!" },
	{ "   the princess will help a", "   space ace through time!", "    The king gives thee a", "          blue gem!" },
	{ "   to take nine items from my", "  storerooms - but only nine!", "   The king also gives thee a", "          white gem!" }
};

King::King() : Interaction("King") {
}

bool King::msgFocus(const FocusMessage &msg) {
	_mode = KING_PROMPT;
	setBounds(Common::Rect(0, 0, 0, 0));

	if (checkQuestReward())
		return true;

	writeString("Dost thou offer pence\n");
	writeString("or service: ");
	showCursor();
	return true;
}

bool King::msgAction(const ActionMessage &msg) {
	if (_mode == KING_QUEST || _mode == KING_REWARD)
		return true;

	writeString("neither\n");
	finish();
	return true;
}

bool King::msgKeypress(const KeypressMessage &msg) {
	switch (_mode) {
	case KING_QUEST:
	case KING_REWARD:
		if (msg.keycode == Common::KEYCODE_SPACE || msg.keycode == Common::KEYCODE_RETURN) {
			prompt();
			close();
		}
		break;

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
				finish();
			} else {
				giveQuest();
			}

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

	if (castleIdx % 2 != 0) {
		// Odd castle index - a "find a location" quest rewards strength points
		_rewardContinent = -1;
		_rewardPoints = (99 - sg._strength) / 8;
		if (_rewardPoints == 0 && sg._strength != 99)
			_rewardPoints = 1;

		sg._strength += _rewardPoints;

	} else {
		// Even castle index - a "kill a monster" quest rewards a gem, based
		// on the continent the castle is on. WORKAROUND: the original reuses
		// a single position field for both overworld and in-location
		// coordinates, so this ends up passing meaningless in-castle
		// coordinates to the continent lookup; using the real overworld
		// position instead
		_rewardContinent = _G(map).getContinentAt(sg._overworldPos);

		switch (_rewardContinent) {
		case 0:
			++sg._redGems;
			break;
		case 1:
			++sg._greenGems;
			break;
		case 2:
			++sg._blueGem;
			break;
		default:
			++sg._whiteGem;
			sg._castleItemAllowance = 9;
			break;
		}
	}

	_mode = KING_REWARD;
	setBounds(Graphics::Views::TextRect(0, 0, 39, 19));
	redraw();
	redrawStats();
	g_engine->send("Commands", GameMessage("SPACE2"));
	return true;
}

void King::giveQuest() {
	_G(savegame)._questStatus[_G(map)._castleNum2] = -1;
	_mode = KING_QUEST;

	setBounds(Graphics::Views::TextRect(0, 0, 39, 19));
	redraw();
	g_engine->send("Commands", GameMessage("SPACE2"));
}

void King::draw() {
	if (_mode == KING_QUEST)
		drawQuest();
	else if (_mode == KING_REWARD)
		drawReward();
}

void King::drawBox() {
	auto s = getSurface();
	s.fillRect(Common::Rect(DIALOG_X1, DIALOG_Y1, DIALOG_X2, DIALOG_Y2), 0);
	s.frameRect(Common::Rect(BORDER_X1, BORDER_Y1, BORDER_X2, BORDER_Y2), _G(colors)._border);
	s.setColor(_G(colors)._text);
}

void King::drawReward() {
	drawBox();
	auto s = getSurface();

	const char *name = _G(savegame)._name;
	s.writeString(Common::Point(12 - (int)strlen(name) / 2, 5), "Ah, %s!  Thou hast", name);
	s.writeString(Common::Point(10, 6), "completed thy quest.");

	if (_rewardContinent < 0) {
		s.writeString(Common::Point(8, 8), "For this I will give thee");
		s.writeString(Common::Point(10, 9), "%d point%s of strength.", _rewardPoints, _rewardPoints == 1 ? "" : "s");
	} else {
		s.writeString(Common::Point(7, 8), "For this I shall tell thee");

		static const int ROWS[4] = { 9, 10, 12, 13 };
		for (int i = 0; i < 4; ++i) {
			if (GEM_REWARD_LINES[_rewardContinent][i])
				s.writeString(Common::Point(5, ROWS[i]), "%s", GEM_REWARD_LINES[_rewardContinent][i]);
		}
	}
}

void King::drawQuest() {
	int castleIdx = _G(map)._castleNum2;

	drawBox();
	auto s = getSurface();

	if (castleIdx % 2 == 0) {
		s.writeString(Common::Point(12, 6), "Go now and kill a");

		int monsterIdx = (castleIdx - 2) / 2;
		if (monsterIdx >= 0 && monsterIdx < 4)
			s.writeString(Common::Point(12, 8), QUEST_MONSTER_NAMES[monsterIdx]);
	} else {
		s.writeString(Common::Point(11, 6), "Go forth and find");

		int locIdx = (castleIdx - 1) / 2;
		if (locIdx >= 0 && locIdx < 4)
			s.writeString(Common::Point(QUEST_LOCATION_X[locIdx], 8), Data::LOCATION_NAMES[QUEST_LOCATION_INDEXES[locIdx]]);
	}

	s.writeString(Common::Point(11, 10), "Do not return until");
	s.writeString(Common::Point(11, 11), "thy quest is done!");
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
