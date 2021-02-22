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
#include <stdlib.h>
#include <string.h>
#include "trecision/nl/lib/addtype.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/define.h"
#include "trecision/nl/message.h"
#include "trecision/trecision.h"

#include <common/file.h>
#include <common/str.h>

namespace Common {
class File;
}

namespace Trecision {

// LOCALS
extern int16 omx, omy;

uint16 BlinkLastDTextChar = MASKCOL;
uint16 MouseBuf[50];
int	KeybInput;
extern bool _linearMode;
extern bool videoLocked;
extern int NlVer;

// joint management
extern uint16 Comb35[7], Count35;
extern uint16 Comb49[4];
extern uint16 Comb4CT[6];
extern uint16 Comb58[6], Count58;
extern uint16 ruotepos[3], ruota;
extern uint16 Try41;

/*-----------------10/12/95 15.26-------------------
						vr
--------------------------------------------------*/
uint16 vr(int16 x, int16 y) {
	uint32 b = (uint32)((uint32)(x) + (uint32)(CurRoomMaxX * (uint32)(y)));
	return ((uint16)(Video2[b]));
}

/*-----------------10/12/95 15.26-------------------
					VPix
--------------------------------------------------*/
void VPix(int16 x, int16 y, uint16 col) {
	if (_video == nullptr)
		return ;

	if (_linearMode && ((VideoPitch == 0) || (VideoPitch == SCREENLEN * 2))) {
		uint32 a = ((uint32)x + MAXX * (uint32)y);
		if ((x >= 0) && (y >= 0) && (x < CurRoomMaxX) && (y < MAXY))
			_video[a] = col;
		return ;
	}

	if ((x >= 0) && (y >= 0) && (x < CurRoomMaxX) && (y < MAXY))
		_video[x + y * (VideoPitch / 2)] = col;
}

/*-----------------10/12/95 15.27-------------------
					VMouseOFF
--------------------------------------------------*/
void VMouseOFF() {
	int32 comx = omx;

	bool vl = videoLocked;
	if (!vl)
		lockVideo();

	for (int32 i = (comx - 10); i <= (comx + 10); i++)
		VPix(i, omy, vr(i, omy));

	for (int32 i = (omy - 10); i <= (omy + 10); i++)
		VPix(comx, i, vr(comx, i));

	if (!vl)
		UnlockVideo();
}

/*-----------------10/12/95 15.29-------------------
					VMouseON
--------------------------------------------------*/
void VMouseON() {
	if (!g_vm->_mouseONOFF)
		return ;

	int32 comx = omx;
	int32 cmx = mx;
	uint16 mc = PalTo16bit(255, 255, 255);

	bool vl = videoLocked;
	if (!vl)
		lockVideo();

	for (int32 i = (comx - 10); i <= (comx + 10); i++) {
		if ((!(((i >= (cmx - 10)) && (i <= (cmx + 10))) && (omy == my))))
			VPix(i, omy, vr(i, omy));
	}

	for (int32 i = (omy - 10); i <= (omy + 10); i++) {
		if ((!(((i >= (my - 10)) && (i <= (my + 10))) && (comx == cmx))))
			VPix(comx, i, vr(comx, i));
	}

	for (int32 i = (cmx - 10); i <= (cmx - 3); i++)
		VPix(i, my, mc);
	for (int32 i = (cmx + 3); i <= (cmx + 10); i++)
		VPix(i, my, mc);

	for (int32 i = (my - 10); i <= (my - 3); i++)
		VPix(cmx, i, mc);
	for (int32 i = (my + 3); i <= (my + 10); i++)
		VPix(cmx, i, mc);

	for (int32 i = (cmx - 2); i <= (cmx + 2); i++) {
		if (cmx == i)
			i++;
		VPix(i, my, vr(i, my));
	}
	for (int32 i = (my - 2); i <= (my + 2); i++) {
		if (i == my)
			i++;
		VPix(cmx, i, vr(cmx, i));
	}

	VPix(cmx, my, mc);
	omx = mx;
	omy = my;

	if (!vl)
		UnlockVideo();
}

/*-----------------05/03/98 11.21------------------
					VMouseCopy
--------------------------------------------------*/
void VMouseRestore() {
	int32 c = 0;

	if (!g_vm->_mouseONOFF)
		return ;

	for (int32 i = (omx - 10); i <= (omx + 10); i++)
		Video2[i + omy * MAXX] = MouseBuf[c++];

	for (int32 i = (omy - 10); i <= (omy + 10); i++)
		Video2[omx + i * MAXX] = MouseBuf[c++];
}

/*-----------------05/03/98 11.21-------------------
					VMouseCopy
--------------------------------------------------*/
void VMouseCopy() {
	int32 c = 0;
	uint16 mc = PalTo16bit(255, 255, 255);

	if (!g_vm->_mouseONOFF)
		return ;

	for (int32 i = (omx - 10); i <= (omx + 10); i++)
		MouseBuf[c++] = Video2[i + omy * MAXX];
	for (int32 i = (omy - 10); i <= (omy + 10); i++)
		MouseBuf[c++] = Video2[omx + i * MAXX];

	for (int32 i = (omx - 10); i <= (omx + 10); i++) {
		if ((i != omx - 2) && (i != omx - 1) && (i != omx + 1) && (i != omx + 2))
			Video2[i + omy * MAXX] = mc;
	}

	for (int32 i = (omy - 10); i <= (omy + 10); i++) {
		if ((i != omy - 2) && (i != omy - 1) && (i != omy + 1) && (i != omy + 2))
			Video2[omx + i * MAXX] = mc;
	}
}

/*-----------------17/02/95 09.53-------------------
 TextLength - calcola lunghezza str dal car 0 a num
--------------------------------------------------*/
uint16 TextLength(const char *sign, uint16 num) {
	if (sign == nullptr)
		return 0;

	uint16 Len;
	if (num == 0)
		Len = strlen(sign);
	else
		Len = num;

	uint16 b = 0;
	for (uint16 c = 0; c < Len; c++)
		b += Font[(uint8)sign[c] * 3 + 2];

	return b;
}

void SDText::set(SDText org) {
	set(org.x, org.y, org.dx, org.dy, org.l[0], org.l[1], org.l[2], org.l[3], org.tcol, org.scol, org.sign);
}

void SDText::set(uint16 px, uint16 py, uint16 pdx, uint16 pdy, uint16 pl0, uint16 pl1, uint16 pl2, uint16 pl3, uint16 ptcol, uint16 pscol, const char *psign) {
	x = px;
	y = py;
	dx = pdx;
	dy = pdy;
	l[0] = pl0;
	l[1] = pl1;
	l[2] = pl2;
	l[3] = pl3;
	tcol = ptcol;
	scol = pscol;
	sign = psign;
}

void SDText::clear() {
	set(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, nullptr);
}
/*-------------------------------------------------------------
   checkDText - Computes and returns the dy of the given DText
--------------------------------------------------------------*/
uint16 SDText::checkDText() {
	if (sign == nullptr)
		return 0;

	uint8 curLine = 0;
	if (TextLength(sign, 0) <= dx) {
		strcpy((char *)DTextLines[curLine], sign);
		return CARHEI;
	}

	uint16 a = 0;
	uint16 tmpDy = 0;
	uint16 lastSpace = 0;
	uint16 curInit = 0;

	while (a < strlen(sign)) {
		a++;
		if (sign[a] == ' ') {
			if (TextLength(sign + curInit, a - curInit) <= dx)
				lastSpace = a;
			else if (TextLength(sign + curInit, lastSpace - curInit) <= dx) {
				uint16 b;
				for (b = curInit; b < lastSpace; b++)
					DTextLines[curLine][b - curInit] = sign[b];

				DTextLines[curLine][b - curInit] = '\0';
				curLine++;

				curInit = lastSpace + 1;

				tmpDy += CARHEI;
				a = curInit;
			} else
				return 0;
		} else if (sign[a] == '\0') {
			if (TextLength(sign + curInit, a - curInit) <= dx) {
				uint16 b;
				for (b = curInit; b < a; b++)
					DTextLines[curLine][b - curInit] = sign[b];
				DTextLines[curLine][b - curInit] = '\0';

				tmpDy += CARHEI;

				return tmpDy;
			}

			if (TextLength(sign + curInit, lastSpace - curInit) <= dx) {
				uint16 b;
				for (b = curInit; b < lastSpace; b++)
					DTextLines[curLine][b - curInit] = sign[b];

				DTextLines[curLine][b - curInit] = '\0';
				curLine++;

				curInit = lastSpace + 1;
				tmpDy += CARHEI;

				if (curInit < strlen(sign)) {
					for (b = curInit; b < strlen(sign); b++)
						DTextLines[curLine][b - curInit] = sign[b];

					DTextLines[curLine][b - curInit] = '\0';

					tmpDy += CARHEI;
				}
				return tmpDy;
			}
			return 0;
		}
	}
	return 0;
}

/*-----------------10/12/95 15.43-------------------
						DText
--------------------------------------------------*/
void SDText::DText() {
	uint16 tmpTCol = tcol;
	uint16 tmpSCol = scol;
	UpdatePixelFormat(&tmpTCol, 1);
	if (scol != MASKCOL)
		UpdatePixelFormat(&tmpSCol, 1);

	if (sign == nullptr)
		return;

	uint16 curDy = checkDText();

	for (uint16 b = 0; b < (curDy / CARHEI); b++) {
		char *curSign = (char *)DTextLines[b];

		uint16 inc = (dx - TextLength(curSign, 0)) / 2;

		uint16 len = strlen(curSign);

		if (len >= MAXCHARS) {
			strcpy(curSign, g_vm->_sysText[19]);
			len = strlen(curSign);
		}

		for (uint16 c = 0; c < len; c++) {
			uint8 curCar = curSign[c]; /* legge prima parte del font */

			uint16 carSco = (uint16)(Font[curCar * 3]) + (uint16)(Font[curCar * 3 + 1] << 8);
			uint16 DataSco = 768;                       /* Scostamento */
			uint16 CarWid = Font[curCar * 3 + 2];        /* Larghezza   */

			if ((c == (len - 1)) && (BlinkLastDTextChar != MASKCOL))
				tmpTCol = BlinkLastDTextChar;

			for (uint16 a = (b * CARHEI); a < ((b + 1)*CARHEI); a++) {
				uint16 CarCounter = 0;
				uint16 CurColor = tmpSCol;

				while (CarCounter <= (CarWid - 1)) {
					if ((a >= l[1]) && (a < l[3])) {
						if ((CurColor != MASKCOL) && (Font[carSco + DataSco])) {
							uint16 FirstLim = inc + CarCounter;
							uint16 EndLim = FirstLim + Font[carSco + DataSco];

							if ((FirstLim >= l[0]) && (EndLim < l[2]))
								wordset(Video2 + (x + FirstLim) + (y + a)*CurRoomMaxX, CurColor, EndLim - FirstLim);
							else if ((FirstLim < l[0]) && (EndLim < l[2]) && (EndLim > l[0]))
								wordset(Video2 + (x + l[0]) + (y + a)*CurRoomMaxX, CurColor, EndLim - l[0]);
							else if ((FirstLim >= l[0]) && (EndLim >= l[2]) && (l[2] > FirstLim))
								wordset(Video2 + (x + FirstLim) + (y + a)*CurRoomMaxX, CurColor, l[2] - FirstLim);
							else if ((FirstLim < l[0]) && (EndLim >= l[2]) && (l[2] > FirstLim))
								wordset(Video2 + (x + l[0]) + (y + a)*CurRoomMaxX, CurColor, l[2] - l[0]);
						}
					}

					CarCounter += Font[carSco + DataSco];
					DataSco++;

					if (CurColor == tmpSCol)
						CurColor = 0;
					else if (CurColor == 0)
						CurColor = tmpTCol;
					else if (CurColor == tmpTCol)
						CurColor = tmpSCol;
				}
			}
			inc += CarWid;
		}
	}
}

#define MAXSAVEFILE		12
/* -----------------21/01/98 16.38-------------------
 * 						IconSnapShot
 * --------------------------------------------------*/
void IconSnapShot() {
	uint16 blackPixel = 0x0000;
	int a;

	for (int b = 0; b < ICONDY; b++) {
		for (a = 0; a < (ICONDX - 1); a++)
			Icone[(READICON + 13)*ICONDX * ICONDY + b * ICONDX + a] = (uint16)UnUpdatePixelFormat(Video2[CurRoomMaxX * b * 10 + a * (CurRoomMaxX / ICONDX)]);
		Icone[(READICON + 13)*ICONDX * ICONDY + b * ICONDX + a] = blackPixel;
	}
}

/* -----------------25/10/97 15.16-------------------
						DataSave
 --------------------------------------------------*/
bool DataSave() {
	FILE *fh;
	uint8 OldInv[MAXICON], OldIconBase, OldInvLen;
	char tempname[20], ch, strcount;
	int8 CurPos, OldPos;
	char savename[MAXSAVEFILE][40];
	uint16 posx, LenText;
	bool ret = true;

	actorStop();
	nextStep();

	for (int a = 0; a < TOP; a++)
		wordset(Video2 + CurRoomMaxX * a + CurScrollPageDx, 0, SCREENLEN);

	SDText SText;
	SText.set(CurScrollPageDx, TOP - 20, SCREENLEN, CARHEI, 0, 0, SCREENLEN, CARHEI, 0x7FFF, MASKCOL, g_vm->_sysText[9]);
	SText.DText();

	ShowScreen(0, 0, MAXX, TOP);

	for (int a = TOP + AREA; a < AREA + 2 * TOP; a++)
		wordset(Video2 + CurRoomMaxX * a + CurScrollPageDx, 0, SCREENLEN);
	ShowScreen(0, TOP + AREA, MAXX, TOP);

	g_vm->_gameQueue.initQueue();
	g_vm->_animQueue.initQueue();
	g_vm->_characterQueue.initQueue();

	FreeKey();

	mleft = mright = 0;
	Mouse(3);
	while (mleft || mright)
		Mouse(3);

	// ferma omino, animazioni, spegne scritte
	memcpy(OldInv, g_vm->_inventory, MAXICON);
	memset(g_vm->_inventory, 0, MAXICON);

	OldIconBase = g_vm->_iconBase;
	g_vm->_iconBase = 0;
	OldInvLen = g_vm->_inventorySize;
	g_vm->_inventorySize = MAXSAVEFILE;

insave:

	CurPos = -1;
	OldPos = -1;

	for (int a = 0; a < g_vm->_inventorySize; a++) {
		strcpy(tempname, "SaveGame._X_");
		tempname[10] = 'A' + a;

		// Check the existence of savegames
		if ((fh = fopen(tempname, "rb")) && (fgetc(fh) == NlVer)) {
			fread(&savename[a], 1, 40, fh);
			fread(Icone + (READICON + 1 + a)*ICONDX * ICONDY, 2, ICONDX * ICONDY, fh);
			fclose(fh);
			UpdatePixelFormat(Icone + (READICON + 1 + a)*ICONDX * ICONDY, ICONDX * ICONDY);

			g_vm->_inventory[a] = LASTICON + a;
		} else {
			strcpy(savename[a], g_vm->_sysText[10]);
			if (fh)
				fclose(fh);
			g_vm->_inventory[a] = iEMPTYSLOT;
		}
	}

	RegenInventory(0, 0);

	bool skipSave = false;
	ch = 0;
	for (; ;) {
		CheckSystem();
		Mouse(3);

		GetKey();

		if ((my >= FIRSTLINE) &&
				(my < (FIRSTLINE + ICONDY)) &&
				(mx >= ICONMARGSX) &&
				(mx < (SCREENLEN - ICONMARGDX))) {
			OldPos = CurPos;
			CurPos = ((mx - ICONMARGSX) / ICONDX);

			if (OldPos != CurPos) {
				for (int a = FIRSTLINE + ICONDY + 10; a < FIRSTLINE + ICONDY + 10 + CARHEI; a++)
					wordset(Video2 + CurRoomMaxX * a + CurScrollPageDx, 0, SCREENLEN);

				posx    = ICONMARGSX + ((CurPos) * (ICONDX)) + ICONDX / 2;
				LenText  = TextLength(savename[CurPos], 0);
				if (posx - (LenText / 2) < 2)
					posx = 2;
				else
					posx = posx - (LenText / 2);
				if ((posx + LenText) > SCREENLEN - 2)
					posx = SCREENLEN - 2 - LenText;

				SText.set(posx, FIRSTLINE + ICONDY + 10, LenText, CARHEI, 0, 0, LenText, CARHEI, 0x7FFF, MASKCOL, savename[CurPos]);
				SText.DText();

				ShowScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
			}

			if (mleft)
				break;
		} else {
			if (OldPos != -1) {
				for (int a = FIRSTLINE + ICONDY + 10; a < FIRSTLINE + ICONDY + 10 + CARHEI; a++)
					wordset(Video2 + CurRoomMaxX * a + CurScrollPageDx, 0, SCREENLEN);

				ShowScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
			}

			OldPos = -1;
			CurPos = -1;

			if (mleft || mright) {
				skipSave = true;
				break;
			}
		}
	}

	if (!skipSave) {
		if (g_vm->_inventory[CurPos] != iEMPTYSLOT)
			strcount = strlen(savename[CurPos]);
		else {
			strcount = 0;
			savename[CurPos][0] = '\0';

			for (int a = FIRSTLINE + ICONDY + 10; a < FIRSTLINE + ICONDY + 10 + CARHEI; a++)
				wordset(Video2 + CurRoomMaxX * a + CurScrollPageDx, 0, SCREENLEN);

			ShowScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
		}

		for (; ;) {
			KeybInput = true;
			CheckSystem();
			ch = GetKey();
			FreeKey();

			Mouse(3);
			KeybInput = false;

			if (ch == 0x1B) {
				ch = 0;
				for (int a = FIRSTLINE + ICONDY + 10; a < FIRSTLINE + ICONDY + 10 + CARHEI; a++)
					wordset(Video2 + CurRoomMaxX * a + CurScrollPageDx, 0, SCREENLEN);

				ShowScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);

				goto insave;
			}

			if (ch == 0x08) {
				if (strcount > 0)
					savename[CurPos][--strcount] = '\0';
				ch = 0;
			} else if (ch == 0x0D)
				break;
			else if ((strcount < 39) && (((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')) || ((ch >= '0') && (ch <= '9')) || (ch == 0x20))) {
				savename[CurPos][strcount++] = ch;
				savename[CurPos][strcount] = '\0';
				ch = 0;
			} else
				ch = 0;

			for (int a = FIRSTLINE + ICONDY + 10; a < FIRSTLINE + ICONDY + 10 + CARHEI; a++)
				wordset(Video2 + CurRoomMaxX * a + CurScrollPageDx, 0, SCREENLEN);

			int endStr = strlen(savename[CurPos]);
			savename[CurPos][endStr] = '_';
			savename[CurPos][endStr + 1] = '\0';

			posx    = ICONMARGSX + ((CurPos) * (ICONDX)) + ICONDX / 2 ;
			LenText  = TextLength(savename[CurPos], 0);
			if (posx - (LenText / 2) < 2)
				posx = 2;
			else
				posx = posx - (LenText / 2);
			if ((posx + LenText) > SCREENLEN - 2)
				posx = SCREENLEN - 2 - LenText;

			SText.set(posx, FIRSTLINE + ICONDY + 10, LenText, CARHEI, 0, 0, LenText, CARHEI, 0x7FFF, MASKCOL, savename[CurPos]);

			if ((ReadTime() / 8) & 1)
				BlinkLastDTextChar = 0x0000;

			SText.DText();
			BlinkLastDTextChar = MASKCOL;

			endStr = strlen(savename[CurPos]);
			savename[CurPos][endStr - 1] = '\0';

			ShowScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
		}

		for (int a = FIRSTLINE; a < MAXY; a++)
			wordset(Video2 + CurRoomMaxX * a + CurScrollPageDx, 0, SCREENLEN);

		strcpy(tempname, "SaveGame._X_");
		tempname[10] = 'A' + CurPos;
		tempname[12] = '\0';
		ret = false;

		fh = fopen(tempname, "wb");

		fputc(NlVer, fh);
		fwrite(&savename[CurPos], 1, 40, fh);

		fwrite(Icone + (READICON + 13)*ICONDX * ICONDY, 2, ICONDX * ICONDY, fh);

		fwrite(&g_vm->_curRoom, sizeof(uint16), 1, fh);
		fwrite(&OldInvLen,           sizeof(uint8), 1, fh);
		fwrite(&g_vm->_cyberInventorySize, sizeof(uint8), 1, fh);
		fwrite(&OldIconBase,         sizeof(uint8), 1, fh);
		fwrite(&Flagskiptalk,         sizeof(int16), 1, fh);
		fwrite(&Flagskipenable,       sizeof(int16), 1, fh);
		fwrite(&FlagMouseEnabled,     sizeof(int16), 1, fh);
		fwrite(&FlagScreenRefreshed,  sizeof(int16), 1, fh);
		fwrite(&FlagPaintCharacter,        sizeof(int16), 1, fh);
		fwrite(&FlagSomeOneSpeak,     sizeof(int16), 1, fh);
		fwrite(&FlagCharacterSpeak,        sizeof(int16), 1, fh);
		fwrite(&FlagInventoryLocked,  sizeof(int16), 1, fh);
		fwrite(&FlagUseWithStarted,   sizeof(int16), 1, fh);
		fwrite(&FlagMousePolling,     sizeof(int16), 1, fh);
		fwrite(&FlagDialogSolitaire,  sizeof(int16), 1, fh);
		fwrite(&FlagCharacterExist,        sizeof(int16), 1, fh);

		fwrite(&OldInv[0],           sizeof(uint8), MAXICON, fh);
		fwrite(&g_vm->_cyberInventory[0], sizeof(uint8), MAXICON, fh);

		fwrite(&_actor._px,              sizeof(float), 1, fh);
		fwrite(&_actor._py,              sizeof(float), 1, fh);
		fwrite(&_actor._pz,              sizeof(float), 1, fh);
		fwrite(&_actor._dx,              sizeof(float), 1, fh);
		fwrite(&_actor._dz,              sizeof(float), 1, fh);
		fwrite(&_actor._theta,           sizeof(float), 1, fh);
		fwrite(&_curPanel,            sizeof(int),   1, fh);
		fwrite(&_oldPanel,            sizeof(int),   1, fh);

		for (int a = 0; a < MAXROOMS; a++) {
			fwrite(g_vm->_room[a]._baseName, sizeof(char), 4, fh);
			fwrite(g_vm->_room[a]._actions, sizeof(uint16), MAXACTIONINROOM, fh);
			fwrite(&g_vm->_room[a]._flag, sizeof(uint8), 1, fh);
			fwrite(&g_vm->_room[a]._bkgAnim, sizeof(uint16), 1, fh);
		}
		for (int a = 0; a < MAXOBJ; a++) {
			fwrite(&g_vm->_obj[a]._lim, sizeof(uint16), 4, fh);
			fwrite(&g_vm->_obj[a]._name, sizeof(uint16), 1, fh);
			fwrite(&g_vm->_obj[a]._examine, sizeof(uint16), 1, fh);
			fwrite(&g_vm->_obj[a]._action, sizeof(uint16), 1, fh);
			fwrite(&g_vm->_obj[a]._anim, sizeof(uint16), 1, fh);
			fwrite(&g_vm->_obj[a]._mode, sizeof(uint8), 1, fh);
			fwrite(&g_vm->_obj[a]._flag, sizeof(uint8), 1, fh);
			fwrite(&g_vm->_obj[a]._goRoom, sizeof(uint8), 1, fh);
			fwrite(&g_vm->_obj[a]._nbox, sizeof(uint8), 1, fh);
			fwrite(&g_vm->_obj[a]._ninv, sizeof(uint8), 1, fh);
			fwrite(&g_vm->_obj[a]._position, sizeof(int8), 1, fh);
		}
		for (int a = 0; a < MAXINVENTORY; a++) {
			fwrite(&g_vm->_inventoryObj[a]._name, sizeof(uint16), 1, fh);
			fwrite(&g_vm->_inventoryObj[a]._examine, sizeof(uint16), 1, fh);
			fwrite(&g_vm->_inventoryObj[a]._action, sizeof(uint16), 1, fh);
			fwrite(&g_vm->_inventoryObj[a]._anim, sizeof(uint16), 1, fh);
			fwrite(&g_vm->_inventoryObj[a]._flag, sizeof(uint8), 1, fh);
		}
		for (int a = 0; a < MAXANIM; a++)
			fwrite(&AnimTab[a],        sizeof(struct SAnim), 1, fh);
		for (int a = 0; a < MAXSAMPLE; a++) {
			fwrite(&GSample[a]._volume,  sizeof(uint8), 1, fh);
			fwrite(&GSample[a]._flag,    sizeof(uint8), 1, fh);
		}
		for (int a = 0; a < MAXCHOICE; a++)
			fwrite(&_choice[a],           sizeof(DialogChoice), 1, fh);

		for (int a = 0; a < MAXDIALOG; a++)
			fwrite(&_dialog[a],          sizeof(Dialog), 1, fh);

		fwrite(&Comb35,      sizeof(uint16), 7, fh);
		fwrite(&Comb49,      sizeof(uint16), 4, fh);
		fwrite(&Comb4CT,     sizeof(uint16), 6, fh);
		fwrite(&Comb58,      sizeof(uint16), 6, fh);
		fwrite(&ruotepos,    sizeof(uint16), 3, fh);
		fwrite(&ruota,       sizeof(uint16), 1, fh);
		fwrite(&Count35,     sizeof(uint16), 1, fh);
		fwrite(&Count58,     sizeof(uint16), 1, fh);
		fwrite(&Try41,       sizeof(uint16), 1, fh);

		fclose(fh);
	}

	for (int a = FIRSTLINE; a < MAXY; a++)
		wordset(Video2 + CurRoomMaxX * a + CurScrollPageDx, 0, SCREENLEN);

	ShowScreen(0, FIRSTLINE, MAXX, TOP);

	for (int a = TOP - 20; a < TOP - 20 + CARHEI; a++)
		wordset(Video2 + CurRoomMaxX * a + CurScrollPageDx, 0, SCREENLEN);

	ShowScreen(0, 0, MAXX, TOP);
	g_vm->_curInventory = 0;

	memcpy(g_vm->_inventory, OldInv, MAXICON);

	g_vm->_iconBase = OldIconBase;
	g_vm->_inventorySize = OldInvLen;

	UnlockVideo();

	mleft = mright = 0;
	Mouse(3);
	while (mleft || mright)
		Mouse(3);

	return ret;
}

/*-----------------09/02/96 20.57-------------------
					DataLoad
--------------------------------------------------*/
bool DataLoad() {
	extern char CurCDSet;
	extern uint8 CurStack;
	FILE *fh;
	uint8 OldInv[MAXICON], OldIconBase, OldInvLen;
	char tempname[20];
	char savename[MAXSAVEFILE][40];
	bool retval = true;

	for (int a = 0; a < TOP; a++)
		wordset(Video2 + CurRoomMaxX * a + CurScrollPageDx, 0, SCREENLEN);

	if (!FlagMouseEnabled) {
		FlagMouseEnabled = true;
		Mouse(1);
	}

	SDText SText;
	SText.set(CurScrollPageDx, TOP - 20, SCREENLEN, CARHEI, 0, 0, SCREENLEN, CARHEI, 0x7FFF, MASKCOL, g_vm->_sysText[11]);
	SText.DText();

	ShowScreen(0, 0, MAXX, TOP);

	for (int a = TOP + AREA; a < AREA + 2 * TOP; a++)
		wordset(Video2 + CurRoomMaxX * a + CurScrollPageDx, 0, SCREENLEN);
	ShowScreen(0, TOP + AREA, MAXX, TOP);

	g_vm->_gameQueue.initQueue();
	g_vm->_animQueue.initQueue();
	g_vm->_characterQueue.initQueue();

	FreeKey();

	mleft = mright = 0;
	Mouse(3);
	while (mleft || mright)
		Mouse(3);

	// Stop character and animations, turn off writings

	memcpy(OldInv, g_vm->_inventory, MAXICON);
	memset(g_vm->_inventory, 0, MAXICON);

	OldIconBase = g_vm->_iconBase;
	g_vm->_iconBase = 0;
	OldInvLen = g_vm->_inventorySize;
	g_vm->_inventorySize = MAXSAVEFILE;
	int8 CurPos = -1;
	int8 OldPos = -1;

	for (int a = 0; a < g_vm->_inventorySize; a++) {
		strcpy(tempname, "SaveGame._X_");
		tempname[10] = 'A' + a;

		// Check if savegames exist
		if ((fh = fopen(tempname, "rb")) && (fgetc(fh) == NlVer)) {
			fread(&savename[a], 1, 40, fh);
			fread(Icone + (READICON + 1 + a)*ICONDX * ICONDY, 2, ICONDX * ICONDY, fh);
			fclose(fh);
			UpdatePixelFormat(Icone + (READICON + 1 + a)*ICONDX * ICONDY, ICONDX * ICONDY);

			g_vm->_inventory[a] = LASTICON + a;
		} else {
			strcpy(savename[a], g_vm->_sysText[10]);
			if (fh)
				fclose(fh);
			fh = NULL;
			g_vm->_inventory[a] = iEMPTYSLOT;
		}
	}

	RegenInventory(0, 0);

	bool skipLoad = false;

	for (;;) {
		CheckSystem();
		Mouse(3);

		GetKey();

		if ((my >= FIRSTLINE) &&
				(my < (FIRSTLINE + ICONDY)) &&
				(mx >= ICONMARGSX) &&
				(mx < (SCREENLEN - ICONMARGDX))) {
			OldPos = CurPos;
			CurPos = ((mx - ICONMARGSX) / ICONDX);

			if (OldPos != CurPos) {
				for (int a = FIRSTLINE + ICONDY + 10; a < FIRSTLINE + ICONDY + 10 + CARHEI; a++)
					wordset(Video2 + CurRoomMaxX * a + CurScrollPageDx, 0, SCREENLEN);

				uint16 posX = ICONMARGSX + ((CurPos) * (ICONDX)) + ICONDX / 2;
				uint16 lenText = TextLength(savename[CurPos], 0);
				if (posX - (lenText / 2) < 2)
					posX = 2;
				else
					posX = posX - (lenText / 2);
				if ((posX + lenText) > SCREENLEN - 2)
					posX = SCREENLEN - 2 - lenText;

				SText.set(posX, FIRSTLINE + ICONDY + 10, lenText, CARHEI, 0, 0, lenText, CARHEI, 0x7FFF, MASKCOL, savename[CurPos]);
				SText.DText();

				ShowScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
			}

			if ((mleft) && (g_vm->_inventory[CurPos] != iEMPTYSLOT))
				break;
		} else {
			if (OldPos != -1) {
				for (int a = FIRSTLINE + ICONDY + 10; a < FIRSTLINE + ICONDY + 10 + CARHEI; a++)
					wordset(Video2 + CurRoomMaxX * a + CurScrollPageDx, 0, SCREENLEN);

				ShowScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
			}

			OldPos = -1;
			CurPos = -1;

			if ((mleft) || (mright)) {
				retval = false;
				skipLoad = true;
				break;
			}
		}
	}

	if (!skipLoad) {
		for (int a = FIRSTLINE; a < MAXY; a++)
			wordset(Video2 + CurRoomMaxX * a + CurScrollPageDx, 0, SCREENLEN);

		strcpy(tempname, "SaveGame._X_");
		tempname[10] = 'A' + CurPos;
		tempname[12] = '\0';

		fh = fopen(tempname, "rb");
		if (!fh) {
			CloseSys(g_vm->_sysText[12]);
			return false;
		}

		fgetc(fh);
		fread(&savename[0], 1, 40, fh);
		fread(Icone + (READICON + 1)*ICONDX * ICONDY, 2, ICONDX * ICONDY, fh);
		UpdatePixelFormat(Icone + (READICON + 1)*ICONDX * ICONDY, ICONDX * ICONDY);

		fread(&g_vm->_curRoom, sizeof(uint16), 1, fh);
		fread(&OldInvLen,           sizeof(uint8), 1, fh);
		fread(&g_vm->_cyberInventorySize, sizeof(uint8), 1, fh);
		fread(&OldIconBase,         sizeof(uint8), 1, fh);
		fread(&Flagskiptalk,         sizeof(int16), 1, fh);
		fread(&Flagskipenable,       sizeof(int16), 1, fh);
		fread(&FlagMouseEnabled,     sizeof(int16), 1, fh);
		fread(&FlagScreenRefreshed,  sizeof(int16), 1, fh);
		fread(&FlagPaintCharacter,        sizeof(int16), 1, fh);
		fread(&FlagSomeOneSpeak,     sizeof(int16), 1, fh);
		fread(&FlagCharacterSpeak,        sizeof(int16), 1, fh);
		fread(&FlagInventoryLocked,  sizeof(int16), 1, fh);
		fread(&FlagUseWithStarted,   sizeof(int16), 1, fh);
		fread(&FlagMousePolling,     sizeof(int16), 1, fh);
		fread(&FlagDialogSolitaire,  sizeof(int16), 1, fh);
		fread(&FlagCharacterExist,        sizeof(int16), 1, fh);

		fread(&OldInv[0],           sizeof(uint8), MAXICON, fh);
		fread(&g_vm->_cyberInventory[0], sizeof(uint8), MAXICON, fh);

		fread(&_actor._px,              sizeof(float), 1, fh);
		fread(&_actor._py,              sizeof(float), 1, fh);
		fread(&_actor._pz,              sizeof(float), 1, fh);
		fread(&_actor._dx,              sizeof(float), 1, fh);
		fread(&_actor._dz,              sizeof(float), 1, fh);
		fread(&_actor._theta,           sizeof(float), 1, fh);
		fread(&_curPanel,            sizeof(int),   1, fh);
		fread(&_oldPanel,            sizeof(int),   1, fh);

		for (int a = 0; a < MAXROOMS; a++) {
			fread(g_vm->_room[a]._baseName, sizeof(char), 4, fh);
			fread(g_vm->_room[a]._actions, sizeof(uint16), MAXACTIONINROOM, fh);
			fread(&g_vm->_room[a]._flag, sizeof(uint8), 1, fh);
			fread(&g_vm->_room[a]._bkgAnim, sizeof(uint16), 1, fh);
		}
		for (int a = 0; a < MAXOBJ; a++) {
			fread(&g_vm->_obj[a]._lim, sizeof(uint16), 4, fh);
			fread(&g_vm->_obj[a]._name, sizeof(uint16), 1, fh);
			fread(&g_vm->_obj[a]._examine, sizeof(uint16), 1, fh);
			fread(&g_vm->_obj[a]._action, sizeof(uint16), 1, fh);
			fread(&g_vm->_obj[a]._anim, sizeof(uint16), 1, fh);
			fread(&g_vm->_obj[a]._mode, sizeof(uint8), 1, fh);
			fread(&g_vm->_obj[a]._flag, sizeof(uint8), 1, fh);
			fread(&g_vm->_obj[a]._goRoom, sizeof(uint8), 1, fh);
			fread(&g_vm->_obj[a]._nbox, sizeof(uint8), 1, fh);
			fread(&g_vm->_obj[a]._ninv, sizeof(uint8), 1, fh);
			fread(&g_vm->_obj[a]._position, sizeof(int8), 1, fh);
		}
		for (int a = 0; a < MAXINVENTORY; a++) {
			fread(&g_vm->_inventoryObj[a]._name, sizeof(uint16), 1, fh);
			fread(&g_vm->_inventoryObj[a]._examine, sizeof(uint16), 1, fh);
			fread(&g_vm->_inventoryObj[a]._action, sizeof(uint16), 1, fh);
			fread(&g_vm->_inventoryObj[a]._anim, sizeof(uint16), 1, fh);
			fread(&g_vm->_inventoryObj[a]._flag, sizeof(uint8), 1, fh);
		}
		for (int a = 0; a < MAXANIM; a++)
			fread(&AnimTab[a],        sizeof(struct SAnim), 1, fh);
		for (int a = 0; a < MAXSAMPLE; a++) {
			fread(&GSample[a]._volume,  sizeof(uint8), 1, fh);
			fread(&GSample[a]._flag,    sizeof(uint8), 1, fh);
		}

		for (int a = 0; a < MAXCHOICE; a++)
			fread(&_choice[a],           sizeof(DialogChoice), 1, fh);

		for (int a = 0; a < MAXDIALOG; a++)
			fread(&_dialog[a],          sizeof(Dialog), 1, fh);

		fread(&Comb35,      sizeof(uint16), 7, fh);
		fread(&Comb49,      sizeof(uint16), 4, fh);
		fread(&Comb4CT,     sizeof(uint16), 6, fh);
		fread(&Comb58,      sizeof(uint16), 6, fh);
		fread(&ruotepos,    sizeof(uint16), 3, fh);
		fread(&ruota,       sizeof(uint16), 1, fh);
		fread(&Count35,     sizeof(uint16), 1, fh);
		fread(&Count58,     sizeof(uint16), 1, fh);
		fread(&Try41,       sizeof(uint16), 1, fh);

		fclose(fh);

		FlagNoPaintScreen = true;
		CurStack = 0;
		Flagscriptactive = false;

		g_vm->_oldRoom = g_vm->_curRoom;
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_curRoom, 0, 0, 0);

		// Set CD preference
		if ((CurCDSet != 1) && ((g_vm->_curRoom == r11) || (g_vm->_curRoom == r12) || (g_vm->_curRoom == r13) || (g_vm->_curRoom == r14) || (g_vm->_curRoom == r15) || (g_vm->_curRoom == r16) || (g_vm->_curRoom == r17) || (g_vm->_curRoom == r18) || (g_vm->_curRoom == r19) || (g_vm->_curRoom == r1A) || (g_vm->_curRoom == r1B) || (g_vm->_curRoom == r1C) || (g_vm->_curRoom == r1D) || (g_vm->_curRoom == rINTRO) || (g_vm->_curRoom == rSYS) || (g_vm->_curRoom == r12CU) || (g_vm->_curRoom == r13CU)))
			CheckFileInCD("11.bkg");
		else if ((CurCDSet != 2) && ((g_vm->_curRoom == r21) || (g_vm->_curRoom == r22) || (g_vm->_curRoom == r23A) || (g_vm->_curRoom == r24) || (g_vm->_curRoom == r25) || (g_vm->_curRoom == r26) || (g_vm->_curRoom == r27) || (g_vm->_curRoom == r28) || (g_vm->_curRoom == r29) || (g_vm->_curRoom == r2A) || (g_vm->_curRoom == r2B) || (g_vm->_curRoom == r2C) || (g_vm->_curRoom == r2D) || (g_vm->_curRoom == r2E) || (g_vm->_curRoom == r2F) || (g_vm->_curRoom == r2G) || (g_vm->_curRoom == r2H) || (g_vm->_curRoom == r31) || (g_vm->_curRoom == r32) || (g_vm->_curRoom == r33) || (g_vm->_curRoom == r34) || (g_vm->_curRoom == r35) || (g_vm->_curRoom == r36) || (g_vm->_curRoom == r37) || (g_vm->_curRoom == r23B) || (g_vm->_curRoom == r29L) || (g_vm->_curRoom == r2BL) || (g_vm->_curRoom == r2GV) || (g_vm->_curRoom == r31P) || (g_vm->_curRoom == r35P) || (g_vm->_curRoom == r36F)))
			CheckFileInCD("33.bkg");
		else if ((CurCDSet != 3) && ((g_vm->_curRoom == r41) || (g_vm->_curRoom == r42) || (g_vm->_curRoom == r43) || (g_vm->_curRoom == r44) || (g_vm->_curRoom == r45) || (g_vm->_curRoom == r46) || (g_vm->_curRoom == r47) || (g_vm->_curRoom == r48) || (g_vm->_curRoom == r49) || (g_vm->_curRoom == r4A) || (g_vm->_curRoom == r4B) || (g_vm->_curRoom == r4C) || (g_vm->_curRoom == r4D) || (g_vm->_curRoom == r4E) || (g_vm->_curRoom == r4F) || (g_vm->_curRoom == r4G) || (g_vm->_curRoom == r4H) || (g_vm->_curRoom == r4I) || (g_vm->_curRoom == r4J) || (g_vm->_curRoom == r4K) || (g_vm->_curRoom == r4L) || (g_vm->_curRoom == r4M) || (g_vm->_curRoom == r4N) || (g_vm->_curRoom == r4O) || (g_vm->_curRoom == r4P) || (g_vm->_curRoom == r4Q) || (g_vm->_curRoom == r4R) || (g_vm->_curRoom == r4S) || (g_vm->_curRoom == r4T) || (g_vm->_curRoom == r4U) || (g_vm->_curRoom == r4V) || (g_vm->_curRoom == r4W) || (g_vm->_curRoom == r4X) || (g_vm->_curRoom == r51) || (g_vm->_curRoom == r52) || (g_vm->_curRoom == r53) || (g_vm->_curRoom == r54) || (g_vm->_curRoom == r55) || (g_vm->_curRoom == r56) || (g_vm->_curRoom == r57) || (g_vm->_curRoom == r58) || (g_vm->_curRoom == r59) || (g_vm->_curRoom == r5A) || (g_vm->_curRoom == r41D) || (g_vm->_curRoom == r45S) || (g_vm->_curRoom == r49M) || (g_vm->_curRoom == r4CT) || (g_vm->_curRoom == r58M) || (g_vm->_curRoom == r58T) || (g_vm->_curRoom == r59L)))
			CheckFileInCD("51.bkg");
	}

	actorStop();
	nextStep();
	CheckSystem();

	for (int a = FIRSTLINE; a < MAXY; a++)
		wordset(Video2 + CurRoomMaxX * a + CurScrollPageDx, 0, SCREENLEN);

	ShowScreen(0, FIRSTLINE, MAXX, TOP);

	for (int a = TOP - 20; a < TOP - 20 + CARHEI; a++)
		wordset(Video2 + CurRoomMaxX * a + CurScrollPageDx, 0, SCREENLEN);

	ShowScreen(0, 0, MAXX, TOP);

	memcpy(g_vm->_inventory, OldInv, MAXICON);

	g_vm->_curInventory = 0;
	g_vm->_iconBase = OldIconBase;
	g_vm->_inventorySize = OldInvLen;

	mleft = mright = 0;
	Mouse(3);
	while (mleft || mright)
		Mouse(3);

	if (Flagscriptactive) {
		FlagMouseEnabled = false;
		Mouse(2);
	}

	return retval;
}

/*-----------------09/02/96 20.57-------------------
					QuitGame
--------------------------------------------------*/
bool QuitGame() {
	for (int a = 0; a < TOP; a++)
		memcpy(ZBuffer + a * CurRoomMaxX, Video2 + CurRoomMaxX * a + CurScrollPageDx, SCREENLEN * 2);

	for (int a = 0; a < TOP; a++)
		wordset(Video2 + CurRoomMaxX * a + CurScrollPageDx, 0, SCREENLEN);

	SDText SText;
	SText.set(CurScrollPageDx, TOP - 20, SCREENLEN, CARHEI, 0, 0, SCREENLEN, CARHEI, 0x7FFF, MASKCOL, g_vm->_sysText[13]);
	SText.DText();

	for (int a = 0; a < TOP; a++)
		VCopy(a * VirtualPageLen + VideoScrollPageDx, Video2 + a * CurRoomMaxX + CurScrollPageDx, SCREENLEN);

	UnlockVideo();

	FreeKey();

	mleft = mright = 0;
	Mouse(3);
	while (mleft || mright)
		Mouse(3);

	char ch = waitKey();

	bool exitFl = ((ch == 'y') || (ch == 'Y'));

	for (int a = 0; a < TOP; a++)
		memcpy(Video2 + CurRoomMaxX * a + CurScrollPageDx, ZBuffer + a * CurRoomMaxX, SCREENLEN * 2);

	for (int a = 0; a < TOP; a++)
		VCopy(a * VirtualPageLen + VideoScrollPageDx, Video2 + a * CurRoomMaxX + CurScrollPageDx, SCREENLEN);

	UnlockVideo();

	return exitFl;
}

/*-----------------09/02/96 20.57-------------------
					DemoOver
--------------------------------------------------*/
void DemoOver() {
	for (int a = 0; a < TOP; a++)
		wordset(Video2 + CurRoomMaxX * a + CurScrollPageDx, 0, SCREENLEN);

	SDText SText;
	SText.set(CurScrollPageDx, TOP - 20, SCREENLEN, CARHEI, 0, 0, SCREENLEN, CARHEI, 0x7FFF, MASKCOL, g_vm->_sysText[17]);
	SText.DText();

	for (int a = 0; a < TOP; a++)
		VCopy(a * VirtualPageLen + VideoScrollPageDx, Video2 + a * CurRoomMaxX + CurScrollPageDx, SCREENLEN);

	UnlockVideo();

	FreeKey();

	mleft = mright = 0;
	Mouse(3);
	while (mleft || mright)
		Mouse(3);

	waitKey();

	CloseSys(NULL);
}

/* -----------------01/02/98 18.38-------------------
 * 					CheckFileInCD
 * --------------------------------------------------*/
void CheckFileInCD(const char *name) {
	extern char CurCDSet;
	char str[200];
	FILEENTRY fe;

	strcpy(fe.name, name);
	LPFILEENTRY pfe = (LPFILEENTRY)bsearch(&fe, FileRef, NumFileRef, sizeof(FILEENTRY), Compare);
	if (pfe == NULL)
		CloseSys(g_vm->_sysText[5]);

	char optcd = 1;
	if ((g_vm->_curRoom == r11) || (g_vm->_curRoom == r12) || (g_vm->_curRoom == r13) || (g_vm->_curRoom == r14) || (g_vm->_curRoom == r15) || (g_vm->_curRoom == r16) || (g_vm->_curRoom == r17) || (g_vm->_curRoom == r18) || (g_vm->_curRoom == r19) || (g_vm->_curRoom == r1A) || (g_vm->_curRoom == r1B) || (g_vm->_curRoom == r1C) || (g_vm->_curRoom == r1D) || (g_vm->_curRoom == rINTRO) || (g_vm->_curRoom == rSYS) || (g_vm->_curRoom == r12CU) || (g_vm->_curRoom == r13CU))
		optcd = 1;
	else if ((g_vm->_curRoom == r21) || (g_vm->_curRoom == r22) || (g_vm->_curRoom == r23A) || (g_vm->_curRoom == r24) || (g_vm->_curRoom == r25) || (g_vm->_curRoom == r26) || (g_vm->_curRoom == r27) || (g_vm->_curRoom == r28) || (g_vm->_curRoom == r29) || (g_vm->_curRoom == r2A) || (g_vm->_curRoom == r2B) || (g_vm->_curRoom == r2C) || (g_vm->_curRoom == r2D) || (g_vm->_curRoom == r2E) || (g_vm->_curRoom == r2F) || (g_vm->_curRoom == r2G) || (g_vm->_curRoom == r2H) || (g_vm->_curRoom == r31) || (g_vm->_curRoom == r32) || (g_vm->_curRoom == r33) || (g_vm->_curRoom == r34) || (g_vm->_curRoom == r35) || (g_vm->_curRoom == r36) || (g_vm->_curRoom == r37) || (g_vm->_curRoom == r23B) || (g_vm->_curRoom == r29L) || (g_vm->_curRoom == r2BL) || (g_vm->_curRoom == r2GV) || (g_vm->_curRoom == r31P) || (g_vm->_curRoom == r35P) || (g_vm->_curRoom == r36F))
		optcd = 2;
	else if ((g_vm->_curRoom == r41) || (g_vm->_curRoom == r42) || (g_vm->_curRoom == r43) || (g_vm->_curRoom == r44) || (g_vm->_curRoom == r45) || (g_vm->_curRoom == r46) || (g_vm->_curRoom == r47) || (g_vm->_curRoom == r48) || (g_vm->_curRoom == r49) || (g_vm->_curRoom == r4A) || (g_vm->_curRoom == r4B) || (g_vm->_curRoom == r4C) || (g_vm->_curRoom == r4D) || (g_vm->_curRoom == r4E) || (g_vm->_curRoom == r4F) || (g_vm->_curRoom == r4G) || (g_vm->_curRoom == r4H) || (g_vm->_curRoom == r4I) || (g_vm->_curRoom == r4J) || (g_vm->_curRoom == r4K) || (g_vm->_curRoom == r4L) || (g_vm->_curRoom == r4M) || (g_vm->_curRoom == r4N) || (g_vm->_curRoom == r4O) || (g_vm->_curRoom == r4P) || (g_vm->_curRoom == r4Q) || (g_vm->_curRoom == r4R) || (g_vm->_curRoom == r4S) || (g_vm->_curRoom == r4T) || (g_vm->_curRoom == r4U) || (g_vm->_curRoom == r4V) || (g_vm->_curRoom == r4W) || (g_vm->_curRoom == r4X) || (g_vm->_curRoom == r51) || (g_vm->_curRoom == r52) || (g_vm->_curRoom == r53) || (g_vm->_curRoom == r54) || (g_vm->_curRoom == r55) || (g_vm->_curRoom == r56) || (g_vm->_curRoom == r57) || (g_vm->_curRoom == r58) || (g_vm->_curRoom == r59) || (g_vm->_curRoom == r5A) || (g_vm->_curRoom == r41D) || (g_vm->_curRoom == r45S) || (g_vm->_curRoom == r49M) || (g_vm->_curRoom == r4CT) || (g_vm->_curRoom == r58M) || (g_vm->_curRoom == r58T) || (g_vm->_curRoom == r59L))
		optcd = 3;

	char ncd = 1;
	if (pfe->offset & (1 << (CurCDSet - 1)))
		ncd = CurCDSet;
	else if (pfe->offset & (1 << (optcd - 1)))
		ncd = optcd;
	else if (pfe->offset & 1)
		ncd = 1;
	else if (pfe->offset & 2)
		ncd = 2;
	else if (pfe->offset & 4)
		ncd = 3;

	wordset(Video2, 0, MAXX * MAXY);
	VMouseOFF();

	for (int a = 0; a < TOP; a++)
		wordset(Video2 + MAXX * a, 0, MAXX);

	sprintf(str, "CD%c.bm", ncd + '0');
	ff = FastFileOpen(str);
	FastFileRead(ff, Video2, FastFileLen(ff));
	FastFileClose(ff);
	UpdatePixelFormat(Video2, MAXX * TOP);

	sprintf(str, g_vm->_sysText[4], ncd + '0');
	SDText SText;
	SText.set(0, TOP - 20, SCREENLEN, CARHEI, 0, 0, SCREENLEN, CARHEI, PalTo16bit(255, 255, 255), MASKCOL, str);
	SText.DText();

	for (int a = 0; a < TOP; a++)
		VCopy(a * MAXX, Video2 + a * MAXX, MAXX);
	UnlockVideo();


	Common::String filename = Common::String::format("NlAnim.cd%c", ncd + '0');
	Common::File testCD;
	bool found = false;

	do {
		found = testCD.open(filename);
		FreeKey();
		waitKey();
	} while (!found);
	testCD.close();

	for (int a = 0; a < TOP; a++)
		wordset(Video2 + MAXX * a, 0, MAXX);
	for (int a = 0; a < TOP; a++)
		VCopy(a * MAXX, Video2 + a * MAXX, MAXX);
	UnlockVideo();

	CurCDSet = ncd;

	sprintf(str, "NlData.cd0");
	FastFileInit(str);
	sprintf(str, "NlSpeech.cd0");
	SpeechFileInit(str);
	sprintf(str, "NlAnim.cd%c", CurCDSet + '0');
	AnimFileInit(str);

	VMouseON();
}

} // End of namespace Trecision
