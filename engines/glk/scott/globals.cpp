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

#include "glk/scott/globals.h"
#include "glk/scott/scott.h"
#include "glk/scott/command_parser.h"
#include "glk/scott/line_drawing.h"
#include "glk/scott/saga_draw.h"
#include "common/algorithm.h"

namespace Glk {
namespace Scott {

Globals *g_globals;

Globals::Globals() : _sys(MAX_SYSMESS), _directions(NUMBER_OF_DIRECTIONS), _extraNouns(NUMBER_OF_EXTRA_NOUNS),
					 _skipList(NUMBER_OF_SKIPPABLE_WORDS), _delimiterList(NUMBER_OF_DELIMITERS), _systemMessages(60),
					 _vectorState(NO_VECTOR_IMAGE) {

	g_globals = this;

	_gameHeader = new Header;

	_englishDirections = {nullptr, "north", "south", "east", "west", "up", "down", "n", "s", "e", "w", "u", "d", " "};

	_extraCommands = {nullptr, "restart", "save", "restore", "load", "transcript", "script", "oops", "undo", "ram",
					  "ramload", "ramrestore", "ramsave", "except", "but", " ", " ", " ", " ", " "};

	_extraCommandsKey = {NO_COMMAND, RESTART, SAVE, RESTORE, RESTORE, SCRIPT, SCRIPT, UNDO, UNDO, RAM,
						 RAMLOAD, RAMLOAD, RAMSAVE, EXCEPT, EXCEPT, RESTORE, RESTORE, SCRIPT, UNDO, RESTART};

	_englishExtraNouns = {nullptr, "game", "story", "on", "off", "load", "restore", "save", "move",
						  "command", "turn", "all", "everything", "it", " ", " "};

	_extraNounsKey = {NO_COMMAND, GAME, GAME, ON, OFF, RAMLOAD, RAMLOAD, RAMSAVE, COMMAND, COMMAND, COMMAND,
					  ALL, ALL, IT, ON, OFF};

	_abbreviations = {nullptr, "i", "l", "x", "z", "q"};

	_abbreviationsKey = {nullptr, "inventory", "look", "examine", "wait", "quit"};

	_englishSkipList = {nullptr, "at", "to", "in", "into", "the", "a", "an", "my", "quickly",
						"carefully", "quietly", "slowly", "violently", "fast", "hard", "now", "room"};

	_englishDelimiterList = {nullptr, ",", "and", "then", " "};

	Common::fill(&_counters[0], &_counters[16], 0);
	Common::fill(&_roomSaved[0], &_roomSaved[16], 0);
}

Globals::~Globals() {
	delete _gameHeader;
}

} // End of namespace Scott
} // End of namespace Glk
