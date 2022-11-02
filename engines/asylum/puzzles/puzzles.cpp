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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "asylum/puzzles/puzzles.h"

#include "asylum/puzzles/boardkeyhidesto.h"
#include "asylum/puzzles/boardsalvation.h"
#include "asylum/puzzles/boardyouth.h"
#include "asylum/puzzles/clock.h"
#include "asylum/puzzles/fisherman.h"
#include "asylum/puzzles/hivecontrol.h"
#include "asylum/puzzles/hivemachine.h"
#include "asylum/puzzles/lock.h"
#include "asylum/puzzles/morguedoor.h"
#include "asylum/puzzles/pipes.h"
#include "asylum/puzzles/tictactoe.h"
#include "asylum/puzzles/timemachine.h"
#include "asylum/puzzles/vcr.h"
#include "asylum/puzzles/wheel.h"
#include "asylum/puzzles/writings.h"

#include "asylum/asylum.h"

namespace Asylum {

Puzzles::Puzzles(AsylumEngine *engine) : _vm(engine) {
	memset(&_puzzles, 0, sizeof(_puzzles));
}

Puzzles::~Puzzles() {
	// Cleanup puzzles
	for (uint i = 0; i < ARRAYSIZE(_puzzles); i++)
		delete _puzzles[i];
}

void Puzzles::reset() {
	for (uint i = 0; i < ARRAYSIZE(_puzzles); i++)
		delete _puzzles[i];

	initPuzzles();
}

EventHandler *Puzzles::getPuzzle(uint32 index) const {
	if (index >= ARRAYSIZE(_puzzles))
		error("[AsylumEngine::getPuzzleEventHandler] Invalid index (was: %d - max: %d)", index, ARRAYSIZE(_puzzles));

	if (_puzzles[index] == nullptr)
		error("[AsylumEngine::getPuzzleEventHandler] This puzzle doesn't have an event handler! (index: %d)", index);

	return (EventHandler *)_puzzles[index];
}

void Puzzles::initPuzzles() {
	_puzzles[kPuzzleVCR]             = new PuzzleVCR(_vm);
	_puzzles[kPuzzlePipes]           = new PuzzlePipes(_vm);
	_puzzles[kPuzzleTicTacToe]       = new PuzzleTicTacToe(_vm);
	_puzzles[kPuzzleLock]            = new PuzzleLock(_vm);
	_puzzles[kPuzzle4]               = nullptr;    // No event handler for Puzzle 5
	_puzzles[kPuzzleWheel]           = new PuzzleWheel(_vm);
	_puzzles[kPuzzleBoardSalvation]  = new PuzzleBoardSalvation(_vm);
	_puzzles[kPuzzleBoardYouth]      = new PuzzleBoardYouth(_vm);
	_puzzles[kPuzzleBoardKeyHidesTo] = new PuzzleBoardKeyHidesTo(_vm);
	_puzzles[kPuzzleWritings]        = new PuzzleWritings(_vm);
	_puzzles[kPuzzle11]              = nullptr;
	_puzzles[kPuzzleMorgueDoor]      = new PuzzleMorgueDoor(_vm);
	_puzzles[kPuzzleClock]           = new PuzzleClock(_vm);
	_puzzles[kPuzzleTimeMachine]     = new PuzzleTimeMachine(_vm);
	_puzzles[kPuzzleFisherman]       = new PuzzleFisherman(_vm);
	_puzzles[kPuzzleHiveMachine]     = new PuzzleHiveMachine(_vm);
	_puzzles[kPuzzleHiveControl]     = new PuzzleHiveControl(_vm);
}

void Puzzles::saveLoadWithSerializer(Common::Serializer &s) {
	_puzzles[kPuzzleVCR]->saveLoadWithSerializer(s);
	s.skip(4);
	_puzzles[kPuzzleLock]->saveLoadWithSerializer(s);
	_puzzles[kPuzzlePipes]->saveLoadWithSerializer(s);
	_puzzles[kPuzzleWheel]->saveLoadWithSerializer(s);
	_puzzles[kPuzzleBoardSalvation]->saveLoadWithSerializer(s);
	_puzzles[kPuzzleBoardYouth]->saveLoadWithSerializer(s);
	s.skip(8);
	_puzzles[kPuzzleBoardKeyHidesTo]->saveLoadWithSerializer(s);
	_puzzles[kPuzzleMorgueDoor]->saveLoadWithSerializer(s);
	s.skip(80); // skip Puzzle 11
	_puzzles[kPuzzleTimeMachine]->saveLoadWithSerializer(s);
	_puzzles[kPuzzleClock]->saveLoadWithSerializer(s);
	_puzzles[kPuzzleFisherman]->saveLoadWithSerializer(s);
	_puzzles[kPuzzleHiveControl]->saveLoadWithSerializer(s);
}

} // End of namespace Asylum
