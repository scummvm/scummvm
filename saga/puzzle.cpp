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
#include "saga/sprite.h"
#include "saga/puzzle.h"
#include "saga/render.h"
#include "saga/resnames.h"

#include "common/system.h"
#include "common/timer.h"

namespace Saga {

#define PUZZLE_X_OFFSET		72
#define PUZZLE_Y_OFFSET		46

#define PUZZLE_FIT			0x01   // 1 when in correct position
#define PUZZLE_MOVED		0x04   // 1 when somewhere in the box
#define PUZZLE_ALL_SET		PUZZLE_FIT | PUZZLE_MOVED

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
	Point(271, 103)
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
	_lang = (_vm->getLanguage() == Common::DE_DEU) ? 1 : 0;

	_hintRqState = kRQNoHint;
	_hintOffer = 0;
	_hintCount = 0;
	_helpCount = 0;
	_puzzlePiece = -1;
	_newPuzzle = true;
	_sliding = false;
	_hintBox.left = 70;
	_hintBox.top = 105;
	_hintBox.setWidth(240);
	_hintBox.setHeight(30);

	initPieceInfo( 0, 268,  18,  0, 0,  0 + PUZZLE_X_OFFSET,   0 + PUZZLE_Y_OFFSET, 0, 3,
		  Point(0, 1),  Point(0, 62), Point(15, 31), Point(0, 0), Point(0, 0), Point(0,0));
	initPieceInfo( 1, 270,  52,  0, 0,  0 + PUZZLE_X_OFFSET,  32 + PUZZLE_Y_OFFSET, 0, 4,
		  Point(0, 31), Point(0, 47), Point(39, 47), Point(15, 1), Point(0, 0), Point(0, 0));
	initPieceInfo( 2, 19,  51,  0, 0,  0 + PUZZLE_X_OFFSET,   0 + PUZZLE_Y_OFFSET, 0, 4,
		  Point(0, 0), Point(23, 46), Point(39, 15), Point(31, 0), Point(0, 0), Point(0, 0));
	initPieceInfo( 3, 73,   0,  0, 0,   32 + PUZZLE_X_OFFSET,   0 + PUZZLE_Y_OFFSET, 0, 6,
		  Point(0, 0), Point(8, 16), Point(0, 31), Point(31, 31), Point(39, 15), Point(31, 0));
	initPieceInfo( 4, 0,  35,  0, 0,   64 + PUZZLE_X_OFFSET,  16 + PUZZLE_Y_OFFSET, 0, 4,
		  Point(0, 15), Point(15, 46), Point(23, 32), Point(7, 1), Point(0, 0), Point(0, 0));
	initPieceInfo( 5, 215,   0,  0, 0,   24 + PUZZLE_X_OFFSET,  32 + PUZZLE_Y_OFFSET, 0, 6,
		  Point(0, 15), Point(8, 31), Point(39, 31), Point(47, 16), Point(39, 0), Point(8, 0));
	initPieceInfo( 6, 159,   0,  0, 0,   32 + PUZZLE_X_OFFSET,  48 + PUZZLE_Y_OFFSET, 0, 5,
		  Point(0, 16), Point(8, 31), Point(55, 31), Point(39, 1), Point(32, 15), Point(0, 0));
	initPieceInfo( 7, 9,  70,  0, 0,   80 + PUZZLE_X_OFFSET,  32 + PUZZLE_Y_OFFSET, 0, 5,
		  Point(0, 31), Point(8, 47), Point(23, 47), Point(31, 31), Point(15, 1), Point(0, 0));
	initPieceInfo( 8, 288,  18,  0, 0,   96 + PUZZLE_X_OFFSET,   0 + PUZZLE_Y_OFFSET, 0, 4,
		  Point(0, 31), Point(15, 62), Point(31, 32), Point(15, 1), Point(0, 0), Point(0, 0));
	initPieceInfo( 9, 112,   0,  0, 0,  112 + PUZZLE_X_OFFSET,   0 + PUZZLE_Y_OFFSET, 0, 4,
		  Point(0, 0), Point(16, 31), Point(47, 31), Point(31, 0), Point(0, 0), Point(0, 0));
	initPieceInfo(10, 27,  89,  0, 0,  104 + PUZZLE_X_OFFSET,  32 + PUZZLE_Y_OFFSET, 0, 4,
		  Point(0, 47), Point(31, 47), Point(31, 0), Point(24, 0), Point(0, 0), Point(0, 0));
	initPieceInfo(11, 43,   0,  0, 0,  136 + PUZZLE_X_OFFSET,  32 + PUZZLE_Y_OFFSET, 0, 6,
		  Point(0, 0), Point(0, 47), Point(15, 47), Point(15, 15), Point(31, 15), Point(23, 0));
	initPieceInfo(12, 0,   0,  0, 0,  144 + PUZZLE_X_OFFSET,   0 + PUZZLE_Y_OFFSET, 0, 4,
		  Point(0, 0), Point(24, 47), Point(39, 47), Point(39, 0), Point(0, 0), Point(0, 0));
	initPieceInfo(13, 262,   0,  0, 0,   64 + PUZZLE_X_OFFSET,   0 + PUZZLE_Y_OFFSET, 0, 3,
		  Point(0, 0), Point(23, 46), Point(47, 0), Point(0, 0), Point(0, 0), Point(0, 0));
	initPieceInfo(14, 271, 103,  0, 0,  152 + PUZZLE_X_OFFSET,  48 + PUZZLE_Y_OFFSET, 0, 4,
		  Point(0, 0), Point(0, 31), Point(31, 31), Point(31, 0), Point(0, 0), Point(0, 0));
}

