/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "saga/saga.h"

#include "saga/actor.h"
#include "saga/interface.h"
#include "saga/scene.h"
#include "saga/puzzle.h"
#include "saga/resnames.h"

#include "common/timer.h"

namespace Saga {

#define PUZZLE_X_OFFSET		72
#define PUZZLE_Y_OFFSET		46

#define PUZZLE_FIT			0x01   // 1 when in correct position
#define PUZZLE_MOVED		0x04   // 1 when somewhere in the box
#define PUZZLE_ALL_SET		PUZZLE_FIT | PUZZLE_MOVED

static Puzzle::PieceInfo pieceInfo[PUZZLE_PIECES] = {
	{268,  18,  0, 0,  0 + PUZZLE_X_OFFSET,   0 + PUZZLE_Y_OFFSET, 0, 3,
	 Point(0, 1),  Point(0, 62), Point(15, 31), Point(0, 0), Point(0, 0), Point(0,0)},
	{270,  52,  0, 0,  0 + PUZZLE_X_OFFSET,  32 + PUZZLE_Y_OFFSET, 0, 4,
	 Point(0, 31), Point(0, 47), Point(39, 47), Point(15, 1), Point(0, 0), Point(0, 0)},
	{19,  51,  0, 0,  0 + PUZZLE_X_OFFSET,   0 + PUZZLE_Y_OFFSET, 0, 4,
	 Point(0, 0), Point(23, 46), Point(39, 15), Point(31, 0), Point(0, 0), Point(0, 0)},
	{73,   0,  0, 0,   32 + PUZZLE_X_OFFSET,   0 + PUZZLE_Y_OFFSET, 0, 6,
	 Point(0, 0), Point(8, 16), Point(0, 31), Point(31, 31), Point(39, 15), Point(31, 0)},
	{0,  35,  0, 0,   64 + PUZZLE_X_OFFSET,  16 + PUZZLE_Y_OFFSET, 0, 4,
	 Point(0, 15), Point(15, 46), Point(23, 32), Point(7, 1), Point(0, 0), Point(0, 0)},
	{215,   0,  0, 0,   24 + PUZZLE_X_OFFSET,  32 + PUZZLE_Y_OFFSET, 0, 6,
	 Point(0, 15), Point(8, 31), Point(39, 31), Point(47, 16), Point(39, 0), Point(8, 0)},
	{159,   0,  0, 0,   32 + PUZZLE_X_OFFSET,  48 + PUZZLE_Y_OFFSET, 0, 5,
	 Point(0, 16), Point(8, 31), Point(55, 31), Point(39, 1), Point(32, 15), Point(0, 0)},
	{9,  70,  0, 0,   80 + PUZZLE_X_OFFSET,  32 + PUZZLE_Y_OFFSET, 0, 5,
	 Point(0, 31), Point(8, 47), Point(23, 47), Point(31, 31), Point(15, 1), Point(0, 0)},
	{288,  18,  0, 0,   96 + PUZZLE_X_OFFSET,   0 + PUZZLE_Y_OFFSET, 0, 4,
	 Point(0, 31), Point(15, 62), Point(31, 32), Point(15, 1), Point(0, 0), Point(0, 0)},
	{112,   0,  0, 0,  112 + PUZZLE_X_OFFSET,   0 + PUZZLE_Y_OFFSET, 0, 4,
	 Point(0, 0), Point(16, 31), Point(47, 31), Point(31, 0), Point(0, 0), Point(0, 0)},
	{27,  89,  0, 0,  104 + PUZZLE_X_OFFSET,  32 + PUZZLE_Y_OFFSET, 0, 4,
	 Point(0, 47), Point(31, 47), Point(31, 0), Point(24, 0), Point(0, 0), Point(0, 0)},
	{43,   0,  0, 0,  136 + PUZZLE_X_OFFSET,  32 + PUZZLE_Y_OFFSET, 0, 6,
	 Point(0, 0), Point(0, 47), Point(15, 47), Point(15, 15), Point(31, 15), Point(23, 0)},
	{0,   0,  0, 0,  144 + PUZZLE_X_OFFSET,   0 + PUZZLE_Y_OFFSET, 0, 4,
	 Point(0, 0), Point(24, 47), Point(39, 47), Point(39, 0), Point(0, 0), Point(0, 0)},
	{262,   0,  0, 0,   64 + PUZZLE_X_OFFSET,   0 + PUZZLE_Y_OFFSET, 0, 3,
	 Point(0, 0), Point(23, 46), Point(47, 0), Point(0, 0), Point(0, 0), Point(0, 0)},
	{271, 103,  0, 0,  152 + PUZZLE_X_OFFSET,  48 + PUZZLE_Y_OFFSET, 0, 4,
	 Point(0, 0), Point(0, 31), Point(31, 31), Point(31, 0), Point(0, 0), Point(0, 0)}
};

static Point pieceOrigins[PUZZLE_PIECES] = {
	Point(268,  18),
	Point(270,  51),
	Point( 19,  51),
	Point( 73,   0),
	Point(  0,  34),
	Point(215,   0),
	Point(159,   0),
	Point(  9,  69),
	Point(288,  18),
	Point(112,   0),
	Point( 27,  88),
	Point( 43,   0),
	Point(  0,   0),
	Point(262,   0),
	Point(271,  10)
};

const char *pieceNames[][PUZZLE_PIECES] = {
	{ "screwdriver", "pliers", "c-clamp", "wood clamp", "level",
	  "twine", "wood plane", "claw hammer", "tape measure", "hatchet",
	  "shears", "ruler", "saw", "mallet", "paint brush"
	},
	{ "Schraubendreher", "Zange", "Schraubzwinge", "Holzzwinge", "Wasserwaage",
	  "Bindfaden", "Hobel", "Schusterhammer", "Bandma$", "Beil",
	  "Schere", "Winkel", "S\204ge", "Hammer", "Pinsel"
	}
};

const char *hintStr[][4] = {
	{ "Check which pieces could fit in each corner first.",
	  "Check which corner has the least number of pieces that can fit and start from there.",
	  "Check each new corner and any new side for pieces that fit.",
	  "I don't see anything out of place."
	},
	{ "\232berpr\201fe zun\204chst, welche die Eckteile sein k\224nnten.",
	  "Schau, in welche Ecke die wenigsten Teile passen, und fang dort an.",
	  "Untersuche jede Ecke und jede Seite auf Teile, die dort passen k\224nnen.",
	  "Ich sehe nichts an der falschen Stelle."
	}
};

#define NUM_SOLICIT_REPLIES 5
const char *solicitStr[][NUM_SOLICIT_REPLIES] = {
	{ "Hey, Fox! Would you like a hint?",
	  "Would you like some help?",
	  "Umm...Umm...",
	  "Psst! want a hint?",
	  "I would have done this differently, you know."
	},
	{ "Hey, Fuchs! Brauchst Du \047nen Tip?",
	  "M\224chtest Du etwas Hilfe?"
	  "\231hm...\216hm..."
	  "Psst! \047n Tip vielleicht?"
	  "Ja, wei$t Du... ich h\204tte das anders gemacht."
	}
};

const char portraitList[] = {
	RID_ITE_JFERRET_SERIOUS,
	RID_ITE_JFERRET_GOOFY,
	RID_ITE_JFERRET_SERIOUS,
	RID_ITE_JFERRET_GOOFY,
	RID_ITE_JFERRET_ALOOF
};

#define NUM_SAKKA 3
const char *sakkaStr[][NUM_SAKKA] = {
	{ "Hey, you're not supposed to help the applicants!",
	  "Guys! This is supposed to be a test!",
	  "C'mon fellows, that's not in the rules!"
	},
	{ "Hey, Du darfst dem Pr\201fling nicht helfen!",
	  "Hallo?! Dies soll eine Pr\201fung sein!",
	  "Also, Jungs. Schummeln steht nicht in den Regeln!"
	}
};

#define NUM_WHINES 5
const char *whineStr[][NUM_WHINES] = {
	{ "Aww, c'mon Sakka!",
	  "One hint won't hurt, will it?",
	  "Sigh...",
	  "I think that clipboard has gone to your head, Sakka!",
	  "Well, I don't recall any specific rule against hinting."
	},
	{ "Och, sei nicht so, Sakka!"
	  "EIN Tip wird schon nicht schaden, oder?",
	  "Seufz..."
	  "Ich glaube, Du hast ein Brett vor dem Kopf, Sakka!",
	  "Hm, ich kann mich an keine Regel erinnern, die Tips verbietet."
	}
};

enum rifOptions {
	kROLater = 0,
	kROAccept = 1,
	kRODecline = 2,
	kROHint = 3
};

const char *optionsStr[][4] = {
	{ "\"I'll do this puzzle later.\"",
	  "\"Yes, I'd like a hint please.\"",
	  "\"No, thank you, I'd like to try and solve it myself.\"",
	  "I think the %s is in the wrong place."
	},
	{ "\"Ich l\224se das Puzzle sp\204ter.\"",
	  "\"Ja, ich m\224chte einen Tip, bitte.\"",
	  "\"Nein danke, ich m\224chte das alleine l\224sen.\"",
	  "Pssst... %s... falsche Stelle..."
	}
};

Puzzle::Puzzle(SagaEngine *vm) : _vm(vm), _solved(false), _active(false) {
	_lang = _vm->getFeatures() & GF_LANG_DE ? 1 : 0;

	_hintRqState = kRQNoHint;
	_hintOffer = 0;
	_hintCount = 0;
	_helpCount = 0;
	_puzzlePiece = -1;
}

void Puzzle::execute(void) {
	_active = true;
	Common::g_timer->installTimerProc(&hintTimerCallback, kPuzzleHintTime, this);

	initPieces();

	_vm->_interface->setMode(kPanelConverse);
	clearHint();
	//_solved = true; // Cheat
	//exitPuzzle();
}

void Puzzle::exitPuzzle(void) {
	_active = false;

	Common::g_timer->removeTimerProc(&hintTimerCallback);

	_vm->_scene->changeScene(ITE_SCENE_LODGE, 0, kTransitionNoFade);
	_vm->_interface->setMode(kPanelMain);
}

void Puzzle::initPieces(void) {
	//	ActorData *puzzle = _vm->_actor->getActor(RID_ITE_ACTOR_PUZZLE);
}

void Puzzle::movePiece(Point mousePt) {
}

void Puzzle::hintTimerCallback(void *refCon) {                                   
        ((Puzzle *)refCon)->solicitHint();                                         
}                                                                               

void Puzzle::solicitHint(void) {
	int i;

	_vm->_actor->setSpeechColor(1, kITEColorBlack);

	Common::g_timer->removeTimerProc(&hintTimerCallback);

	switch(_hintRqState) {
	case kRQNoHint:
		//	Pick a random hint request.
		i = _hintOffer++;
		if (_hintOffer >= NUM_SOLICIT_REPLIES)
			_hintOffer = 0;

		_vm->_actor->nonActorSpeech(&solicitStr[_lang][i], 1, 0);

		//	Determine which of the journeymen will offer then
		//	hint, and then show that character's portrait.
		_hintGiver = portraitList[i];
		_vm->_interface->setRightPortrait(_hintGiver);

		//	Add Rif's reply to the list.
		clearHint();

		//	Roll to see if Sakka scolds
		if (_vm->_rnd.getRandomNumber(1)) {
			_hintRqState = kRQSakkaDenies;
			Common::g_timer->installTimerProc(&hintTimerCallback, 200, this);
		} else {
			_hintRqState = kRQHintRequested;
			Common::g_timer->installTimerProc(&hintTimerCallback, 400, this);
		}

		break;

	case kRQHintRequested:
		i = _vm->_rnd.getRandomNumber(NUM_SAKKA - 1);
		_vm->_actor->nonActorSpeech(&sakkaStr[_lang][i], 1, 0);

		_vm->_interface->setRightPortrait(RID_ITE_SAKKA_APPRAISING);

		_hintRqState = kRQHintRequestedStage2;
		Common::g_timer->installTimerProc(&hintTimerCallback, 400, this);

		_vm->_interface->converseClear();
		_vm->_interface->converseAddText(optionsStr[_lang][kROAccept], 1, 0, 0 );
		_vm->_interface->converseAddText(optionsStr[_lang][kRODecline], 2, 0, 0 );
		_vm->_interface->converseAddText(optionsStr[_lang][kROLater], 0, 0, 0 );
		_vm->_interface->converseDisplayText();
		break;

	case kRQHintRequestedStage2:
		if (_vm->_rnd.getRandomNumber(1)) {			//	Skip Reply part
			i = _vm->_rnd.getRandomNumber(NUM_WHINES - 1);
			_vm->_actor->nonActorSpeech(&whineStr[_lang][i], 1, 0);
		}

		_vm->_interface->setRightPortrait(_hintGiver);

		_hintRqState = kRQSakkaDenies;
		break;

	case kRQSakkaDenies:
		_vm->_interface->converseClear();
		_vm->_interface->converseAddText(optionsStr[_lang][kROAccept], 1, 0, 0);
		_vm->_interface->converseAddText(optionsStr[_lang][kRODecline], 2, 0, 0);
		_vm->_interface->converseAddText(optionsStr[_lang][kROLater], 0, 0, 0);
		_vm->_interface->converseDisplayText();

		Common::g_timer->installTimerProc(&hintTimerCallback, kPuzzleHintTime, this);

		_hintRqState = kRQSkipEverything;
		break;

	default:
		break;
	}
}

void Puzzle::handleReply(int reply) {
	switch(reply) {
	case 0:		// Quit the puzzle
		exitPuzzle();
		break;

	case 1:		// Accept the hint
		giveHint();
		break;

	case 2:		// Decline the hint
		_vm->_actor->abortSpeech();
		_hintRqState = kRQNoHint;
		Common::g_timer->removeTimerProc(&hintTimerCallback);
		Common::g_timer->installTimerProc(&hintTimerCallback, kPuzzleHintTime * 2, this);
		clearHint();
		break;
	}
}

void Puzzle::giveHint(void) {
	int i, total = 0;

	_vm->_interface->converseClear();

	_vm->_actor->abortSpeech();
	_vm->_interface->setRightPortrait(_hintGiver);

	for (i = 0; i < PUZZLE_PIECES; i++)
		total += pieceInfo[i].flag & PUZZLE_FIT;

	if (_hintCount == 0 && (pieceInfo[1].flag & PUZZLE_FIT
			|| pieceInfo[12].flag & PUZZLE_FIT))
		_hintCount++;
	if (_hintCount == 1 && pieceInfo[14].flag & PUZZLE_FIT)
		_hintCount++;

	if (_hintCount == 2 && total > 3)
		_hintCount++;

	_vm->_actor->setSpeechColor(1, kITEColorBlack);

	if (_hintCount < 3) {
		_vm->_actor->nonActorSpeech(&hintStr[_lang][_hintCount], 1, 0 );
	} else {
		int piece = 0;

		for (i = PUZZLE_PIECES - 1; i >= 0; i--) {
			piece = _piecePriority[i];
			if (pieceInfo[piece].flag & PUZZLE_MOVED
					&& !(pieceInfo[piece].flag & PUZZLE_FIT)) {
				if (_helpCount < 12)
					_helpCount++;
				break;
			}
		}

		if (i >= 0) {
			char hintBuf[64];
			const char *hintPtr = hintBuf;
			sprintf(hintBuf, optionsStr[_lang][kROHint], pieceNames[piece]);

			_vm->_actor->nonActorSpeech(&hintPtr, 1, 0);
		}
		else {
				//	If no pieces are in the wrong place
			_vm->_actor->nonActorSpeech(&hintStr[_lang][3], 1, 0);
		}
	}
	_hintCount++;

	_hintRqState = kRQNoHint;

	_vm->_interface->converseAddText(optionsStr[_lang][kROLater], 0, 0, 0);
	_vm->_interface->converseDisplayText();

	Common::g_timer->removeTimerProc(&hintTimerCallback);
	Common::g_timer->installTimerProc(&hintTimerCallback, kPuzzleHintTime, this);
}

void Puzzle::clearHint(void) {
	_vm->_interface->converseClear();
	_vm->_interface->converseAddText(optionsStr[_lang][kROLater], 0, 0, 0);
	_vm->_interface->converseDisplayText();
	_vm->_interface->setStatusText(" ");
}

} // End of namespace Saga
