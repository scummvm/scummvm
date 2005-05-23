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

#include "saga/interface.h"
#include "saga/scene.h"
#include "saga/puzzle.h"
#include "saga/resnames.h"

#include "common/timer.h"

namespace Saga {

Puzzle::Puzzle(SagaEngine *vm) : _vm(vm), _solved(false), _active(false) {
}

void Puzzle::execute(void) {
	_active = true;
	Common::g_timer->installTimerProc(&hintTimerCallback, ticksToMSec(30), this);

	_solved = true; // Cheat
	exitPuzzle();
}

void Puzzle::exitPuzzle(void) {
	_active = false;

	Common::g_timer->removeTimerProc(&hintTimerCallback);

	_vm->_scene->changeScene(ITE_SCENE_LODGE, 0, kTransitionNoFade);
	_vm->_interface->setMode(kPanelMain);
}


void Puzzle::hintTimerCallback(void *refCon) {                                   
        ((Puzzle *)refCon)->hintTimer();                                         
}                                                                               

void Puzzle::hintTimer(void) {
}

void Puzzle::handleReply(int reply) {
}

void Puzzle::movePiece(Point mousePt) {
}


} // End of namespace Saga
