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

#include "saga2/saga2.h"
#include "saga2/modal.h"
#include "saga2/speech.h"
#include "saga2/grabinfo.h"

namespace Saga2 {

ModalWindow *mWinPtr;

extern gPixelMap    tileDrawMap;
extern Point16      fineScroll;
extern gToolBase    G_BASE;

APPFUNC(cmdModalWindow);

void ModalModeSetup(void) {}
void ModalModeCleanup(void) {}
void ModalModeHandleTask(void) {}
void ModalModeHandleKey(short, short);

GameMode        ModalMode = {
	NULL,                                   // no previous mode
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

ModalWindow *ModalWindow::current = NULL;

ModalWindow::ModalWindow(const Rect16 &r,
                         uint16 ident,
                         AppFunc *cmd)
	: DecoratedWindow(r, ident, "DialogWindow", cmd) {
}

ModalWindow::~ModalWindow(void) {

	//  Kludge because of Visual C's patching of vptr in destructor.
	if (isOpen()) close();

}

bool ModalWindow::isModal(void) {
	return openFlag;
}

bool ModalWindow::open(void) {
	g_vm->_mouseInfo->replaceObject();
	g_vm->_mouseInfo->clearGauge();
	g_vm->_mouseInfo->setText(NULL);
	g_vm->_mouseInfo->setIntent(GrabInfo::WalkTo);

	prevModeStackCtr = GameMode::getStack(prevModeStackPtr);

	GameMode *gameModes[] = {&PlayMode, &TileMode, &ModalMode};
	GameMode::SetStack(gameModes, 3);
	current = this;



	return gWindow::open();
}

void ModalWindow::close(void) {
	gWindow::close();

	GameMode::SetStack(prevModeStackPtr, prevModeStackCtr);
	updateWindowSection(extent);
}


void ModalModeHandleKey(short k, short s) {
}

} // end of namespace Saga2
