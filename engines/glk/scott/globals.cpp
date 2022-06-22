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

#include "common/algorithm.h"
#include "glk/scott/scott.h"
#include "glk/scott/command_parser.h"
#include "glk/scott/line_drawing.h"
#include "glk/scott/saga_draw.h"
#include "glk/scott/globals.h"

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

	_spanishDirections = {nullptr, "norte", "sur", "este", "oeste", "arriba", "abajo", "n", "s", "e", "o", "u", "d", "w"};

	_germanDirections = {nullptr, "norden", "sueden", "osten", "westen", "oben", "unten", "n", "s", "o", "w", "u", "d", " "};

	_germanExtraCommands = {nullptr, "restart", "save", "restore", "load", "transcript", "script", "oops", "undo", "ram", "ramload", "ramrestore", "ramsave",
							"ausser", "bis", "laden", "wiederherstellen", "transkript", "rueckgaengig", "neustarten"};

	_spanishExtraCommands = {nullptr, "restart", "save", "restore", "load", "transcript", "script", "oops", "undo", "ram", "ramload", "ramrestore", "ramsave",
							 "excepto", "menos", "reanuda", "cargar", "transcripcion", "deshacer", "reinicia"};

	_germanExtraNouns = {nullptr, "spiel", "story", "on", "off", "wiederherstellen", "laden", "speichern", "move", "verschieben", "runde",
						 "alle", "alles", "es", "einschalten", "ausschalten"};

	_spanishExtraNouns = {nullptr, "juego", "story", "on", "off", "cargar", "reanuda", "conserva", "move", "command", "jugada", "toda", "todo", "eso", "activar", "desactivar"};

	_germanSkipList = {nullptr, "nach", "die", "der", "das", "im", "mein", "meine", "an", "auf", "den", "lassen", "lass", "fallen", " ", " ", " ", " "};

	_germanDelimiterList = {nullptr, ",", "und", "dann", "and"};

	Common::fill(&_counters[0], &_counters[16], 0);
	Common::fill(&_roomSaved[0], &_roomSaved[16], 0);

	_md5Index.setVal("cb7dadc9d5f8bce453b9139265e4dd7d",  0); // goldenbaton
	_md5Index.setVal("b22d1f4d46c99ff4443d541d3fe424c1",  2); // timemachine
	_md5Index.setVal("3a5c3f4079c1c0347f03420db8ad4596",  4); // arrowofdeath1
	_md5Index.setVal("d3f8943c4f5f71ce00139065055a72ee",  6); // arrowofdeath2
	_md5Index.setVal("441edd90fc7f9ff39a5eebe035a974e9",  8); // pulsar7
	_md5Index.setVal("ed99306a2fb23bf6579068a4d74034ee", 10); // circus
	_md5Index.setVal("5e381e83f15d77e3542be4a4cffc8e25", 12); // feasibility
	_md5Index.setVal("b0f8676817475753f1edd7f1eeea31fb", 14); // akyrz
	_md5Index.setVal("84d5fbb16a37e495abf09d191fd8b1a2", 16); // perseus
	_md5Index.setVal("afde056c152de79ea20453c42a2d08af", 18); // 10indians
	_md5Index.setVal("6c6fbbbb50032463a6ea71c6750ea1f5", 20); // waxworks11
	_md5Index.setVal("0eec511d3cde815c73e5464ab0cdbef9" ,22); // marveladventure
	_md5Index.setVal("b4d8fc4eabed4f2400717303561ad0fa",  0); // misadv1
	_md5Index.setVal("3ce5ea1a0473244bf469fd3c51f1dc48",  6); // midadv2
	_md5Index.setVal("10109d9776b9372f9c768b53a664b113", 12); // robin of sherwood
	_md5Index.setVal("552c95ec15d750cbfa02c1f11dcbca1e", 15); // robin of sherwood
	_md5Index.setVal("108063b2a16a199794f2ecf52ce26377", 16); // gremlins
	_md5Index.setVal("947e35037cf02269ac013857925137ce", 17); // gremlins
	_md5Index.setVal("33c920f7ba150dfa1a832d510eebd8fe", 19); // gremlins
	_md5Index.setVal("e5d743d8727c8aca011a737bbb5ad818", 21); // gremlins
	_md5Index.setVal("267c3fe2bb150365de0358f07b5df15c", 24); // seas of blood
	_md5Index.setVal("46280fb1d701a41492b8434467c31029", 23); // seas of blood
	_md5Index.setVal("a1db488c49ad221fa0dc79591cb5a3db", 26); // sorcerer of claymorgue castle
	_md5Index.setVal("1ebaf9a378355246aa7ed2623bb27fab", 28); // sorcerer of claymorgue castle
	_md5Index.setVal("dde67117a432420ef05f8d665fbbbe10", 27); // sorcerer of claymorgue castle
	_md5Index.setVal("64fcee173adecc0f03c595e25d4def04", 30); // sorcerer of claymorgue castle
	_md5Index.setVal("05cf6c64ecde5288ae2e46099bfd19a3", 31); // adventureland
	_md5Index.setVal("5af919881417920ec6a3961b4577f587", 32); // adventureland
	_md5Index.setVal("414d459ceb211230356ad823475866b3", 38); // savage island
	_md5Index.setVal("d80e133dd396565f773052cb317e8222", 40); // questprobe hulk
}

Globals::~Globals() {
	delete _gameHeader;
}

} // End of namespace Scott
} // End of namespace Glk
