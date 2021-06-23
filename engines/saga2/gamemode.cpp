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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

//#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/saga2.h"
#include "saga2/floating.h"

namespace Saga2 {

/* ===================================================================== *
   Functions to set what mode the game is in.
 * ===================================================================== */

//Initialize Static GameObject Data Members

GameMode    *GameMode::currentMode = NULL;  // pointer to current mode.
GameMode    *GameMode::newMode = NULL;      // next mode to run

GameMode       *GameMode::modeStackPtr[Max_Modes] = { NULL };
GameMode       *GameMode::newmodeStackPtr[Max_Modes] = { NULL };
int         GameMode::modeStackCtr = 0;
int         GameMode::newmodeStackCtr = 0;
int         GameMode::newmodeFlag = false;

void GameMode::modeUnStack(void) {
	modeStackPtr[modeStackCtr] = NULL;                        //Always Start Cleanup At modeStackCtr
	modeStackPtr[modeStackCtr--]->cleanup();
	return;
}

void GameMode::modeUnStack(int StopHere) {
	if (!modeStackCtr)   //If Nothing Currently On The Stack
		return;
	for (int i = modeStackCtr - 1; i >= StopHere; i--) { //Stop Here Is How Far You Want To Unstack
		if (modeStackPtr[i] != NULL)
			modeStackPtr[i]->cleanup();
		modeStackPtr[i] = NULL;                        //Always Start Cleanup At modeStackCtr
		modeStackCtr--;                        //Always Start Cleanup At modeStackCtr
	}
	return;
}

bool GameMode::update(void) {
	bool            result = false;
	int             ModeCtr = 0;


	newmodeFlag = false;

	for (int i = 0; i < newmodeStackCtr; i++, ModeCtr++)
		if (newmodeStackPtr[i] != modeStackPtr[i])
			break;

	//Now ModeCtr Equals How Deep In The Mode Is Equal

	modeUnStack(ModeCtr);

	for (int i = ModeCtr; i < newmodeStackCtr; i++)
		modeStack(newmodeStackPtr[i]);

	return result;

}

int GameMode::getStack(GameMode **saveStackPtr) {
	memcpy(saveStackPtr, modeStackPtr, sizeof(GameMode *) * modeStackCtr);
	return modeStackCtr;
}

void GameMode::SetStack(GameMode *modeFirst, ...) {
	va_list Modes;
	va_start(Modes, modeFirst); //Initialize To First Argument Even Though We Dont Use It In The Loop
	newmodeStackCtr = 0; //reset Ctr For New Mode
	GameMode *thisMode = modeFirst;

	//Put List In New Array Of GameMode Object Pointers

	while (thisMode != 0) {
		newmodeStackPtr[newmodeStackCtr] = thisMode;
		newmodeStackCtr++;
		thisMode  = va_arg(Modes, GameMode *);

	}
	va_end(Modes); //Clean Up

	newmodeFlag = true;
}

void GameMode::SetStack(GameMode **newStack, int newStackSize) {
	newmodeStackCtr = newStackSize;
	memcpy(newmodeStackPtr, newStack, sizeof(GameMode *) * newStackSize);
	newmodeFlag = true;
}

void GameMode::modeStack(GameMode *AddThisMode) {
	modeStackPtr[modeStackCtr++] = AddThisMode;
	AddThisMode->setup();
	return;
}

} // end of namespace Saga2