void Puzzle::initPieceInfo(int i, int16 curX, int16 curY, byte offX, byte offY, int16 trgX,
						   int16 trgY, uint8 flag, uint8 count, Point point0, Point point1,
						   Point point2, Point point3, Point point4, Point point5) {
	_pieceInfo[i].curX = curX;
	_pieceInfo[i].curY = curY;
	_pieceInfo[i].offX = offX;
	_pieceInfo[i].offY = offY;
	_pieceInfo[i].trgX = trgX;
	_pieceInfo[i].trgY = trgY;
	_pieceInfo[i].flag = flag;
	_pieceInfo[i].count = count;
	_pieceInfo[i].point[0] = point0;
	_pieceInfo[i].point[1] = point1;
	_pieceInfo[i].point[2] = point2;
	_pieceInfo[i].point[3] = point3;
	_pieceInfo[i].point[4] = point4;
	_pieceInfo[i].point[5] = point5;
}


void Puzzle::execute(void) {
	_active = true;
	Common::g_timer->installTimerProc(&hintTimerCallback, kPuzzleHintTime, this);

	initPieces();

	showPieces();

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
	ActorData *puzzle = _vm->_actor->getActor(_vm->_actor->actorIndexToId(ITE_ACTOR_PUZZLE));
	SpriteInfo *spI;

	for (int i = 0; i < PUZZLE_PIECES; i++) {
		spI = &puzzle->_spriteList.infoList[i];
		_pieceInfo[i].offX = (byte)(spI->width >> 1);
		_pieceInfo[i].offY = (byte)(spI->height >> 1);

		if (_newPuzzle) {
			_pieceInfo[i].curX = pieceOrigins[i].x;
			_pieceInfo[i].curY = pieceOrigins[i].y;
		}
		_piecePriority[i] = i;
	}
	_newPuzzle = false;
}

void Puzzle::showPieces(void) {
	ActorData *puzzle = _vm->_actor->getActor(_vm->_actor->actorIndexToId(ITE_ACTOR_PUZZLE));
	Surface *backBuffer = _vm->_gfx->getBackBuffer();

	for (int j = PUZZLE_PIECES - 1 ; j >= 0; j--) {
		int num = _piecePriority[j];

		if (_puzzlePiece != num) {
			_vm->_sprite->draw(backBuffer, _vm->getDisplayClip(), puzzle->_spriteList, num, Point(_pieceInfo[num].curX, _pieceInfo[num].curY), 256);
		}
	}
}

