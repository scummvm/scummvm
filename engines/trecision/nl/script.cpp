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

#include "trecision/nl/lib/addtype.h"
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

	Script[s16CARD].firstframe = f;
	Script[s16CARD].flag = 0;

	ScriptFrame[f].cls = MC_HOMO;
	ScriptFrame[f].event = ME_HOMOACTION;
	ScriptFrame[f++].wparam1 = a166USACREDITCARD;

	ScriptFrame[f].cls = MC_ACTION;
	ScriptFrame[f].event = ME_MOUSEOPERATE;
	ScriptFrame[f++].lparam = oBIGLIETTERIA16;

	Script[s16MONETA].firstframe = ++f;
	Script[s16MONETA].flag = 0;

	ScriptFrame[f].cls = MC_HOMO;
	ScriptFrame[f].event = ME_HOMOACTION;
	ScriptFrame[f++].wparam1 = a167USAMONETA;

	ScriptFrame[f].cls = MC_ACTION;
	ScriptFrame[f].event = ME_MOUSEOPERATE;
	ScriptFrame[f++].lparam = oBIGLIETTERIA16;

	Script[s19EVA].firstframe = ++f;
	Script[s19EVA].flag = 0;

	ScriptFrame[f].cls = MC_MOUSE;
	ScriptFrame[f].event = ME_MRIGHT;
	ScriptFrame[f].bparam = true;
	ScriptFrame[f].wparam1 = 214;
	ScriptFrame[f].wparam2 = TOP + 140;
	ScriptFrame[f++].lparam = ocEVA19;

	Script[s4AHELLEN].firstframe = ++f;
	Script[s4AHELLEN].flag = 0;

	ScriptFrame[f].cls = MC_HOMO;
	ScriptFrame[f].event = ME_HOMOACTION;
	ScriptFrame[f++].wparam1 = a4A2;

	ScriptFrame[f].cls = MC_MOUSE;
	ScriptFrame[f].event = ME_MLEFT;
	ScriptFrame[f].bparam = true;
	ScriptFrame[f].wparam1 = 336;
	ScriptFrame[f].wparam2 = 263 + TOP;
	ScriptFrame[f++].lparam = 0;

	ScriptFrame[f].cls = MC_MOUSE;
	ScriptFrame[f].event = ME_MLEFT;
	ScriptFrame[f].bparam = true;
	ScriptFrame[f].wparam1 = 336;
	ScriptFrame[f].wparam2 = 260 + TOP;
	ScriptFrame[f++].lparam = 0;

	ScriptFrame[f].cls = MC_DIALOG;
	ScriptFrame[f].event = ME_STARTDIALOG;
	ScriptFrame[f++].wparam1 = dF4A3;

	Script[s49MERIDIANA].firstframe = ++f;
	Script[s49MERIDIANA].flag = 0;

	ScriptFrame[f].cls = MC_SCRIPT;
	ScriptFrame[f].event = ME_CHANGER;
	ScriptFrame[f].wparam1 = r49;
	ScriptFrame[f].wparam2 = a496;
	ScriptFrame[f].bparam = true;
	ScriptFrame[f++].lparam = 1;

	ScriptFrame[f].cls = MC_MOUSE;
	ScriptFrame[f].event = ME_MLEFT;
	ScriptFrame[f].bparam = true;
	ScriptFrame[f].wparam1 = 360;
	ScriptFrame[f].wparam2 = 255 + TOP;
	ScriptFrame[f++].lparam = 0;

	ScriptFrame[f].cls = MC_SCRIPT;
	ScriptFrame[f].event = ME_CHANGER;
	ScriptFrame[f].wparam1 = r4A;
	ScriptFrame[f].wparam2 = 0;
	ScriptFrame[f].bparam = true;
	ScriptFrame[f++].lparam = 1;

	//	Fast change room in double room of level 2
	Script[s21TO22].firstframe = ++f;
	Script[s21TO22].flag = 0;

	ScriptFrame[f].cls = MC_MOUSE;
	ScriptFrame[f].wparam1 = 317;
	ScriptFrame[f].bparam = true;
	ScriptFrame[f].wparam2 = 166;
	ScriptFrame[f].event = ME_MRIGHT;
	ScriptFrame[f++].lparam = oCATENAT21;

	ScriptFrame[f].cls = MC_MOUSE;
	ScriptFrame[f].wparam1 = 429;
	ScriptFrame[f].bparam = true;
	ScriptFrame[f].wparam2 = 194;
	ScriptFrame[f].event = ME_MLEFT;
	ScriptFrame[f++].lparam = od21ALLA22;

	Script[s21TO23].firstframe = ++f;
	Script[s21TO23].flag = 0;

	ScriptFrame[f].cls = MC_MOUSE;
	ScriptFrame[f].wparam1 = 317;
	ScriptFrame[f].bparam = true;
	ScriptFrame[f].wparam2 = 166;
	ScriptFrame[f].event = ME_MRIGHT;
	ScriptFrame[f++].lparam = oCATENAT21;

	ScriptFrame[f].cls = MC_MOUSE;
	ScriptFrame[f].wparam1 = 204;
	ScriptFrame[f].bparam = true;
	ScriptFrame[f].wparam2 = 157;
	ScriptFrame[f].event = ME_MLEFT;
	ScriptFrame[f++].lparam = od21ALLA23;

	Script[s24TO23].firstframe = ++f;
	Script[s24TO23].flag = 0;
	ScriptFrame[f].cls = MC_MOUSE;
	ScriptFrame[f].wparam1 = 316;
	ScriptFrame[f].bparam = true;
	ScriptFrame[f].wparam2 = 213;
	ScriptFrame[f].event = ME_MRIGHT;
	ScriptFrame[f++].lparam = oPASSAGGIO24;

	ScriptFrame[f].cls = MC_MOUSE;
	ScriptFrame[f].wparam1 = 120;
	ScriptFrame[f].bparam = true;
	ScriptFrame[f].wparam2 = 196;
	ScriptFrame[f].event = ME_MLEFT;
	ScriptFrame[f++].lparam = od24ALLA23;

	Script[s24TO26].firstframe = ++f;
	Script[s24TO26].flag = 0;

	ScriptFrame[f].cls = MC_MOUSE;
	ScriptFrame[f].wparam1 = 316;
	ScriptFrame[f].bparam = true;
	ScriptFrame[f].wparam2 = 213;
	ScriptFrame[f].event = ME_MRIGHT;
	ScriptFrame[f++].lparam = oPASSAGGIO24;

	ScriptFrame[f].cls = MC_MOUSE;
	ScriptFrame[f].wparam1 = 527;
	ScriptFrame[f].bparam = true;
	ScriptFrame[f].wparam2 = 187;
	ScriptFrame[f].event = ME_MLEFT;
	ScriptFrame[f++].lparam = od24ALLA26;

	Script[s2ETO2C].firstframe = ++f;
	Script[s2ETO2C].flag = 0;

	ScriptFrame[f].cls = MC_MOUSE;
	ScriptFrame[f].wparam1 = 420;
	ScriptFrame[f].bparam = true;
	ScriptFrame[f].wparam2 = 238;
	ScriptFrame[f].event = ME_MRIGHT;
	ScriptFrame[f++].lparam = oCATWALKA2E;

	ScriptFrame[f].cls = MC_MOUSE;
	ScriptFrame[f].wparam1 = 66;
	ScriptFrame[f].bparam = true;
	ScriptFrame[f].wparam2 = 336;
	ScriptFrame[f].event = ME_MLEFT;
	ScriptFrame[f++].lparam = od2EALLA2C;

	Script[s2ETO2F].firstframe = ++f;
	Script[s2ETO2F].flag = 0;

	ScriptFrame[f].cls = MC_MOUSE;
	ScriptFrame[f].wparam1 = 420;
	ScriptFrame[f].bparam = false;
	ScriptFrame[f].wparam2 = 238;
	ScriptFrame[f].event = ME_MRIGHT;
	ScriptFrame[f++].lparam = oCATWALKA2E;

	ScriptFrame[f].cls = MC_MOUSE;
	ScriptFrame[f].wparam1 = 213;
	ScriptFrame[f].bparam = false;
	ScriptFrame[f].wparam2 = 69;
	ScriptFrame[f].event = ME_MLEFT;
	ScriptFrame[f++].lparam = oENTRANCE2E;
}

} // End of namespace Trecision
