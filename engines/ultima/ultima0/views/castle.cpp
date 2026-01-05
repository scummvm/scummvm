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

#include "ultima/ultima0/views/castle.h"
#include "ultima/ultima0/ultima0.h"

namespace Ultima {
namespace Ultima0 {
namespace Views {

static const char *GO_FORTH = "     Go now upon this quest, and may\n"
	"Lady Luck be fair unto you.....\n"
	".....Also I, British, have increased\n"
	"each of thy attributes by one.!";

bool Castle::msgFocus(const FocusMessage &msg) {
	const auto &player = g_engine->_player;

	if (Common::String(player._name).empty())
		_mode = NAMING;
	else if (player._taskCompleted) {
		_mode = TASK_COMPLETE;
	} else {
		_mode = TASK_INCOMPLETE;
	}

	g_engine->playMidi("lordbrit.mid");
	return true;
}

bool Castle::msgUnfocus(const UnfocusMessage &msg) {
	g_engine->stopMidi();
	return true;
}

void Castle::draw() {
	auto s = getSurface();
	s.clear();

	if (_mode <= FIRST_TASK) {
		firstTime();
	} else if (_mode == TASK_COMPLETE) {
		taskCompleted();
	} else if (_mode == TASK_INCOMPLETE) {
		taskIncomplete();
	}
}

void Castle::firstTime() {
	auto s = getSurface();
	const auto &player = g_engine->_player;

	if (_mode >= NAMING && _mode <= FIRST_TASK) {
		s.writeString(Common::Point(5, 2), "Welcome Peasant into the halls of\n"
			"the mighty Lord British.  Herein thou\n"
			"may choose to dare battle with the\n"
			"evil creatures of the depths, for\n"
			"great reward!\n\n"
			"What is thy name peasant? ");
		s.setColor(C_GREY);
		s.writeString(_playerName);
		s.setColor(C_TEXT_DEFAULT);
	}

	if (_mode >= GRAND_ADVENTURE && _mode <= FIRST_TASK)
		s.writeString(Common::Point(0, 10), "Doest thou wish for great adventure ?\n\n");

	if (_mode == BEGONE) {
		s.writeString("Then leave and begone!");
	} else if (_mode == FIRST_TASK) {
		s.writeString("Good! Thou shalt try to become a\nKnight!!!\n\n"
			"Thy first task is to go into the\n"
			"dungeons and to return only after\n"
			"killing ");
		s.setColor(C_VIOLET);
		s.writeString(getTaskName(player._task));

		pressAnyKey();
	}
}

void Castle::taskCompleted() {
	auto &player = g_engine->_player;
	auto s = getSurface();

	s.writeString(Common::Point(0, 3), "Aaaahhhh.... ");
	s.writeString(player._name);
	s.writeString("\n\nThou has accomplished\nthy quest.\n\n");

	if (player._task == MAX_MONSTERS) {
		s.writeString("Thou hast proved thyself worthy of\n"
			"Knighthood, continue if thou doth wish\n"
			"but thou hast accomplished the\n"
			"main objective of the game.\n\n"
			"Now, maybe thou art foolhardy enough to\n"
			"try difficulty level ");
		s.writeString(Common::String::format("%d.", player._skill + 1));
	} else {
		// LB gives you extra attributes
		for (int i = 0; i < MAX_ATTR; i++)
			player._attr[i]++;

		// Choose the next task
		nextTask();

		s.writeString("Unfortunately, this is not enough to\n"
			"become a knight.\n\n");
		s.writeString("Thou must now kill ");
		s.setColor(C_VIOLET);
		s.writeString(getTaskName(player._task));

		s.setColor(C_TEXT_DEFAULT);
		s.writeString("\n\n");
		s.writeString(GO_FORTH);

		pressAnyKey();
	}
}

void Castle::taskIncomplete() {
	const auto &player = g_engine->_player;
	auto s = getSurface();

	s.writeString(Common::Point(0, 3),
		"Why hast thou returned ?\n"
		"Thou must kill ");
	s.setColor(C_VIOLET);
	s.writeString(getTaskName(player._task));

	s.setColor(C_TEXT_DEFAULT);
	s.writeString("\n\nGo now and complete thy quest");
}

void Castle::pressAnyKey() {
	auto s = getSurface();
	s.writeString(Common::Point(20, 23), "Press any Key to Continue",
		Graphics::kTextAlignCenter);
}

bool Castle::msgKeypress(const KeypressMessage &msg) {
	auto &player = g_engine->_player;

	switch (_mode) {
	case NAMING:
		if (msg.keycode == Common::KEYCODE_BACKSPACE && !_playerName.empty()) {
			_playerName.deleteLastChar();
		} else if (msg.keycode == Common::KEYCODE_RETURN && !_playerName.empty()) {
			Common::strcpy_s(player._name, _playerName.c_str());
			_mode = GRAND_ADVENTURE;
		} else if (Common::isAlpha(msg.ascii) && _playerName.size() < MAX_NAME) {
			_playerName += msg.ascii;
		}
		redraw();
		break;

	case GRAND_ADVENTURE:
		if (msg.keycode == Common::KEYCODE_y) {
			nextTask();
			_mode = FIRST_TASK;
		} else if (msg.keycode == Common::KEYCODE_n) {
			_mode = BEGONE;
		}
		redraw();
		break;

	default:
		// All other modes exit the castle
		replaceView("WorldMap");
		break;
	}

	return true;
}

bool Castle::msgAction(const ActionMessage &msg) {
	if (_mode >= FIRST_TASK) {
		replaceView("WorldMap");
	}

	return true;
}

void Castle::nextTask() {
	auto &player = g_engine->_player;

	player._task = CLIP(player._attr[AT_WISDOM] / 3, 1, 10);
	player._taskCompleted = false;
}

Common::String Castle::getTaskName(int taskNum) const {
	Common::String mons = MONSTER_INFO[taskNum]._name;

	return Common::String::format("%s %s",
		strchr("aeiou", tolower(mons.firstChar())) != nullptr ? "an" : "a",
		mons.c_str());
}

} // namespace Views
} // namespace Ultima0
} // namespace Ultima