void Puzzle::drawCurrentPiece() {
	ActorData *puzzle = _vm->_actor->getActor(_vm->_actor->actorIndexToId(ITE_ACTOR_PUZZLE));
	Surface *backBuffer = _vm->_gfx->getBackBuffer();

	_vm->_sprite->draw(backBuffer, _vm->_scene->getSceneClip(), puzzle->_spriteList, _puzzlePiece,
			   Point(_pieceInfo[_puzzlePiece].curX, _pieceInfo[_puzzlePiece].curY), 256);
}

void Puzzle::movePiece(Point mousePt) {
	int newx, newy;

	showPieces();

	if (_puzzlePiece == -1)
		return;

	if (_sliding) {
		newx = _slidePointX;
		newy = _slidePointY;
	} else {
		if (mousePt.y >= 137)
			return;

		newx = mousePt.x;
		newy = mousePt.y;
	}

	newx -= _pieceInfo[_puzzlePiece].offX;
	newy -= _pieceInfo[_puzzlePiece].offY;

	_pieceInfo[_puzzlePiece].curX = newx;
	_pieceInfo[_puzzlePiece].curY = newy;

	drawCurrentPiece();
}

void Puzzle::handleClick(Point mousePt) {
	if (_puzzlePiece != -1) {
		dropPiece(mousePt);

		if (!_active)
			return; // we won

		drawCurrentPiece();
		_puzzlePiece = -1;

		return;
	}

	for (int j = 0; j < PUZZLE_PIECES; j++)	{
		int i = _piecePriority[j];
		int adjX = mousePt.x - _pieceInfo[i].curX;
		int adjY = mousePt.y - _pieceInfo[i].curY;

		if (hitTestPoly(&_pieceInfo[i].point[0], _pieceInfo[i].count, Point(adjX, adjY))) {
			_puzzlePiece = i;
			break;
		}
	}

	if (_puzzlePiece == -1)
		return;

	alterPiecePriority();

	// Display scene background
	_vm->_scene->draw();
	showPieces();

	int newx = mousePt.x - _pieceInfo[_puzzlePiece].offX;
	int newy = mousePt.y - _pieceInfo[_puzzlePiece].offY;

	if (newx != _pieceInfo[_puzzlePiece].curX
		|| newy != _pieceInfo[_puzzlePiece].curY) {
		_pieceInfo[_puzzlePiece].curX = newx;
		_pieceInfo[_puzzlePiece].curY = newy;
	}
	_vm->_interface->setStatusText(pieceNames[_lang][_puzzlePiece]);
}

void Puzzle::alterPiecePriority(void) {
	for (int i = 1; i < PUZZLE_PIECES; i++) {
		if (_puzzlePiece == _piecePriority[i]) {
			for (int j = i - 1; j >= 0; j--)
				_piecePriority[j+1] = _piecePriority[j];
			_piecePriority[0] = _puzzlePiece;
			break;
		}
	}
}

void Puzzle::slidePiece(int x1, int y1, int x2, int y2) {
	int count;
	Point slidePoints[320];

	x1 += _pieceInfo[_puzzlePiece].offX;
	y1 += _pieceInfo[_puzzlePiece].offY;

	count = pathLine(&slidePoints[0], Point(x1, y1),
		 Point(x2 + _pieceInfo[_puzzlePiece].offX, y2 + _pieceInfo[_puzzlePiece].offY));

	if (count > 1) {
		int factor = count / 4;
		_sliding = true;

		if (!factor)
			factor++;

		for (int i = 1; i < count; i += factor) {
			_slidePointX = slidePoints[i].x;
			_slidePointY = slidePoints[i].y;
			_vm->_render->drawScene();
			_vm->_system->delayMillis(10);
		}
		_sliding = false;
	}

	_pieceInfo[_puzzlePiece].curX = x2;
	_pieceInfo[_puzzlePiece].curY = y2;
}

