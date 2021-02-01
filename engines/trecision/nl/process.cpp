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

#include <stdio.h>
#include "trecision/nl/lib/addtype.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/define.h"
#include "trecision/nl/message.h"
#include "trecision/nl/extern.h"

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

		if ((_inventoryStatus == INV_PAINT) || (_inventoryStatus == INV_DEPAINT))
			RollInventory(_inventoryStatus);

		if ((_inventoryStatus != INV_OFF)  && ((OldRegInvSI  != RegenInvStartIcon) ||
											   (OldRegInvSL != RegenInvStartLine) || (OldLightIcon != LightIcon) ||
											   (SemForceRegenInventory))) {
			RegenInventory(RegenInvStartIcon, RegenInvStartLine);
			OldRegInvSI = RegenInvStartIcon;
			OldRegInvSL = RegenInvStartLine;
			OldLightIcon = LightIcon;
			SemForceRegenInventory = false;
		}

		PaintScreen(0);
		TextStackTop = -1;

		SemScreenRefreshed = true;
		uint32 PaintTime = ReadTime();
		if ((PaintTime - TheTime) >= 5)
			NextRefresh = PaintTime + 1;
		else
			NextRefresh = TheTime + 5;
		SemMousePolling = true;
	}
}

/*-------------------------------------------------------------------------*/
/*                              PROCESSMOUSE          					   */
/*-------------------------------------------------------------------------*/
void ProcessMouse() {
	static LLBOOL MaskMouse;
	static uint16 oldmx, oldmy;
	static LLBOOL LastMouseON = true;
	uint16 tmpmx;

	if ((LastMouseON == true) && (SemMouseEnabled == false)) {
		oldmx = 0;    // SPEGNI
		oldmy = 0;
		Mouse(2);
	} else if ((LastMouseON == false) && (SemMouseEnabled == true)) {
		oldmx = 0;    // ACCENDI
		oldmy = 0;
		Mouse(1);
	}

	LastMouseON = SemMouseEnabled;

	Mouse(3);

	if (/*(!Semscriptactive) && */(!SemMouseEnabled))
		return;

	if ((my >= VideoCent.y0) && (my <= VideoCent.y1))
		tmpmx = mx;
	else
		tmpmx = mx;

	if ((mright || mleft)) {
		if (!MaskMouse) {
			if (mright)
				doEvent(MC_MOUSE, ME_MRIGHT, MP_DEFAULT, (uint16)tmpmx, (uint16)my, 0, 0);
			else
				doEvent(MC_MOUSE, ME_MLEFT, MP_DEFAULT, (uint16)tmpmx, (uint16)my, 0, 0);
			MaskMouse = true;
		}
	} else
		MaskMouse = false;

	if (!(mright || mleft))
		if (!Semscriptactive)
			if ((tmpmx != oldmx) || (my != oldmy)) {
				doEvent(MC_MOUSE, ME_MMOVE, MP_DEFAULT, (uint16)tmpmx, (uint16)my, 0, 0);
				oldmx = tmpmx;
				oldmy = my;
			}

}

} // End of namespace Trecision
