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

#include "common/array.h"
#include "common/str.h"
#include "common/str-array.h"
#include "glk/scott/definitions.h"

namespace Glk {
namespace Scott {

/* This is supposed to be the original ScottFree system
 messages in second person, as far as possible */
const char *g_sysDict[] = {
	"North",
	"South",
	"East",
	"West",
	"Up",
	"Down",
	"The game is now over. Play again?",
	"You have stored",
	"treasures. ",
	"On a scale of 0 to 100 that rates",
	"O.K.",
	"O.K.",
	"O.K. ",
	"Well done.\n",
	"I don't understand ",
	"You can't do that yet. ",
	"Huh ? ",
	"Give me a direction too. ",
	"You haven't got it. ",
	"You have it. ",
	"You don't see it. ",
	"It is beyond your power to do that. ",
	"\nDangerous to move in the dark! ",
	"You fell down and broke your neck. ",
	"You can't go in that direction. ",
	"I don't know how to \"",
	"\" something. ",
	"I don't know what a \"",
	"\" is. ",
	"You can't see. It is too dark!\n",
	"You are in a ",
	"\nYou can also see: ",
	"Obvious exits: ",
	"You are carrying:\n",
	"Nothing.\n",
	"Tell me what to do ? ",
	"<HIT ENTER>",
	"Light has run out. ",
	"Light runs out in",
	"turns! ",
	"You are carrying too much. \n",
	"You're dead. ",
	"Resume a saved game? ",
	"None",
	"There's nothing here to take. ",
	"You carry nothing. ",
	"Your light is growing dim. ",
	", ",
	"\n",
	" - ",
	"What ?",
	"yes",
	"no",
	"Answer yes or no.\n",
	"Are you sure? ",
	"Move undone. ",
	"Can't undo on first turn. ",
	"No more undo states stored. ",
	"Saved. ",
	"You can't use ALL with that verb. ",
	"Transcript is now off.\n",
	"Transcript is now on.\n",
	"No transcript is currently running.\n",
	"A transcript is already running.\n",
	"Failed to create transcript file. ",
	"Start of transcript\n\n",
	"\n\nEnd of transcript\n",
	"BAD DATA! Invalid save file.\n",
	"State saved.\n",
	"State restored.\n",
	"No saved state exists.\n"
};

/* These are supposed to be the original ScottFree system
 messages in first person, as far as possible */
const char *g_sysDictIAm[] = {
	"North",
	"South",
	"East",
	"West",
	"Up",
	"Down",
	"The game is now over. Play again?",
	"You have stored",
	"treasures. ",
	"On a scale of 0 to 100 that rates",
	"O.K.",
	"O.K.",
	"O.K. ",
	"Well done.\n",
	"I don't understand ",
	"I can't do that yet. ",
	"Huh ? ",
	"Give me a direction too.",
	"I'm not carrying it. ",
	"I already have it. ",
	"I don't see it here. ",
	"It is beyond my power to do that. ",
	"Dangerous to move in the dark! ",
	"\nI fell and broke my neck.",
	"I can't go in that direction. ",
	"I don't know how to \"",
	"\" something. ",
	"I don't know what a \"",
	"\" is. ",
	"I can't see. It is too dark!\n",
	"I'm in a ",
	"\nI can also see: ",
	"Obvious exits: ",
	"I'm carrying: \n",
	"Nothing.\n",
	"Tell me what to do ? ",
	"<HIT ENTER>",
	"Light has run out. ",
	"Light runs out in",
	"turns! ",
	"I've too much to carry. \n",
	"I'm dead. ",
	"Resume a saved game? ",
	"None",
	"There's nothing here to take. ",
	"I have nothing to drop. ",
	"My light is growing dim. ",
	nullptr
};

/* These are supposed to be the original TI-99/4A system
 messages in first person, as far as possible */
const char *g_sysDictTI994A[] = {
	"North",
	"South",
	"East",
	"West",
	"Up",
	"Down",
	"This adventure is over. Play again?",
	"You have stored",
	"treasures. ",
	"On a scale of 0 to 100 that rates",
	"OK. ",
	"OK. ",
	"OK. ",
	"Well done.\n",
	"I don't understand the command. ",
	"I can't do that yet. ",
	"Huh? ",
	"Give me a direction too.",
	"I'm not carrying it. ",
	"I already have it. ",
	"I don't see it here. ",
	"It is beyond my power to do that. ",
	"Dangerous to move in the dark!\n",
	"\nI fell down and broke my neck.",
	"I can't go in that direction. ",
	"I don't know how to \"",
	"\" something. ",
	"I don't know what a \"",
	"\" is. ",
	"I can't see. It is too dark!\n",
	"I am in a ",
	"\nVisible items are : ",
	"Obvious exits : ",
	"I am carrying : ",
	"Nothing. ",
	"What shall I do? ",
	"<HIT ENTER>",
	"Light went out! ",
	"Light runs out in",
	"turns! ",
	"I am carrying too much.\n",
	"I'm dead... ",
	"Resume a saved game? ",
	"None",
	"There's nothing here to take. ",
	"I have nothing to drop. ",
	"Light is growing dim ",
	", ",
	" ",
	", ",
	nullptr,
};

const char *g_sysDictZX[] = {
	"NORTH",
	"SOUTH",
	"EAST",
	"WEST",
	"UP",
	"DOWN",
	"The Adventure is over. Want to try this Adventure again? ",
	"I've stored",
	"Treasures. ",
	"On a scale of 0 to 100 that rates",
	"Dropped.",
	"Taken.",
	"O.K. ",
	"FANTASTIC! You've solved it ALL! \n",
	"I must be stupid, but I just don't understand what you mean ",
	"I can't do that...yet! ",
	"Huh? ",
	"I need a direction too. ",
	"I'm not carrying it. ",
	"I already have it. ",
	"I don't see it here. ",
	"It's beyond my Power to do that. ",
	"It's dangerous to move in the dark! ",
	"\nI fell and broke my neck! I'm DEAD! ",
	"I can't go in that direction. ",
	"I don't know how to \"",
	"\" something. ",
	"I don't know what a \"",
	"\" is. ",
	"It's too dark to see!\n",
	"I am in a ",
	". Visible items:\n",
	"Exits: ",
	"I'm carrying the following: ",
	"Nothing at all. ",
	"---TELL ME WHAT TO DO ? ",
	"<HIT ENTER> ",
	"Light has run out. ",
	"Light runs out in",
	"turns! ",
	"I'm carrying too much! Try: TAKE INVENTORY. ",
	"I'm DEAD!! ",
	"Restore a previously saved game ? ",
	"None",
	"There's nothing here to take. ",
	"I have nothing to drop. ",
	"My light is growing dim. ",
	" ",
	" ",
	". ",
	"What ? ",
	nullptr
};

} // End of namespace Scott
} // End of namespace Glk
