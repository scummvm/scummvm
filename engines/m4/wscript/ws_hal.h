
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
 */

#ifndef M4_WSCRIPT_WS_HAL_H
#define M4_WSCRIPT_WS_HAL_H

#include "m4/wscript/ws_machine.h"
#include "m4/wscript/ws_cruncher.h"

namespace M4 {

#define CCB_SKIP			0x0001
#define CCB_HIDE			0x0002
#define CCB_REDRAW			0x0004
#define CCB_STREAM			0x0008
#define CCB_DISC_STREAM		0x0010

#define CCB_NO_DRAW			(CCB_SKIP | CCB_HIDE)

#define ERR_INTERNAL		0
#define ERR_SEQU			1
#define ERR_MACH			2

struct WSHal_Globals {
	RectList *_deadRectList = nullptr;
};

bool ws_InitHAL();
void ws_KillHAL();

void ws_DoDisplay(Buffer *background, int16 *depth_table, Buffer *screenCodeBuff,
	uint8 *myPalette, uint8 *ICT, bool updateVideo);
void ws_hal_RefreshWoodscriptBuffer(cruncher *myCruncher, Buffer *background,
	int16 *depth_table, Buffer *screenCodes, uint8 *myPalette, uint8 *ICT);
void GetBezCoeffs(frac16 *ctrlPoints, frac16 *coeffs);
void GetBezPoint(frac16 *x, frac16 *y, frac16 *coeffs, frac16 tVal);
bool InitCCB(CCB *myCCB);
void HideCCB(CCB *myCCB);
void ShowCCB(CCB *myCCB);
//void SetLastCCB(CCB *myCCB);
void MoveCCB(CCB *myCCB, frac16 deltaX, frac16 deltaY);
void KillCCB(CCB *myCCB, bool restoreFlag);
void Cel_msr(Anim8 *myAnim8);
void ws_OverrideCrunchTime(machine *m);

bool CheckAddr();
void ws_Error(machine *m, int32 errorType, quadchar errorCode, const char *errMsg);
void ws_DumpMachine(machine *m);
void ws_LogErrorMsg(const char *sourceFile, uint32 lineNum, const char *fmt, ...);

} // End of namespace M4

#endif
