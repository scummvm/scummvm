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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/floating.h"

namespace Saga2 {

/* ===================================================================== *
   Functions to set what mode the game is in.
 * ===================================================================== */

//Initialize Static GameObject Data Members

GameMode    *GameMode::_currentMode = nullptr;  // pointer to current mode.
GameMode    *GameMode::_newMode = nullptr;      // next mode to run

GameMode       *GameMode::_modeStackPtr[kMax_Modes] = { nullptr };
GameMode       *GameMode::_newmodeStackPtr[kMax_Modes] = { nullptr };
int         GameMode::_modeStackCtr = 0;
int         GameMode::_newmodeStackCtr = 0;
int         GameMode::_newmodeFlag = false;

void GameMode::modeUnStack() {
	_modeStackPtr[_modeStackCtr] = nullptr;                        //Always Start Cleanup At _modeStackCtr
	_modeStackPtr[_modeStackCtr--]->_cleanup();
	return;
}

void GameMode::modeUnStack(int StopHere) {
	if (!_modeStackCtr)   //If Nothing Currently On The Stack
		return;
	for (int i = _modeStackCtr - 1; i >= StopHere; i--) { //Stop Here Is How Far You Want To Unstack
		if (_modeStackPtr[i] != nullptr)
			_modeStackPtr[i]->_cleanup();
		_modeStackPtr[i] = nullptr;                        //Always Start Cleanup At _modeStackCtr
		_modeStackCtr--;                        //Always Start Cleanup At _modeStackCtr
	}
	return;
}

bool GameMode::update() {
	bool            result = false;
	int             ModeCtr = 0;


	_newmodeFlag = false;

	for (int i = 0; i < _newmodeStackCtr; i++, ModeCtr++)
		if (_newmodeStackPtr[i] != _modeStackPtr[i])
			break;

	//Now ModeCtr Equals How Deep In The Mode Is Equal

	modeUnStack(ModeCtr);

	for (int i = ModeCtr; i < _newmodeStackCtr; i++)
		modeStack(_newmodeStackPtr[i]);

	return result;

}

int GameMode::getStack(GameMode **saveStackPtr) {
	memcpy(saveStackPtr, _modeStackPtr, sizeof(GameMode *) * _modeStackCtr);
	return _modeStackCtr;
}

void GameMode::SetStack(GameMode *modeFirst, ...) {
	va_list Modes;
	va_start(Modes, modeFirst); //Initialize To First Argument Even Though We Dont Use It In The Loop
	_newmodeStackCtr = 0; //reset Ctr For New Mode
	GameMode *thisMode = modeFirst;

	//Put List In New Array Of GameMode Object Pointers

	while (thisMode != nullptr) {
		_newmodeStackPtr[_newmodeStackCtr] = thisMode;
		_newmodeStackCtr++;
		thisMode  = va_arg(Modes, GameMode *);

	}
	va_end(Modes); //Clean Up

	_newmodeFlag = true;
}

void GameMode::SetStack(GameMode **newStack, int newStackSize) {
	_newmodeStackCtr = newStackSize;
	memcpy(_newmodeStackPtr, newStack, sizeof(GameMode *) * newStackSize);
	_newmodeFlag = true;
}

void GameMode::modeStack(GameMode *AddThisMode) {
	_modeStackPtr[_modeStackCtr++] = AddThisMode;
	AddThisMode->_setup();
	return;
}

} // end of namespace Saga2
