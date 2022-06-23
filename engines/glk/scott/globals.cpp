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
	_md5Index.setVal("4262f85382d1bc3b8924a1929511a558", 13); // robin of sherwood
	_md5Index.setVal("bf3a4d72cff5ef97bebce6b12c756df2", 14); // robin of sherwood
	_md5Index.setVal("552c95ec15d750cbfa02c1f11dcbca1e", 15); // robin of sherwood
	_md5Index.setVal("108063b2a16a199794f2ecf52ce26377", 16); // gremlins
	_md5Index.setVal("947e35037cf02269ac013857925137ce", 17); // gremlins
	_md5Index.setVal("95b2582a89c59961d5b709c9b32e4131", 18); // gremlins
	_md5Index.setVal("33c920f7ba150dfa1a832d510eebd8fe", 19); // gremlins
	_md5Index.setVal("644c366956202d41df0ea1c4303c5895", 20); // gremlins
	_md5Index.setVal("e5d743d8727c8aca011a737bbb5ad818", 21); // gremlins
	_md5Index.setVal("c60977950ff22ae57483f073345b8373", 22); // gremlins
	_md5Index.setVal("46280fb1d701a41492b8434467c31029", 23); // seas of blood
	_md5Index.setVal("267c3fe2bb150365de0358f07b5df15c", 24); // seas of blood
	_md5Index.setVal("0300c2d21289157539bbd03ab4e366ee", 25); // seas of blood
	_md5Index.setVal("a1db488c49ad221fa0dc79591cb5a3db", 26); // sorcerer of claymorgue castle
	_md5Index.setVal("dde67117a432420ef05f8d665fbbbe10", 27); // sorcerer of claymorgue castle
	_md5Index.setVal("1ebaf9a378355246aa7ed2623bb27fab", 28); // sorcerer of claymorgue castle
	_md5Index.setVal("8d2af429e53df1c4da0d21bdc9de6826", 29); // sorcerer of claymorgue castle
	_md5Index.setVal("64fcee173adecc0f03c595e25d4def04", 30); // sorcerer of claymorgue castle
	_md5Index.setVal("05cf6c64ecde5288ae2e46099bfd19a3", 31); // adventureland
	_md5Index.setVal("5af919881417920ec6a3961b4577f587", 32); // adventureland
	_md5Index.setVal("ccd3e3c805134b4fc36ad92e1cae623f", 33); // adventureland
	_md5Index.setVal("6c3de0b0ef39fad9d63e788de8cd972c", 34); // adventureland
	_md5Index.setVal("547036c586bfcd53e741ecfad74e3001", 35); // adventureland
	_md5Index.setVal("f0087b1f42ea9a0656462bf339278b08", 36); // savage island
	_md5Index.setVal("414d459ceb211230356ad823475866b3", 38); // savage island
	_md5Index.setVal("d80e133dd396565f773052cb317e8222", 40); // questprobe hulk
	_md5Index.setVal("5d0ea85ca1f260ca718a6bbb6da4cdb9", 41); // questprobe spiderman
	_md5Index.setVal("3d88539a6dd7e6e179bb61041125cc0f", 42); // questprobe spiderman
	_md5Index.setVal("f2711fe0376442f6f320da1b73b5b1a3", 43); // golden baton
	_md5Index.setVal("ed22cb234af638e7d9f570b937f9fc52", 11); // golden baton
	_md5Index.setVal("cc8e94d3fb665d5d23b728e9c1f262ba", 46); // time machine
	_md5Index.setVal("da044c4c57dc194002ba47f5b2921411", 47); // arrow of death 1
	_md5Index.setVal("3d40cb011167e7ae9f6695cdd1f4a1bf", 48); // arrow of death 2
	_md5Index.setVal("db68753e4c4c536693edca2f58747044", 49); // pulsar 7
	_md5Index.setVal("4f732cb27e2a0bb484443a7dd1974ccf", 50); // circus
	_md5Index.setVal("6c7ed2fd5f0247a55beb266344967662", 51); // feasibility
	_md5Index.setVal("94e4b070e45204b12d1655091c56045d", 53); // akyrz
	_md5Index.setVal("c423ad31ab3f6b45f0215c2e7fc3eb7e", 52); // akyrz
	_md5Index.setVal("96a1ccb3212808eee03e74cdc1f0d1a4", 54); // perseus
	_md5Index.setVal("79ee3669ccfff7338dfc1810228005dc", 55); // 10 indians
	_md5Index.setVal("facc11aa8b51e88a807236b765203eb0", 56); // waxworks11
}

Globals::~Globals() {
	delete _gameHeader;
}

} // End of namespace Scott
} // End of namespace Glk
