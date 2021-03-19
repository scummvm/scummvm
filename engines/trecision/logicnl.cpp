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

#include "trecision.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/define.h"
#include "trecision/nl/message.h"

namespace Trecision {

/* -----------------18/09/97 11.54-------------------
					initScript
 --------------------------------------------------*/
void TrecisionEngine::initScript() {
	int f = 0;

	_script[s16CARD]._firstFrame = f;
	_script[s16CARD]._flag = 0;

	_scriptFrame[f]._class = MC_CHARACTER;
	_scriptFrame[f]._event = ME_CHARACTERACTION;
	_scriptFrame[f++]._u16Param1 = a166USECREDITCARD;

	_scriptFrame[f]._class = MC_ACTION;
	_scriptFrame[f]._event = ME_MOUSEOPERATE;
	_scriptFrame[f++]._u32Param = oTICKETOFFICE16;

	_script[S16MONEY]._firstFrame = ++f;
	_script[S16MONEY]._flag = 0;

	_scriptFrame[f]._class = MC_CHARACTER;
	_scriptFrame[f]._event = ME_CHARACTERACTION;
	_scriptFrame[f++]._u16Param1 = a167USEMONEY;

	_scriptFrame[f]._class = MC_ACTION;
	_scriptFrame[f]._event = ME_MOUSEOPERATE;
	_scriptFrame[f++]._u32Param = oTICKETOFFICE16;

	_script[s19EVA]._firstFrame = ++f;
	_script[s19EVA]._flag = 0;

	_scriptFrame[f]._class = MC_MOUSE;
	_scriptFrame[f]._event = ME_MRIGHT;
	_scriptFrame[f]._u8Param = 1;
	_scriptFrame[f]._u16Param1 = 214;
	_scriptFrame[f]._u16Param2 = TOP + 140;
	_scriptFrame[f++]._u32Param = ocEVA19;

	_script[s4AHELLEN]._firstFrame = ++f;
	_script[s4AHELLEN]._flag = 0;

	_scriptFrame[f]._class = MC_CHARACTER;
	_scriptFrame[f]._event = ME_CHARACTERACTION;
	_scriptFrame[f++]._u16Param1 = a4A2;

	_scriptFrame[f]._class = MC_MOUSE;
	_scriptFrame[f]._event = ME_MLEFT;
	_scriptFrame[f]._u8Param = 1;
	_scriptFrame[f]._u16Param1 = 336;
	_scriptFrame[f]._u16Param2 = 263 + TOP;
	_scriptFrame[f++]._u32Param = 0;

	_scriptFrame[f]._class = MC_MOUSE;
	_scriptFrame[f]._event = ME_MLEFT;
	_scriptFrame[f]._u8Param = 1;
	_scriptFrame[f]._u16Param1 = 336;
	_scriptFrame[f]._u16Param2 = 260 + TOP;
	_scriptFrame[f++]._u32Param = 0;

	_scriptFrame[f]._class = MC_DIALOG;
	_scriptFrame[f]._event = ME_STARTDIALOG;
	_scriptFrame[f++]._u16Param1 = dF4A3;

	_script[s49SUNDIAL]._firstFrame = ++f;
	_script[s49SUNDIAL]._flag = 0;

	_scriptFrame[f]._class = MC_SCRIPT;
	_scriptFrame[f]._event = ME_CHANGER;
	_scriptFrame[f]._u16Param1 = r49;
	_scriptFrame[f]._u16Param2 = a496;
	_scriptFrame[f]._u8Param = 1;
	_scriptFrame[f++]._u32Param = 1;

	_scriptFrame[f]._class = MC_MOUSE;
	_scriptFrame[f]._event = ME_MLEFT;
	_scriptFrame[f]._u8Param = 1;
	_scriptFrame[f]._u16Param1 = 360;
	_scriptFrame[f]._u16Param2 = 255 + TOP;
	_scriptFrame[f++]._u32Param = 0;

	_scriptFrame[f]._class = MC_SCRIPT;
	_scriptFrame[f]._event = ME_CHANGER;
	_scriptFrame[f]._u16Param1 = r4A;
	_scriptFrame[f]._u16Param2 = 0;
	_scriptFrame[f]._u8Param = 1;
	_scriptFrame[f++]._u32Param = 1;

	//	Fast change room in double room of level 2
	_script[s21TO22]._firstFrame = ++f;
	_script[s21TO22]._flag = 0;

	_scriptFrame[f]._class = MC_MOUSE;
	_scriptFrame[f]._u16Param1 = 317;
	_scriptFrame[f]._u8Param = 1;
	_scriptFrame[f]._u16Param2 = 166;
	_scriptFrame[f]._event = ME_MRIGHT;
	_scriptFrame[f++]._u32Param = oCATENAT21;

	_scriptFrame[f]._class = MC_MOUSE;
	_scriptFrame[f]._u16Param1 = 429;
	_scriptFrame[f]._u8Param = 1;
	_scriptFrame[f]._u16Param2 = 194;
	_scriptFrame[f]._event = ME_MLEFT;
	_scriptFrame[f++]._u32Param = od21TO22;

	_script[s21TO23]._firstFrame = ++f;
	_script[s21TO23]._flag = 0;

	_scriptFrame[f]._class = MC_MOUSE;
	_scriptFrame[f]._u16Param1 = 317;
	_scriptFrame[f]._u8Param = 1;
	_scriptFrame[f]._u16Param2 = 166;
	_scriptFrame[f]._event = ME_MRIGHT;
	_scriptFrame[f++]._u32Param = oCATENAT21;

	_scriptFrame[f]._class = MC_MOUSE;
	_scriptFrame[f]._u16Param1 = 204;
	_scriptFrame[f]._u8Param = 1;
	_scriptFrame[f]._u16Param2 = 157;
	_scriptFrame[f]._event = ME_MLEFT;
	_scriptFrame[f++]._u32Param = od21TO23;

	_script[s24TO23]._firstFrame = ++f;
	_script[s24TO23]._flag = 0;
	_scriptFrame[f]._class = MC_MOUSE;
	_scriptFrame[f]._u16Param1 = 316;
	_scriptFrame[f]._u8Param = 1;
	_scriptFrame[f]._u16Param2 = 213;
	_scriptFrame[f]._event = ME_MRIGHT;
	_scriptFrame[f++]._u32Param = oPASSAGE24;

	_scriptFrame[f]._class = MC_MOUSE;
	_scriptFrame[f]._u16Param1 = 120;
	_scriptFrame[f]._u8Param = 1;
	_scriptFrame[f]._u16Param2 = 196;
	_scriptFrame[f]._event = ME_MLEFT;
	_scriptFrame[f++]._u32Param = od24TO23;

	_script[s24TO26]._firstFrame = ++f;
	_script[s24TO26]._flag = 0;

	_scriptFrame[f]._class = MC_MOUSE;
	_scriptFrame[f]._u16Param1 = 316;
	_scriptFrame[f]._u8Param = 1;
	_scriptFrame[f]._u16Param2 = 213;
	_scriptFrame[f]._event = ME_MRIGHT;
	_scriptFrame[f++]._u32Param = oPASSAGE24;

	_scriptFrame[f]._class = MC_MOUSE;
	_scriptFrame[f]._u16Param1 = 527;
	_scriptFrame[f]._u8Param = 1;
	_scriptFrame[f]._u16Param2 = 187;
	_scriptFrame[f]._event = ME_MLEFT;
	_scriptFrame[f++]._u32Param = od24TO26;

	_script[s2ETO2C]._firstFrame = ++f;
	_script[s2ETO2C]._flag = 0;

	_scriptFrame[f]._class = MC_MOUSE;
	_scriptFrame[f]._u16Param1 = 420;
	_scriptFrame[f]._u8Param = 1;
	_scriptFrame[f]._u16Param2 = 238;
	_scriptFrame[f]._event = ME_MRIGHT;
	_scriptFrame[f++]._u32Param = oCATWALKA2E;

	_scriptFrame[f]._class = MC_MOUSE;
	_scriptFrame[f]._u16Param1 = 66;
	_scriptFrame[f]._u8Param = 1;
	_scriptFrame[f]._u16Param2 = 336;
	_scriptFrame[f]._event = ME_MLEFT;
	_scriptFrame[f++]._u32Param = od2ETO2C;

	_script[s2ETO2F]._firstFrame = ++f;
	_script[s2ETO2F]._flag = 0;

	_scriptFrame[f]._class = MC_MOUSE;
	_scriptFrame[f]._u16Param1 = 420;
	_scriptFrame[f]._u8Param = 1;
	_scriptFrame[f]._u16Param2 = 238;
	_scriptFrame[f]._event = ME_MRIGHT;
	_scriptFrame[f++]._u32Param = oCATWALKA2E;

	_scriptFrame[f]._class = MC_MOUSE;
	_scriptFrame[f]._u16Param1 = 213;
	_scriptFrame[f]._u8Param = 1;
	_scriptFrame[f]._u16Param2 = 69;
	_scriptFrame[f]._event = ME_MLEFT;
	_scriptFrame[f++]._u32Param = oENTRANCE2E;
}

} // End of namespace Trecision
