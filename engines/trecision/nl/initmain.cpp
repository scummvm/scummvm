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

#include <string.h>
#include "trecision/nl/lib/addtype.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/define.h"
#include "trecision/nl/message.h"
#include "trecision/nl/extern.h"
#include "trecision/trecision.h"

namespace Trecision {

/*-----------------03/01/97 16.15-------------------
					InitMain
--------------------------------------------------*/
void initMain() {
	for (int c = 0; c < MAXOBJ; c++)
		g_vm->_obj[c]._position = -1;

	initNames();
	initScript();
	openSys();

	LoadAll();

	InitMessageSystem();                   // schedule.c
	g_vm->_inventorySize = 0;
	memset(g_vm->_inventory, iNULL, MAXICON);

	g_vm->_inventory[g_vm->_inventorySize++] = iBANCONOTE;
	g_vm->_inventory[g_vm->_inventorySize++] = iSAM;
	g_vm->_inventory[g_vm->_inventorySize++] = iCARD03;
	g_vm->_inventory[g_vm->_inventorySize++] = iPEN;
	g_vm->_inventory[g_vm->_inventorySize++] = iKEY05;

	g_vm->_curRoom = rINTRO;

	ProcessTime();

	doEvent(MC_SYSTEM, ME_START, MP_DEFAULT, 0, 0, 0, 0);
}

/* -----------------25/12/96 16.40-------------------
					NextMessage
 --------------------------------------------------*/
void NextMessage() {
	if (!SemNoPaintScreen)
		ProcessTime();

	ProcessMouse();
	Scheduler();

	if ((g_vm->TheMessage->cls == MC_SYSTEM) && (g_vm->TheMessage->event == ME_QUIT))
		CloseSys(NULL);

	AtFrameHandler(BACKGROUND_ANIM);

	ProcessTheMessage();

	if (Semscriptactive)
		EvalScript();
}

} // End of namespace Trecision
