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

#include "common/scummsys.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/message.h"
#include "trecision/nl/extern.h"
#include "trecision/trecision.h"

namespace Trecision {

uint32 NextRefresh = 0;
/*-------------------------------------------------------------------------*/
/*                               PROCESSTIME          					   */
/*-------------------------------------------------------------------------*/
void ProcessTime() {
#define SCR 2
	void RefreshAllAnimations();

	extern int16  TextStackTop;

	static uint8 OldRegInvSI  = 0xFF;
	static uint8 OldRegInvSL  = 0xFF;
	static uint8 OldLightIcon = 0xFF;

	TheTime = ReadTime();
	RefreshAllAnimations();

	if (TheTime >= NextRefresh) {
		PaintString();

		if ((g_vm->_inventoryStatus == INV_PAINT) || (g_vm->_inventoryStatus == INV_DEPAINT))
			RollInventory(g_vm->_inventoryStatus);

		if ((g_vm->_inventoryStatus != INV_OFF)
			&& ((OldRegInvSI != g_vm->_regenInvStartIcon) || (OldRegInvSL != g_vm->_regenInvStartLine) || (OldLightIcon != g_vm->_lightIcon) || (FlagForceRegenInventory))) {
			RegenInventory(g_vm->_regenInvStartIcon, g_vm->_regenInvStartLine);
			OldRegInvSI = g_vm->_regenInvStartIcon;
			OldRegInvSL = g_vm->_regenInvStartLine;
			OldLightIcon = g_vm->_lightIcon;
			FlagForceRegenInventory = false;
		}

		PaintScreen(0);
		TextStackTop = -1;

		FlagScreenRefreshed = true;
		uint32 PaintTime = ReadTime();
		if ((PaintTime - TheTime) >= 5)
			NextRefresh = PaintTime + 1;
		else
			NextRefresh = TheTime + 5;
		FlagMousePolling = true;
	}
}

/*-------------------------------------------------------------------------*/
/*                              PROCESSMOUSE          					   */
/*-------------------------------------------------------------------------*/
void ProcessMouse() {
	static bool MaskMouse;
	static uint16 oldmx;
	static uint16 oldmy;
	static bool LastMouseON = true;

	if ((LastMouseON == true) && (FlagMouseEnabled == false)) {
		oldmx = 0;    // Switch off
		oldmy = 0;
		Mouse(2);
	} else if ((LastMouseON == false) && (FlagMouseEnabled == true)) {
		oldmx = 0;    // Switch on
		oldmy = 0;
		Mouse(1);
	}

	LastMouseON = FlagMouseEnabled;
	Mouse(3);

	if (!FlagMouseEnabled)
		return;

	uint16 tmpMx = mx;

	if (mright || mleft) {
		if (!MaskMouse) {
			doEvent(MC_MOUSE, mright ? ME_MRIGHT : ME_MLEFT, MP_DEFAULT, tmpMx, my, 0, 0);
			MaskMouse = true;
		}
	} else
		MaskMouse = false;

	if (!(mright || mleft)) {
		if (!Flagscriptactive) {
			if ((tmpMx != oldmx) || (my != oldmy)) {
				doEvent(MC_MOUSE, ME_MMOVE, MP_DEFAULT, tmpMx, my, 0, 0);
				oldmx = tmpMx;
				oldmy = my;
			}
		}
	}
}

} // End of namespace Trecision
