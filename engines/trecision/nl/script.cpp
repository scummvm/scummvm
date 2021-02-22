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
#include "trecision/nl/sysdef.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/define.h"
#include "trecision/nl/message.h"
#include "trecision/nl/extern.h"

namespace Trecision {

/* -----------------18/09/97 11.54-------------------
					initScript
 --------------------------------------------------*/
void initScript() {
	int f = 0;

	Script[s16CARD]._firstFrame = f;
	Script[s16CARD]._flag = 0;

	ScriptFrame[f]._class = MC_CHARACTER;
	ScriptFrame[f]._event = ME_CHARACTERACTION;
	ScriptFrame[f++]._u16Param1 = a166USACREDITCARD;

	ScriptFrame[f]._class = MC_ACTION;
	ScriptFrame[f]._event = ME_MOUSEOPERATE;
	ScriptFrame[f++]._u32Param = oTICKETOFFICE16;

	Script[S16MONEY]._firstFrame = ++f;
	Script[S16MONEY]._flag = 0;

	ScriptFrame[f]._class = MC_CHARACTER;
	ScriptFrame[f]._event = ME_CHARACTERACTION;
	ScriptFrame[f++]._u16Param1 = a167USAMONETA;

	ScriptFrame[f]._class = MC_ACTION;
	ScriptFrame[f]._event = ME_MOUSEOPERATE;
	ScriptFrame[f++]._u32Param = oTICKETOFFICE16;

	Script[s19EVA]._firstFrame = ++f;
	Script[s19EVA]._flag = 0;

	ScriptFrame[f]._class = MC_MOUSE;
	ScriptFrame[f]._event = ME_MRIGHT;
	ScriptFrame[f]._u8Param = 1;
	ScriptFrame[f]._u16Param1 = 214;
	ScriptFrame[f]._u16Param2 = TOP + 140;
	ScriptFrame[f++]._u32Param = ocEVA19;

	Script[s4AHELLEN]._firstFrame = ++f;
	Script[s4AHELLEN]._flag = 0;

	ScriptFrame[f]._class = MC_CHARACTER;
	ScriptFrame[f]._event = ME_CHARACTERACTION;
	ScriptFrame[f++]._u16Param1 = a4A2;

	ScriptFrame[f]._class = MC_MOUSE;
	ScriptFrame[f]._event = ME_MLEFT;
	ScriptFrame[f]._u8Param = 1;
	ScriptFrame[f]._u16Param1 = 336;
	ScriptFrame[f]._u16Param2 = 263 + TOP;
	ScriptFrame[f++]._u32Param = 0;

	ScriptFrame[f]._class = MC_MOUSE;
	ScriptFrame[f]._event = ME_MLEFT;
	ScriptFrame[f]._u8Param = 1;
	ScriptFrame[f]._u16Param1 = 336;
	ScriptFrame[f]._u16Param2 = 260 + TOP;
	ScriptFrame[f++]._u32Param = 0;

	ScriptFrame[f]._class = MC_DIALOG;
	ScriptFrame[f]._event = ME_STARTDIALOG;
	ScriptFrame[f++]._u16Param1 = dF4A3;

	Script[s49SUNDIAL]._firstFrame = ++f;
	Script[s49SUNDIAL]._flag = 0;

	ScriptFrame[f]._class = MC_SCRIPT;
	ScriptFrame[f]._event = ME_CHANGER;
	ScriptFrame[f]._u16Param1 = r49;
	ScriptFrame[f]._u16Param2 = a496;
	ScriptFrame[f]._u8Param = 1;
	ScriptFrame[f++]._u32Param = 1;

	ScriptFrame[f]._class = MC_MOUSE;
	ScriptFrame[f]._event = ME_MLEFT;
	ScriptFrame[f]._u8Param = 1;
	ScriptFrame[f]._u16Param1 = 360;
	ScriptFrame[f]._u16Param2 = 255 + TOP;
	ScriptFrame[f++]._u32Param = 0;

	ScriptFrame[f]._class = MC_SCRIPT;
	ScriptFrame[f]._event = ME_CHANGER;
	ScriptFrame[f]._u16Param1 = r4A;
	ScriptFrame[f]._u16Param2 = 0;
	ScriptFrame[f]._u8Param = 1;
	ScriptFrame[f++]._u32Param = 1;

	//	Fast change room in double room of level 2
	Script[s21TO22]._firstFrame = ++f;
	Script[s21TO22]._flag = 0;

	ScriptFrame[f]._class = MC_MOUSE;
	ScriptFrame[f]._u16Param1 = 317;
	ScriptFrame[f]._u8Param = 1;
	ScriptFrame[f]._u16Param2 = 166;
	ScriptFrame[f]._event = ME_MRIGHT;
	ScriptFrame[f++]._u32Param = oCATENAT21;

	ScriptFrame[f]._class = MC_MOUSE;
	ScriptFrame[f]._u16Param1 = 429;
	ScriptFrame[f]._u8Param = 1;
	ScriptFrame[f]._u16Param2 = 194;
	ScriptFrame[f]._event = ME_MLEFT;
	ScriptFrame[f++]._u32Param = od21ALLA22;

	Script[s21TO23]._firstFrame = ++f;
	Script[s21TO23]._flag = 0;

	ScriptFrame[f]._class = MC_MOUSE;
	ScriptFrame[f]._u16Param1 = 317;
	ScriptFrame[f]._u8Param = 1;
	ScriptFrame[f]._u16Param2 = 166;
	ScriptFrame[f]._event = ME_MRIGHT;
	ScriptFrame[f++]._u32Param = oCATENAT21;

	ScriptFrame[f]._class = MC_MOUSE;
	ScriptFrame[f]._u16Param1 = 204;
	ScriptFrame[f]._u8Param = 1;
	ScriptFrame[f]._u16Param2 = 157;
	ScriptFrame[f]._event = ME_MLEFT;
	ScriptFrame[f++]._u32Param = od21ALLA23;

	Script[s24TO23]._firstFrame = ++f;
	Script[s24TO23]._flag = 0;
	ScriptFrame[f]._class = MC_MOUSE;
	ScriptFrame[f]._u16Param1 = 316;
	ScriptFrame[f]._u8Param = 1;
	ScriptFrame[f]._u16Param2 = 213;
	ScriptFrame[f]._event = ME_MRIGHT;
	ScriptFrame[f++]._u32Param = oPASSAGGIO24;

	ScriptFrame[f]._class = MC_MOUSE;
	ScriptFrame[f]._u16Param1 = 120;
	ScriptFrame[f]._u8Param = 1;
	ScriptFrame[f]._u16Param2 = 196;
	ScriptFrame[f]._event = ME_MLEFT;
	ScriptFrame[f++]._u32Param = od24ALLA23;

	Script[s24TO26]._firstFrame = ++f;
	Script[s24TO26]._flag = 0;

	ScriptFrame[f]._class = MC_MOUSE;
	ScriptFrame[f]._u16Param1 = 316;
	ScriptFrame[f]._u8Param = 1;
	ScriptFrame[f]._u16Param2 = 213;
	ScriptFrame[f]._event = ME_MRIGHT;
	ScriptFrame[f++]._u32Param = oPASSAGGIO24;

	ScriptFrame[f]._class = MC_MOUSE;
	ScriptFrame[f]._u16Param1 = 527;
	ScriptFrame[f]._u8Param = 1;
	ScriptFrame[f]._u16Param2 = 187;
	ScriptFrame[f]._event = ME_MLEFT;
	ScriptFrame[f++]._u32Param = od24ALLA26;

	Script[s2ETO2C]._firstFrame = ++f;
	Script[s2ETO2C]._flag = 0;

	ScriptFrame[f]._class = MC_MOUSE;
	ScriptFrame[f]._u16Param1 = 420;
	ScriptFrame[f]._u8Param = 1;
	ScriptFrame[f]._u16Param2 = 238;
	ScriptFrame[f]._event = ME_MRIGHT;
	ScriptFrame[f++]._u32Param = oCATWALKA2E;

	ScriptFrame[f]._class = MC_MOUSE;
	ScriptFrame[f]._u16Param1 = 66;
	ScriptFrame[f]._u8Param = 1;
	ScriptFrame[f]._u16Param2 = 336;
	ScriptFrame[f]._event = ME_MLEFT;
	ScriptFrame[f++]._u32Param = od2EALLA2C;

	Script[s2ETO2F]._firstFrame = ++f;
	Script[s2ETO2F]._flag = 0;

	ScriptFrame[f]._class = MC_MOUSE;
	ScriptFrame[f]._u16Param1 = 420;
	ScriptFrame[f]._u8Param = 1;
	ScriptFrame[f]._u16Param2 = 238;
	ScriptFrame[f]._event = ME_MRIGHT;
	ScriptFrame[f++]._u32Param = oCATWALKA2E;

	ScriptFrame[f]._class = MC_MOUSE;
	ScriptFrame[f]._u16Param1 = 213;
	ScriptFrame[f]._u8Param = 1;
	ScriptFrame[f]._u16Param2 = 69;
	ScriptFrame[f]._event = ME_MLEFT;
	ScriptFrame[f++]._u32Param = oENTRANCE2E;
}

} // End of namespace Trecision
