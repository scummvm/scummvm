/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "sky/sky.h"
#include "sky/skydefs.h"


void SkyState::initTimer(void) {

	_tseqFrames = 0;
	_timer = Engine::_timer; 
	_timer->installProcedure(&timerHandler, 1000000 / 50); //call 50 times per second
	
}
	
void SkyState::timerHandler(void *ptr) {

	SkyState *_this = (SkyState *)ptr;
	
	//music_command(0x300);
	_this->doTimerSequence();
}

void SkyState::startTimerSequence(byte *sequence) {

	_tseqFrames = *sequence++;
	_tseqData = sequence;
	_tseqCounter = SEQUENCE_COUNT;
	debug(5, "Starting new sequence of %d frames.", _tseqFrames);
	
}

void SkyState::doTimerSequence(void) {

	byte  nrToSkip, nrToDo, tmp;
	byte *scrPtr = _workScreen;
	
	if (_tseqFrames == 0)
		return;
	
	_tseqCounter--;
	if (_tseqCounter != 0)
		return;

	_tseqCounter = SEQUENCE_COUNT;

	do {
		do {
			nrToSkip = *_tseqData++;
			scrPtr += nrToSkip;
		} while (nrToSkip == 255);
		do {
			nrToDo = *_tseqData++;
			tmp = nrToDo;
			while (tmp--) {
				*scrPtr++ = *_tseqData++;
			}
		} while (nrToDo == 255);
	} while ((scrPtr - _workScreen) < (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT));

	showScreen();
	_tseqFrames--;
		
}
