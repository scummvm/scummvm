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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_MODAL_H
#define SAGA2_MODAL_H

#include "saga2/floating.h"

namespace Saga2 {

void ModalModeSetup();
void ModalModeCleanup();
void ModalModeHandleTask();
void ModalModeHandleKey(short, short);

//Modal Mode GameMode Object

extern GameMode     ModalMode;

/* ===================================================================== *
   ModalWindow --
 * ===================================================================== */
class ModalWindow : public DecoratedWindow {

	GameMode    *prevModeStackPtr[Max_Modes];
	int         prevModeStackCtr;

public:

	ModalWindow(const Rect16 &r,
	            uint16 ident,
	            AppFunc *cmd);
	~ModalWindow();

	bool open();
	void close();
	bool isModal();

	static ModalWindow *current;
	void handleKey(short, short);
};

} // end of namespace Saga2

#endif
