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
#include "saga2/modal.h"
#include "saga2/speech.h"
#include "saga2/grabinfo.h"

namespace Saga2 {

ModalWindow *mWinPtr;

APPFUNC(cmdModalWindow);

void ModalModeSetup() {}
void ModalModeCleanup() {}
void ModalModeHandleTask() {}

GameMode        ModalMode = {
	nullptr,                                // no previous mode
	false,                                  // mode is not nestable
	ModalModeSetup,
	ModalModeCleanup,
	ModalModeHandleTask,
	ModalModeHandleKey,
	nullptr
};

extern void updateWindowSection(const Rect16 &r);

/* ===================================================================== *
   Modal Window class member functions
 * ===================================================================== */

ModalWindow *ModalWindow::current = nullptr;

ModalWindow::ModalWindow(const Rect16 &r, uint16 ident, AppFunc *cmd)
		: DecoratedWindow(r, ident, "DialogWindow", cmd) {
	prevModeStackCtr = 0;

	for (int i = 0; i < Max_Modes; i++)
		prevModeStackPtr[i] = nullptr;
}

ModalWindow::~ModalWindow() {

	//  Kludge because of Visual C's patching of vptr in destructor.
	if (isOpen()) close();

}

bool ModalWindow::isModal() {
	return openFlag;
}

bool ModalWindow::open() {
	g_vm->_mouseInfo->replaceObject();
	g_vm->_mouseInfo->clearGauge();
	g_vm->_mouseInfo->setText(nullptr);
	g_vm->_mouseInfo->setIntent(GrabInfo::WalkTo);

	prevModeStackCtr = GameMode::getStack(prevModeStackPtr);

	GameMode *gameModes[] = {&PlayMode, &TileMode, &ModalMode};
	GameMode::SetStack(gameModes, 3);
	current = this;



	return gWindow::open();
}

void ModalWindow::close() {
	gWindow::close();

	GameMode::SetStack(prevModeStackPtr, prevModeStackCtr);
	updateWindowSection(_extent);
}


void ModalModeHandleKey(short k, short s) {
}

} // end of namespace Saga2
