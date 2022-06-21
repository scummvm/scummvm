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

#include "glk/scott/scott.h"
#include "glk/scott/command_parser.h"
#include "glk/scott/saga_draw.h"
#include "glk/scott/globals.h"

namespace Glk {
namespace Scott {

#define GREMLINS_ANIMATION_RATE 670

void updateGremlinsAnimations(void) {
	if (_G(_rooms)[MY_LOC]._image == 255) {
		g_scott->glk_request_timer_events(0);
		return;
	}
	g_scott->openGraphicsWindow();
	if (_G(_graphics) == nullptr) {
		g_scott->glk_request_timer_events(0);
		return;
	}

	int timer_delay = GREMLINS_ANIMATION_RATE;
	switch (MY_LOC) {
	case 1:                          /* Bedroom */
		if (_G(_items)[50]._location == 1) /* Gremlin throwing darts */
		{
			if (_G(_animationFlag)) {
				g_scott->drawImage(60); /* Gremlin throwing dart frame 1 */
			} else {
				g_scott->drawImage(59); /* Gremlin throwing dart frame 2 */
			}
		}
		break;
	case 17:                          /* Dotty's Tavern */
		if (_G(_items)[82]._location == 17) /* Gang of GREMLINS */
		{
			if (_G(_animationFlag)) {
				g_scott->drawImage(49); /* Gremlin hanging from curtains frame 1 */
				g_scott->drawImage(51); /* Gremlin ear frame 1 */
				g_scott->drawImage(54); /* Gremlin's mouth frame 1 */
			} else {
				g_scott->drawImage(50); /* Gremlin hanging from curtains frame 2 */
				g_scott->drawImage(52); /* Gremlin ear frame 2 */
				g_scott->drawImage(53); /* Gremlin's mouth frame 2 */
			}
		}
		break;
	case 16:                          /* Behind a Bar */
		if (_G(_items)[82]._location == 16) /* Gang of GREMLINS */
		{
			if (_G(_animationFlag)) {
				g_scott->drawImage(57); /* Flasher gremlin frame 1 */
				g_scott->drawImage(24); /* Gremlin tongue frame 1 */
				if (CURRENT_GAME == GREMLINS_GERMAN)
					g_scott->drawImage(46); /* Gremlin ear frame 1 */
				else
					g_scott->drawImage(73); /* Gremlin ear frame 1 */
			} else {
				g_scott->drawImage(58); /* Flasher gremlin frame 2 */

				if (CURRENT_GAME == GREMLINS_GERMAN) {
					g_scott->drawImage(33); /* Gremlin tongue frame 2 */
					g_scott->drawImage(23); /* Gremlin ear frame 2 */
				} else {
					g_scott->drawImage(72); /* Gremlin tongue frame 2 */
					if (CURRENT_GAME == GREMLINS_SPANISH)
						g_scott->drawImage(23); /* Gremlin ear frame 2 */
					else
						g_scott->drawImage(74); /* Gremlin ear frame 2 */
				}
			}
		}
		break;
	case 19:                          /* Square */
		if (_G(_items)[82]._location == 19) /* Gang of GREMLINS */
		{
			if (_G(_animationFlag)) {
				g_scott->drawImage(55); /* Silhouette of Gremlins frame 1 */
			} else {
				g_scott->drawImage(71); /* Silhouette of Gremlins frame 1 */
			}
		}
		break;
	case 6:                          /* on a road */
		if (_G(_items)[82]._location == 6) /* Gang of GREMLINS */
		{
			if (_G(_animationFlag)) {
				if ((_G(_game)->_subType & (LOCALIZED | C64)) == LOCALIZED) {
					g_scott->drawImage(25); /* Silhouette 2 of Gremlins  */
				} else {
					g_scott->drawImage(75); /* Silhouette 2 of Gremlins  */
				}
			} else {
				g_scott->drawImage(48); /* Silhouette 2 of Gremlins flipped */
			}
		}
		break;
	case 3:                   /* Kitchen */
		if (_G(_counters)[2] == 2) /* Blender is on */
		{
			if (_G(_animationFlag)) {
				g_scott->drawImage(56); /* Blended Gremlin */
			} else {
				g_scott->drawImage(12); /* Blended Gremlin flipped */
			}
		}
		break;
	default:
		timer_delay = 0;
		break;
	}
	_G(_animationFlag) = (_G(_animationFlag) == 0);
	g_scott->glk_request_timer_events(timer_delay);
}

void gremlinsLook(void) {
	if (_G(_rooms)[MY_LOC]._image != 255) {
		if (MY_LOC == 17 && _G(_items)[82]._location == 17)
			g_scott->drawImage(45); /* Bar full of Gremlins */
		else
			g_scott->drawImage(_G(_rooms)[MY_LOC]._image);
		_G(_animationFlag) = 0;
		updateGremlinsAnimations();
	}
	/* Ladder image at the top of the department store */
	if (MY_LOC == 34 && _G(_items)[53]._location == MY_LOC) {
		g_scott->drawImage(42);
	} else if (MY_LOC == 10 && _G(_items)[15]._location == 0) {
		if (_G(_items)[99]._location == MY_LOC && CURRENT_GAME == GREMLINS_GERMAN_C64)
			g_scott->drawImage(90); /* Dazed Stripe */
		g_scott->drawImage(82);     /* Empty pool with puddle */
		/* Draw puddle on top of Stripe */
		/* Doesn't look great, but better than the other way round */
	}
}

void fillInGermanSystemMessages() {
	_G(_sys)[I_DONT_KNOW_HOW_TO] = "Ich weiss nicht, wie man etwas \"";
	_G(_sys)[SOMETHING] = "\" macht. ";
	_G(_sys)[I_DONT_KNOW_WHAT_A] = "\"";
	_G(_sys)[IS] = "\" kenne ich nicht. ";
	_G(_sys)[YES] = "Ja";
	_G(_sys)[NO] = "Nein";
	_G(_sys)[ANSWER_YES_OR_NO] = "Antworte Ja oder Nein.\n";
	_G(_sys)[I_DONT_UNDERSTAND] = "Ich verstehe nicht. ";
	_G(_sys)[ARE_YOU_SURE] = "Sind Sie sicher? ";
	_G(_sys)[NOTHING_HERE_TO_TAKE] = "Hier gibt es nichts zu nehmen. ";
	_G(_sys)[YOU_HAVE_NOTHING] = "Ich traege nichts. ";
	_G(_sys)[MOVE_UNDONE] = "Verschieben rueckgaengig gemacht. ";
	_G(_sys)[CANT_UNDO_ON_FIRST_TURN] = "Sie koennen die erste Runde nicht rueckgaengig machen. ";
	_G(_sys)[NO_UNDO_STATES] = "Keine rueckgaengig-Zustaende mehr gespeichert. ";
	_G(_sys)[SAVED] = "Spiel gespeichert. ";
	_G(_sys)[CANT_USE_ALL] = "Sie koennen ALLES nicht mit diesem Verb verwenden. ";
	_G(_sys)[TRANSCRIPT_ON] = "Das Transkript ist jetzt eingeschaltet. ";
	_G(_sys)[TRANSCRIPT_OFF] = "Das Transkript ist jetzt deaktiviert. ";
	_G(_sys)[NO_TRANSCRIPT] = "Es wird kein Transkript ausgefuehrt. ";
	_G(_sys)[TRANSCRIPT_ALREADY] = "Eine Transkript laeuft bereits. ";
	_G(_sys)[FAILED_TRANSCRIPT] = "Transkriptdatei konnte nicht erstellt werden. ";
	_G(_sys)[TRANSCRIPT_START] = "Beginn einer Transkript.\n\n";
	_G(_sys)[TRANSCRIPT_END] = "\n\nEnde eniner Transkript.\n";
	_G(_sys)[BAD_DATA] = "SCHLECHTE DATEN! Ungueltige Speicherdatei.\n";
	_G(_sys)[STATE_SAVED] = "Zustand speichern.\n";
	_G(_sys)[NO_SAVED_STATE] = "Es ist kein gespeicherter Zustand vorhanden.\n";
	_G(_sys)[STATE_RESTORED] = "Zustand wiederhergestellt.\n";

	_G(_sys)[YOU_ARE] = "Ich bin ";
	_G(_sys)[WHAT] = _G(_sys)[HUH];

	for (int i = 0; i < NUMBER_OF_DIRECTIONS; i++)
		_G(_directions)[i] = _G(_germanDirections)[i];
	for (int i = 0; i < NUMBER_OF_SKIPPABLE_WORDS; i++)
		_G(_skipList)[i] = _G(_germanSkipList)[i];
	for (int i = 0; i < NUMBER_OF_DELIMITERS; i++)
		_G(_delimiterList)[i] = _G(_germanDelimiterList)[i];
	for (int i = 0; i < NUMBER_OF_EXTRA_COMMANDS; i++)
		_G(_extraCommands)[i] = _G(_germanExtraCommands)[i];
	for (int i = 0; i < NUMBER_OF_EXTRA_NOUNS; i++)
		_G(_extraNouns)[i] = _G(_germanExtraNouns)[i];
}

void loadExtraGermanGremlinsc64Data() {
	_G(_verbs)[0] = "AUTO\0";
	_G(_nouns)[0] = "ANY\0";
	_G(_nouns)[28] = "*Y.M.C\0";

	// These are broken in some versions
	_G(_actions)[0]._condition[0] = 1005;
	_G(_actions)[6]._vocab = 100;

	_G(_gameHeader)->_numActions = 236;

	SysMessageType messagekey[] = {
		NORTH,
		SOUTH,
		EAST,
		WEST,
		UP,
		DOWN,
		EXITS,
		YOU_SEE,
		YOU_ARE,
		YOU_CANT_GO_THAT_WAY,
		OK,
		WHAT_NOW,
		HUH,
		YOU_HAVE_IT,
		TAKEN,
		DROPPED,
		YOU_HAVENT_GOT_IT,
		INVENTORY,
		YOU_DONT_SEE_IT,
		THATS_BEYOND_MY_POWER,
		DIRECTION,
		YOURE_CARRYING_TOO_MUCH,
		IM_DEAD,
		RESUME_A_SAVED_GAME,
		PLAY_AGAIN,
		YOU_CANT_DO_THAT_YET,
		I_DONT_UNDERSTAND,
		NOTHING};

	for (int i = 0; i < 28; i++) {
		_G(_sys)[messagekey[i]] = _G(_systemMessages)[i];
	}

	_G(_sys)[HIT_ENTER] = _G(_systemMessages)[30];

	fillInGermanSystemMessages();

	_G(_items)[99]._image = 255;
}

void loadExtraGermanGremlinsData() {
	_G(_verbs)[0] = "AUTO\0";
	_G(_nouns)[0] = "ANY\0";
	_G(_nouns)[28] = "*Y.M.C\0";

	_G(_messages)[90] = "Ehe ich etwas anderes mache, much aich erst alles andere fallenlassen. ";
	fillInGermanSystemMessages();
}

void loadExtraSpanishGremlinsData() {
	_G(_verbs)[0] = "AUTO\0";
	_G(_nouns)[0] = "ANY\0";

	for (int i = YOU_ARE; i <= HIT_ENTER; i++)
		_G(_sys)[i] = _G(_systemMessages)[15 - YOU_ARE + i];
	for (int i = I_DONT_UNDERSTAND; i <= THATS_BEYOND_MY_POWER; i++)
		_G(_sys)[i] = _G(_systemMessages)[6 - I_DONT_UNDERSTAND + i];

	for (int i = DROPPED; i <= OK; i++)
		_G(_sys)[i] = _G(_systemMessages)[2 - DROPPED + i];
	_G(_sys)[PLAY_AGAIN] = _G(_systemMessages)[5];
	_G(_sys)[YOURE_CARRYING_TOO_MUCH] = _G(_systemMessages)[22];
	_G(_sys)[IM_DEAD] = _G(_systemMessages)[23];
	_G(_sys)[YOU_CANT_GO_THAT_WAY] = _G(_systemMessages)[14];
	_G(_sys)[WHAT] = _G(_sys)[HUH];
	_G(_sys)[YES] = "s}";
	_G(_sys)[NO] = "no";
	_G(_sys)[ANSWER_YES_OR_NO] = "Contesta s} o no.\n";
	_G(_sys)[I_DONT_KNOW_WHAT_A] = "No s\x84 qu\x84 es un \"";
	_G(_sys)[IS] = "\". ";
	_G(_sys)[I_DONT_KNOW_HOW_TO] = "No s\x84 c|mo \"";
	_G(_sys)[SOMETHING] = "\" algo. ";

	_G(_sys)[ARE_YOU_SURE] = "\x83\x45stas segura? ";
	_G(_sys)[NOTHING_HERE_TO_TAKE] = "No hay nada aqu} para tomar. ";
	_G(_sys)[YOU_HAVE_NOTHING] = "No llevo nada. ";
	_G(_sys)[MOVE_UNDONE] = "Deshacer. ";
	_G(_sys)[CANT_UNDO_ON_FIRST_TURN] = "No se puede deshacer en el primer turno. ";
	_G(_sys)[NO_UNDO_STATES] = "No hay m{s estados de deshacer disponibles. ";
	_G(_sys)[SAVED] = "Juego guardado. ";
	_G(_sys)[CANT_USE_ALL] = "No puedes usar TODO con este verbo. ";
	_G(_sys)[TRANSCRIPT_ON] = "Transcripci|n en. ";
	_G(_sys)[TRANSCRIPT_OFF] = "Transcripci|n desactivada. ";
	_G(_sys)[NO_TRANSCRIPT] = "No se est{ ejecutando ninguna transcripci|n. ";
	_G(_sys)[TRANSCRIPT_ALREADY] = "Ya se est{ ejecutando una transcripci|n. ";
	_G(_sys)[FAILED_TRANSCRIPT] = "No se pudo crear el archivo de transcripci|n. ";
	_G(_sys)[TRANSCRIPT_START] = "Comienzo de una transcripci|n.\n\n";
	_G(_sys)[TRANSCRIPT_END] = "\n\nFin de una transcripci|n.\n";
	_G(_sys)[BAD_DATA] = "\x80MALOS DATOS! Guardar archivo no v{lido.\n";
	_G(_sys)[STATE_SAVED] = "Estado guardado.\n";
	_G(_sys)[NO_SAVED_STATE] = "No existe ning\x85n estado guardado.\n";
	_G(_sys)[STATE_RESTORED] = "Estado restaurado.\n";

	for (int i = 0; i < NUMBER_OF_DIRECTIONS; i++)
		_G(_directions)[i] = _G(_spanishDirections)[i];
	for (int i = 0; i < NUMBER_OF_EXTRA_NOUNS; i++)
		_G(_extraNouns)[i] = _G(_spanishExtraNouns)[i];
	for (int i = 0; i < NUMBER_OF_EXTRA_COMMANDS; i++)
		_G(_extraCommands)[i] = _G(_spanishExtraCommands)[i];
}

void gremlinsAction(int parameter) {
	g_scott->drawImage(68); /* Mogwai */
	g_scott->display(_G(_bottomWindow), "\n%s\n", _G(_sys)[HIT_ENTER].c_str());
	g_scott->hitEnter();
	g_scott->look();
}


} // End of namespace Scott
} // End of namespace Glk
