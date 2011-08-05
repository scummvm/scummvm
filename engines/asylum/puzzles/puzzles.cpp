/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
#include "asylum/puzzles/puzzle11.h"
#include "asylum/puzzles/tictactoe.h"
#include "asylum/puzzles/timemachine.h"
#include "asylum/puzzles/vcr.h"
#include "asylum/puzzles/wheel.h"
#include "asylum/puzzles/writings.h"

#include "asylum/asylum.h"
#include "asylum/console.h"
#include "asylum/shared.h"

namespace Asylum {

Puzzles::Puzzles(AsylumEngine *engine) : _vm(engine) {
	memset(&_puzzles, 0, sizeof(_puzzles));
}

Puzzles::~Puzzles() {
	// Cleanup puzzles
	for (uint i = 0; i < ARRAYSIZE(_puzzles); i++)
		delete _puzzles[i];

	// Zero passed pointers
	_vm = NULL;
}

void Puzzles::reset() {
	for (uint i = 0; i < ARRAYSIZE(_puzzles); i++)
	delete _puzzles[i];

	initPuzzles();
}

EventHandler *Puzzles::getPuzzle(uint32 index) const {
	if (index >= ARRAYSIZE(_puzzles))
		error("[AsylumEngine::getPuzzleEventHandler] Invalid index (was: %d - max: %d)", index, ARRAYSIZE(_puzzles));

	if (_puzzles[index] == NULL)
		error("[AsylumEngine::getPuzzleEventHandler] This puzzle doesn't have an event handler! (index: %d)", index);

	return (EventHandler *)_puzzles[index];
}

void Puzzles::initPuzzles() {
	_puzzles[0] = new PuzzleVCR(_vm);
	_puzzles[1] = new PuzzlePipes(_vm);
	_puzzles[2] = new PuzzleTicTacToe(_vm);
	_puzzles[3] = new PuzzleLock(_vm);
	_puzzles[4] = NULL;    // No event handler for Puzzle 5
	_puzzles[5] = new PuzzleWheel(_vm);
	_puzzles[6] = new PuzzleBoardSalvation(_vm);
	_puzzles[7] = new PuzzleBoardYouth(_vm);
	_puzzles[8] = new PuzzleBoardKeyHidesTo(_vm);
	_puzzles[9] = new PuzzleWritings(_vm);
	_puzzles[10] = new Puzzle11(_vm);
	_puzzles[11] = new PuzzleMorgueDoor(_vm);
	_puzzles[12] = new PuzzleClock(_vm);
	_puzzles[13] = new PuzzleTimeMachine(_vm);
	_puzzles[14] = new PuzzleFisherman(_vm);
	_puzzles[15] = new PuzzleHiveMachine(_vm);
	_puzzles[16] = new PuzzleHiveControl(_vm);
}

void Puzzles::saveLoadWithSerializer(Common::Serializer &s) {
	for (int32 i = 0; i < ARRAYSIZE(_puzzles); i++) {
		if (_puzzles[i])
			_puzzles[i]->saveLoadWithSerializer(s);
	}
}

} // End of namespace Asylum