void Puzzle::dropPiece(Point mousePt) {
	int boxx = PUZZLE_X_OFFSET;
	int boxy = PUZZLE_Y_OFFSET;
	int boxw = boxx + 184;
	int boxh = boxy + 80;

	// if the center is within the box quantize within
	// else move it back to its original start point
	if (mousePt.x >= boxx && mousePt.x < boxw && mousePt.y >= boxy && mousePt.y <= boxh) {
		ActorData *puzzle = _vm->_actor->getActor(_vm->_actor->actorIndexToId(ITE_ACTOR_PUZZLE));
		SpriteInfo *spI;
		int newx = mousePt.x - _pieceInfo[_puzzlePiece].offX;
		int newy = mousePt.y - _pieceInfo[_puzzlePiece].offY;

		if (newx < boxx)
			newx = PUZZLE_X_OFFSET;
		if (newy < boxy)
			newy = PUZZLE_Y_OFFSET;

		spI = &puzzle->_spriteList.infoList[_puzzlePiece];

		if (newx + spI->width > boxw)
			newx = boxw - spI->width ;
		if (newy + spI->height > boxh)
			newy = boxh - spI->height ;

		int x1 = ((newx - PUZZLE_X_OFFSET) & ~7) + PUZZLE_X_OFFSET;
		int y1 = ((newy - PUZZLE_Y_OFFSET) & ~7) + PUZZLE_Y_OFFSET;
		int x2 = x1 + 8;
		int y2 = y1 + 8;
		newx = (x2 - newx < newx - x1) ? x2 : x1;
		newy = (y2 - newy < newy - y1) ? y2 : y1;

		// if any part of the puzzle piece falls outside the box
		// force it back in

		// is the piece at the target location
		if (newx == _pieceInfo[_puzzlePiece].trgX
				&& newy == _pieceInfo[_puzzlePiece].trgY) {
			_pieceInfo[_puzzlePiece].flag |= (PUZZLE_MOVED | PUZZLE_FIT);
		} else {
			_pieceInfo[_puzzlePiece].flag &= ~PUZZLE_FIT;
			_pieceInfo[_puzzlePiece].flag |= PUZZLE_MOVED;
		}
		_pieceInfo[_puzzlePiece].curX = newx;
		_pieceInfo[_puzzlePiece].curY = newy;
	} else {
		int newx = pieceOrigins[_puzzlePiece].x;
		int newy = pieceOrigins[_puzzlePiece].y;
		_pieceInfo[_puzzlePiece].flag &= ~(PUZZLE_FIT | PUZZLE_MOVED);

		// slide piece from current position to new position
		slidePiece(_pieceInfo[_puzzlePiece].curX, _pieceInfo[_puzzlePiece].curY,
					newx, newy);
	}

	// is the puzzle completed?

	_solved = true;
	for (int i = 0; i < PUZZLE_PIECES; i++)
		if ((_pieceInfo[i].flag & PUZZLE_FIT) == 0)	{
			_solved = false;
			break;
		}

	if (_solved)
		exitPuzzle();
}

void Puzzle::hintTimerCallback(void *refCon) {
	((Puzzle *)refCon)->solicitHint();
}

