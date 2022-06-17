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

/*
 * Based on ScottFree interpreter version 1.14 developed by Swansea
 * University Computer Society without disassembly of any other game
 * drivers, only of game databases as permitted by EEC law (for purposes
 * of compatibility).
 *
 * Licensed under GPLv2
 *
 * https://github.com/angstsmurf/spatterlight/tree/master/terps/scott
 */

#ifndef GLK_SCOTT_PARSER_H
#define GLK_SCOTT_PARSER_H

#include "common/str-array.h"
#include "glk/glk_types.h"

namespace Glk {
namespace Scott {

#define NUMBER_OF_DIRECTIONS 14
#define NUMBER_OF_SKIPPABLE_WORDS 18
#define NUMBER_OF_DELIMITERS 5
#define NUMBER_OF_EXTRA_COMMANDS 20
#define NUMBER_OF_EXTRA_NOUNS 16

struct Command {
	int _verb;
	int _noun;
	int _item;
	int _verbWordIndex;
	int _nounWordIndex;
	int _allFlag;
	struct Command *_previous;
	struct Command *_next;
};

enum ExtraCommand : int {
	NO_COMMAND,
	RESTART,
	SAVE,
	RESTORE,
	SCRIPT,
	ON,
	OFF,
	UNDO,
	RAM,
	RAMSAVE,
	RAMLOAD,
	GAME,
	COMMAND,
	ALL,
	IT,
	EXCEPT
};

char **splitIntoWords(glui32 *string, int length);
int getInput(int *vb, int *no);
void freeCommands();
glui32 *toUnicode(const char *string);
char *fromUnicode(glui32 *unicodeString, int origLength);
int recheckForExtraCommand();
int whichWord(const char *word, Common::StringArray list, int wordLength);

} // End of namespace Scott
} // End of namespace Glk

#endif