void Puzzle::solicitHint(void) {
	int i;

	_vm->_actor->setSpeechColor(1, kITEColorBlack);

	Common::g_timer->removeTimerProc(&hintTimerCallback);

	switch (_hintRqState) {
	case kRQSpeaking:
		if (_vm->_actor->isSpeaking()) {
			Common::g_timer->installTimerProc(&hintTimerCallback, 50000, this);
			break;
		}

		_hintRqState = _hintNextRqState;
		Common::g_timer->installTimerProc(&hintTimerCallback, 333333, this);
		break;

	case kRQNoHint:
		//	Pick a random hint request.
		i = _hintOffer++;
		if (_hintOffer >= NUM_SOLICIT_REPLIES)
			_hintOffer = 0;

		//	Determine which of the journeymen will offer then
		//	hint, and then show that character's portrait.
		_hintGiver = portraitList[i];
		_hintSpeaker = _hintGiver - RID_ITE_JFERRET_SERIOUS;
		_vm->_interface->setRightPortrait(_hintGiver);

		_vm->_actor->nonActorSpeech(_hintBox, &solicitStr[_lang][i], 1, PUZZLE_SOLICIT_SOUNDS + i * 3 + _hintSpeaker, 0);

		//	Add Rif's reply to the list.
		clearHint();

		//	Roll to see if Sakka scolds
		if (_vm->_rnd.getRandomNumber(1)) {
			_hintRqState = kRQSakkaDenies;
			Common::g_timer->installTimerProc(&hintTimerCallback, 200000, this);
		} else {
			_hintRqState = kRQSpeaking;
			_hintNextRqState = kRQHintRequested;
			Common::g_timer->installTimerProc(&hintTimerCallback, 50000, this);
		}

		break;

	case kRQHintRequested:
		i = _vm->_rnd.getRandomNumber(NUM_SAKKA - 1);
		_vm->_actor->nonActorSpeech(_hintBox, &sakkaStr[_lang][i], 1, PUZZLE_SAKKA_SOUNDS + i, 0);

		_vm->_interface->setRightPortrait(RID_ITE_SAKKA_APPRAISING);

		_hintRqState = kRQSpeaking;
		_hintNextRqState = kRQHintRequestedStage2;
		Common::g_timer->installTimerProc(&hintTimerCallback, 50000, this);

		_vm->_interface->converseClear();
		_vm->_interface->converseAddText(optionsStr[_lang][kROAccept], 1, 0, 0 );
		_vm->_interface->converseAddText(optionsStr[_lang][kRODecline], 2, 0, 0 );
		_vm->_interface->converseAddText(optionsStr[_lang][kROLater], 0, 0, 0 );
		_vm->_interface->converseDisplayText();
		break;

	case kRQHintRequestedStage2:
		if (_vm->_rnd.getRandomNumber(1)) {			//	Skip Reply part
			i = _vm->_rnd.getRandomNumber(NUM_WHINES - 1);
			_vm->_actor->nonActorSpeech(_hintBox, &whineStr[_lang][i], 1, PUZZLE_WHINE_SOUNDS + i * 3 + _hintSpeaker, 0);
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
		total += _pieceInfo[i].flag & PUZZLE_FIT;

	if (_hintCount == 0 && (_pieceInfo[1].flag & PUZZLE_FIT
			|| _pieceInfo[12].flag & PUZZLE_FIT))
		_hintCount++;
	if (_hintCount == 1 && _pieceInfo[14].flag & PUZZLE_FIT)
		_hintCount++;

	if (_hintCount == 2 && total > 3)
		_hintCount++;

	_vm->_actor->setSpeechColor(1, kITEColorBlack);

	if (_hintCount < 3) {
		_vm->_actor->nonActorSpeech(_hintBox, &hintStr[_lang][_hintCount], 1, PUZZLE_HINT_SOUNDS + _hintCount * 3 + _hintSpeaker, 0);
	} else {
		int piece = 0;

		for (i = PUZZLE_PIECES - 1; i >= 0; i--) {
			piece = _piecePriority[i];
			if (_pieceInfo[piece].flag & PUZZLE_MOVED
					&& !(_pieceInfo[piece].flag & PUZZLE_FIT)) {
				if (_helpCount < 12)
					_helpCount++;
				break;
			}
		}

		if (i >= 0) {
			static char hintBuf[64];
			static const char *hintPtr = hintBuf;
			sprintf(hintBuf, optionsStr[_lang][kROHint], pieceNames[_lang][piece]);

			_vm->_actor->nonActorSpeech(_hintBox, &hintPtr, 1, PUZZLE_TOOL_SOUNDS + _hintSpeaker + piece * 3, 0);
		}
		else {
				//	If no pieces are in the wrong place
			_vm->_actor->nonActorSpeech(_hintBox, &hintStr[_lang][3], 1, PUZZLE_HINT_SOUNDS + 3 * 3 + _hintSpeaker, 0);
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
