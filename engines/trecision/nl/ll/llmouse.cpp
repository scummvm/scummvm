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
#include "trecision/nl/ll/llinc.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/define.h"
#include "trecision/nl/message.h"
#include "trecision/trecision.h"
#include "trecision/logic.h"

#include "common/file.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/str.h"
#include "common/system.h"
#include "graphics/cursorman.h"
#include "trecision/graphics.h"
#include "trecision/video.h"

namespace Trecision {

uint16 BlinkLastDTextChar = MASKCOL;
extern byte NlVer;

/**
 * Derived serializer class with extra synchronization types
 */
class Serializer : public Common::Serializer {
public:
	Serializer(Common::SeekableReadStream *in, Common::WriteStream *out) : Common::Serializer(in, out) {}

	// Trecision saved games contain a single byte for the savegame version,
	// so use this for setting the serializer version instead of syncVersion()
	void setSaveVersion(byte version) { _version = version; }
};

void VMouseOFF() {
	CursorMan.showMouse(false);
}

void VMouseON() {
	if (!g_vm->_mouseONOFF)
		return ;

	CursorMan.showMouse(true);
}

/*-----------------17/02/95 09.53-------------------
 TextLength - Compute string length from character 0 to num
--------------------------------------------------*/
uint16 TextLength(const char *sign, uint16 num) {
	if (sign == nullptr)
		return 0;

	uint16 len = (num == 0) ? strlen(sign) : num;

	uint16 retVal = 0;
	for (uint16 c = 0; c < len; c++)
		retVal += g_vm->Font[(uint8)sign[c] * 3 + 2];

	return retVal;
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
	g_vm->_graphicsMgr->updatePixelFormat(&tmpTCol, 1);
	if (scol != MASKCOL)
		g_vm->_graphicsMgr->updatePixelFormat(&tmpSCol, 1);

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

			uint16 carSco = (uint16)(g_vm->Font[curCar * 3]) + (uint16)(g_vm->Font[curCar * 3 + 1] << 8);
			uint16 DataSco = 768;                       /* Scostamento */
			uint16 CarWid = g_vm->Font[curCar * 3 + 2]; /* Larghezza   */

			if ((c == (len - 1)) && (BlinkLastDTextChar != MASKCOL))
				tmpTCol = BlinkLastDTextChar;

			for (uint16 a = (b * CARHEI); a < ((b + 1)*CARHEI); a++) {
				uint16 CarCounter = 0;
				uint16 CurColor = tmpSCol;

				while (CarCounter <= (CarWid - 1)) {
					if ((a >= l[1]) && (a < l[3])) {
						if ((CurColor != MASKCOL) && (g_vm->Font[carSco + DataSco])) {
							uint16 firstLim = inc + CarCounter;
							uint16 lastLim = firstLim + g_vm->Font[carSco + DataSco];
							uint16 *dst1 = g_vm->_screenBuffer + (x + firstLim) + (y + a) * SCREENLEN;
							uint16 *dst2 = g_vm->_screenBuffer + (x + l[0]) + (y + a) * SCREENLEN;
							uint16 *dst = nullptr;
							uint16 size = 0;
							
							if (firstLim >= l[0] && lastLim < l[2]) {
								dst = dst1;
								size = lastLim - firstLim;
							} else if (firstLim < l[0] && lastLim < l[2] && lastLim > l[0]) {
								dst = dst2;
								size = lastLim - l[0];
							} else if (firstLim >= l[0] && lastLim >= l[2] && l[2] > firstLim) {
								dst = dst1;
								size = l[2] - firstLim;
							} else if (firstLim < l[0] && lastLim >= l[2] && l[2] > firstLim) {
								dst = dst2;
								size = l[2] - l[0];
							}

							if (dst && size > 0) {
								uint16 *d = (uint16 *)dst;
								for (uint32 i = 0; i < size; i++)
									*d++ = CurColor;
							}
						}
					}

					CarCounter += g_vm->Font[carSco + DataSco];
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
			g_vm->Icone[(READICON + 13) * ICONDX * ICONDY + b * ICONDX + a] = g_vm->_graphicsMgr->restorePixelFormat(g_vm->_screenBuffer[SCREENLEN * b * 10 + a * (SCREENLEN / ICONDX)]);
		g_vm->Icone[(READICON + 13)*ICONDX * ICONDY + b * ICONDX + a] = blackPixel;
	}
}

bool syncSaveData(int slot, char *desc, bool save) {
	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	Common::String saveName = g_vm->getSavegameName(slot);
	Common::InSaveFile *in = !save ? saveFileMan->openForLoading(saveName) : nullptr;
	Common::OutSaveFile *out = save ? saveFileMan->openForSaving(saveName) : nullptr;
	if (!in && !out)
		return false;

	byte version = !save ? in->readByte() : NlVer;
	// TODO: Check for newer save versions
	Common::Serializer ser(in, out);
	ser.setVersion(version);
	if (save)
		out->writeByte(version);

	ser.syncBytes((byte *)desc, 40);
	
	uint16 *thumbnailBuf = g_vm->Icone + (READICON + 13) * ICONDX * ICONDY;
	for (int i = 0; i < ICONDX * ICONDY; i++)
		ser.syncAsUint16LE(thumbnailBuf[i]);
	if (!save)
		g_vm->_graphicsMgr->updatePixelFormat(thumbnailBuf, ICONDX * ICONDY);

	ser.syncAsUint16LE(g_vm->_curRoom);
	ser.syncAsByte(/*OldInvLen*/ g_vm->_inventorySize);
	ser.syncAsByte(g_vm->_cyberInventorySize);
	ser.syncAsByte(/*OldIconBase*/ g_vm->_iconBase);
	ser.syncAsSint16LE(Flagskiptalk);
	ser.syncAsSint16LE(Flagskipenable);
	ser.syncAsSint16LE(g_vm->_flagMouseEnabled);
	ser.syncAsSint16LE(g_vm->_flagScreenRefreshed);
	ser.syncAsSint16LE(FlagPaintCharacter);
	ser.syncAsSint16LE(FlagSomeOneSpeak);
	ser.syncAsSint16LE(FlagCharacterSpeak);
	ser.syncAsSint16LE(g_vm->_flagInventoryLocked);
	ser.syncAsSint16LE(FlagUseWithStarted);
	ser.syncAsSint16LE(FlagMousePolling);
	ser.syncAsSint16LE(FlagDialogSolitaire);
	ser.syncAsSint16LE(FlagCharacterExist);
	ser.syncBytes(/*OldInv*/ g_vm->_inventory, MAXICON);
	ser.syncBytes(g_vm->_cyberInventory, MAXICON);
	ser.syncAsFloatLE(_actor._px);
	ser.syncAsFloatLE(_actor._py);
	ser.syncAsFloatLE(_actor._pz);
	ser.syncAsFloatLE(_actor._dx);
	ser.syncAsFloatLE(_actor._dz);
	ser.syncAsFloatLE(_actor._theta);
	ser.syncAsSint32LE(_curPanel);
	ser.syncAsSint32LE(_oldPanel);

	for (int a = 0; a < MAXROOMS; a++) {
		ser.syncBytes((byte *)g_vm->_room[a]._baseName, 4);
		for (int i = 0; i < MAXACTIONINROOM; i++)
			ser.syncAsUint16LE(g_vm->_room[a]._actions[i]);		
		ser.syncAsByte(g_vm->_room[a]._flag);
		ser.syncAsUint16LE(g_vm->_room[a]._bkgAnim);
	}

	for (int a = 0; a < MAXOBJ; a++) {
		for (int i = 0; i < 4; i++)
			ser.syncAsUint16LE(g_vm->_obj[a]._lim[i]);
		ser.syncAsUint16LE(g_vm->_obj[a]._name);
		ser.syncAsUint16LE(g_vm->_obj[a]._examine);
		ser.syncAsUint16LE(g_vm->_obj[a]._action);
		ser.syncAsUint16LE(g_vm->_obj[a]._anim);
		ser.syncAsByte(g_vm->_obj[a]._mode);
		ser.syncAsByte(g_vm->_obj[a]._flag);
		ser.syncAsByte(g_vm->_obj[a]._goRoom);
		ser.syncAsByte(g_vm->_obj[a]._nbox);
		ser.syncAsByte(g_vm->_obj[a]._ninv);
		ser.syncAsSByte(g_vm->_obj[a]._position);
	}

	for (int a = 0; a < MAXINVENTORY; a++) {
		ser.syncAsUint16LE(g_vm->_inventoryObj[a]._name);
		ser.syncAsUint16LE(g_vm->_inventoryObj[a]._examine);
		ser.syncAsUint16LE(g_vm->_inventoryObj[a]._action);
		ser.syncAsUint16LE(g_vm->_inventoryObj[a]._anim);
		ser.syncAsByte(g_vm->_inventoryObj[a]._flag);
	}

	for (int a = 0; a < MAXANIM; a++) {
		SAnim *cur = &g_vm->_animMgr->_animTab[a];
		ser.syncBytes((byte *)cur->_name, 14);
		ser.syncAsUint16LE(cur->_flag);
		for (int i = 0; i < MAXCHILD; i++) {
			for (int j = 0; j < 4; j++) {
				ser.syncAsUint16LE(cur->_lim[i][j]);
			}
		}
		ser.syncAsByte(cur->_nbox);
		for (int i = 0; i < MAXATFRAME; i++) {
			ser.syncAsByte(cur->_atFrame[i]._type);
			ser.syncAsByte(cur->_atFrame[i]._child);
			ser.syncAsUint16LE(cur->_atFrame[i]._numFrame);
			ser.syncAsUint16LE(cur->_atFrame[i]._index);
		}
	}
	
	for (int a = 0; a < MAXSAMPLE; a++) {
		ser.syncAsByte(GSample[a]._volume);
		ser.syncAsByte(GSample[a]._flag);
	}

	for (int a = 0; a < MAXCHOICE; a++) {
		DialogChoice *cur = &g_vm->_choice[a];
		ser.syncAsUint16LE(cur->_flag);
		ser.syncAsUint16LE(cur->_sentenceIndex);
		ser.syncAsUint16LE(cur->_firstSubTitle);
		ser.syncAsUint16LE(cur->_subTitleNumb);
		for (int i = 0; i < MAXDISPSCELTE; i++)
			ser.syncAsUint16LE(cur->_on[i]);
		for (int i = 0; i < MAXDISPSCELTE; i++)
			ser.syncAsUint16LE(cur->_off[i]);
		ser.syncAsUint16LE(cur->_startFrame);
		ser.syncAsUint16LE(cur->_nextDialog);
	}

	for (int a = 0; a < MAXDIALOG; a++) {
		Dialog *cur = &_dialog[a];
		ser.syncAsUint16LE(cur->_flag);
		ser.syncAsUint16LE(cur->_interlocutor);
		ser.syncBytes((byte *)cur->_startAnim, 14);
		ser.syncAsUint16LE(cur->_startLen);
		ser.syncAsUint16LE(cur->_firstChoice);
		ser.syncAsUint16LE(cur->_choiceNumb);
		for (int i = 0; i < MAXNEWSMKPAL; i++)
			ser.syncAsUint16LE(cur->_newPal[i]);
	}

	for (int i = 0; i < 7; i++)
		ser.syncAsUint16LE(g_vm->_logicMgr->Comb35[i]);
	for (int i = 0; i < 4; i++)
		ser.syncAsUint16LE(g_vm->_logicMgr->Comb49[i]);
	for (int i = 0; i < 6; i++)
		ser.syncAsUint16LE(g_vm->_logicMgr->Comb4CT[i]);
	for (int i = 0; i < 6; i++)
		ser.syncAsUint16LE(g_vm->_logicMgr->Comb58[i]);
	for (int i = 0; i < 3; i++)
		ser.syncAsUint16LE(g_vm->_wheelPos[i]);
	ser.syncAsUint16LE(g_vm->_wheel);
	ser.syncAsUint16LE(g_vm->_logicMgr->Count35);
	ser.syncAsUint16LE(g_vm->_logicMgr->Count58);
	ser.syncAsUint16LE(g_vm->_slotMachine41Counter);

	delete in;
	delete out;
	
	return true;
}

/* -----------------25/10/97 15.16-------------------
						DataSave
 --------------------------------------------------*/
bool DataSave() {
	uint8 OldInv[MAXICON], OldIconBase, OldInvLen;
	char tempname[20], ch, strcount;
	char savename[MAXSAVEFILE][40];
	uint16 posx, LenText;
	bool ret = true;
	
	actorStop();
	nextStep();

	for (int a = 0; a < TOP; a++)
		memset(g_vm->_screenBuffer + SCREENLEN * a, 0, SCREENLEN * 2);

	SDText SText;
	SText.set(0, TOP - 20, SCREENLEN, CARHEI, 0, 0, SCREENLEN, CARHEI, 0x7FFF, MASKCOL, g_vm->_sysText[9]);
	SText.DText();

	g_vm->_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	for (int a = TOP + AREA; a < AREA + 2 * TOP; a++)
		memset(g_vm->_screenBuffer + SCREENLEN * a, 0, SCREENLEN * 2);
	g_vm->_graphicsMgr->copyToScreen(0, TOP + AREA, MAXX, TOP);

	g_vm->_gameQueue.initQueue();
	g_vm->_animQueue.initQueue();
	g_vm->_characterQueue.initQueue();

	FreeKey();

	mleft = mright = false;
	Mouse(MCMD_UPDT);
	while (mleft || mright)
		Mouse(MCMD_UPDT);

	// ferma omino, animazioni, spegne scritte
	memcpy(OldInv, g_vm->_inventory, MAXICON);
	memset(g_vm->_inventory, 0, MAXICON);

	OldIconBase = g_vm->_iconBase;
	g_vm->_iconBase = 0;
	OldInvLen = g_vm->_inventorySize;
	g_vm->_inventorySize = MAXSAVEFILE;

insave:

	int8 CurPos = -1;
	int8 OldPos = -1;
	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	
	for (int a = 0; a < g_vm->_inventorySize; a++) {
		Common::String saveName = g_vm->getSavegameName(a);
		Common::InSaveFile *saveFile = saveFileMan->openForLoading(saveName);
		
		if (saveFile && saveFile->readByte() == NlVer) {
			saveFile->read(&savename[a], 40);
			uint16 *thumbnailBuf = g_vm->Icone + (READICON + 1 + a) * ICONDX * ICONDY;
			for (int i = 0; i < ICONDX * ICONDY; i++)
				thumbnailBuf[i] = saveFile->readUint16LE();
			
			g_vm->_graphicsMgr->updatePixelFormat(thumbnailBuf, ICONDX * ICONDY);

			g_vm->_inventory[a] = LASTICON + a;
		} else {
			strcpy(savename[a], g_vm->_sysText[10]);
			g_vm->_inventory[a] = iEMPTYSLOT;
		}

		delete saveFile;
	}

	g_vm->refreshInventory(0, 0);

	bool skipSave = false;
	ch = 0;
	for (;;) {
		g_vm->checkSystem();
		Mouse(MCMD_UPDT);

		GetKey();

		if ((my >= FIRSTLINE) &&
				(my < (FIRSTLINE + ICONDY)) &&
				(mx >= ICONMARGSX) &&
				(mx < (SCREENLEN - ICONMARGDX))) {
			OldPos = CurPos;
			CurPos = ((mx - ICONMARGSX) / ICONDX);

			if (OldPos != CurPos) {
				for (int a = FIRSTLINE + ICONDY + 10; a < FIRSTLINE + ICONDY + 10 + CARHEI; a++)
					memset(g_vm->_screenBuffer + SCREENLEN * a, 0, SCREENLEN * 2);

				posx    = ICONMARGSX + ((CurPos) * (ICONDX)) + ICONDX / 2;
				LenText  = TextLength(savename[CurPos], 0);

				posx = CLIP(posx - (LenText / 2), 2, SCREENLEN - 2 - LenText);
				SText.set(posx, FIRSTLINE + ICONDY + 10, LenText, CARHEI, 0, 0, LenText, CARHEI, 0x7FFF, MASKCOL, savename[CurPos]);
				SText.DText();

				g_vm->_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
			}

			if (mleft)
				break;
		} else {
			if (OldPos != -1) {
				for (int a = FIRSTLINE + ICONDY + 10; a < FIRSTLINE + ICONDY + 10 + CARHEI; a++)
					memset(g_vm->_screenBuffer + SCREENLEN * a, 0, SCREENLEN * 2);

				g_vm->_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
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
				memset(g_vm->_screenBuffer + SCREENLEN * a, 0, SCREENLEN * 2);

			g_vm->_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
		}

		for (; ;) {
			g_vm->_keybInput = true;
			g_vm->checkSystem();
			ch = GetKey();
			FreeKey();

			Mouse(MCMD_UPDT);
			g_vm->_keybInput = false;

			if (ch == 0x1B) {
				ch = 0;
				for (int a = FIRSTLINE + ICONDY + 10; a < FIRSTLINE + ICONDY + 10 + CARHEI; a++)
					memset(g_vm->_screenBuffer + SCREENLEN * a, 0, SCREENLEN * 2);

				g_vm->_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);

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
				memset(g_vm->_screenBuffer + SCREENLEN * a, 0, SCREENLEN * 2);

			int endStr = strlen(savename[CurPos]);
			savename[CurPos][endStr] = '_';
			savename[CurPos][endStr + 1] = '\0';

			posx    = ICONMARGSX + ((CurPos) * (ICONDX)) + ICONDX / 2 ;
			LenText  = TextLength(savename[CurPos], 0);

			posx = CLIP(posx - (LenText / 2), 2, SCREENLEN - 2 - LenText);
			SText.set(posx, FIRSTLINE + ICONDY + 10, LenText, CARHEI, 0, 0, LenText, CARHEI, 0x7FFF, MASKCOL, savename[CurPos]);

			if ((ReadTime() / 8) & 1)
				BlinkLastDTextChar = 0x0000;

			SText.DText();
			BlinkLastDTextChar = MASKCOL;

			endStr = strlen(savename[CurPos]);
			savename[CurPos][endStr - 1] = '\0';

			g_vm->_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
		}

		for (int a = FIRSTLINE; a < MAXY; a++)
			memset(g_vm->_screenBuffer + SCREENLEN * a, 0, SCREENLEN * 2);

		ret = false;

		syncSaveData(CurPos, savename[CurPos], true);
	}

	for (int a = FIRSTLINE; a < MAXY; a++)
		memset(g_vm->_screenBuffer + SCREENLEN * a, 0, SCREENLEN * 2);

	g_vm->_graphicsMgr->copyToScreen(0, FIRSTLINE, MAXX, TOP);

	for (int a = TOP - 20; a < TOP - 20 + CARHEI; a++)
		memset(g_vm->_screenBuffer + SCREENLEN * a, 0, SCREENLEN * 2);

	g_vm->_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);
	g_vm->_curInventory = 0;

	memcpy(g_vm->_inventory, OldInv, MAXICON);

	g_vm->_iconBase = OldIconBase;
	g_vm->_inventorySize = OldInvLen;

	mleft = mright = false;
	Mouse(MCMD_UPDT);
	while (mleft || mright)
		Mouse(MCMD_UPDT);

	return ret;
}

/*-----------------09/02/96 20.57-------------------
					DataLoad
--------------------------------------------------*/
bool DataLoad() {
	extern char CurCDSet;
	uint8 OldInv[MAXICON], OldIconBase, OldInvLen;
	char savename[MAXSAVEFILE][40];
	bool retval = true;
	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	
	for (int a = 0; a < TOP; a++)
		memset(g_vm->_screenBuffer + SCREENLEN * a, 0, SCREENLEN * 2);

	if (!g_vm->_flagMouseEnabled) {
		g_vm->_flagMouseEnabled = true;
		Mouse(MCMD_ON);
	}

	SDText SText;
	SText.set(0, TOP - 20, SCREENLEN, CARHEI, 0, 0, SCREENLEN, CARHEI, 0x7FFF, MASKCOL, g_vm->_sysText[11]);
	SText.DText();

	g_vm->_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	for (int a = TOP + AREA; a < AREA + 2 * TOP; a++)
		memset(g_vm->_screenBuffer + SCREENLEN * a, 0, SCREENLEN * 2);
	g_vm->_graphicsMgr->copyToScreen(0, TOP + AREA, MAXX, TOP);

	g_vm->_gameQueue.initQueue();
	g_vm->_animQueue.initQueue();
	g_vm->_characterQueue.initQueue();

	FreeKey();

	mleft = mright = false;
	Mouse(MCMD_UPDT);
	while (mleft || mright)
		Mouse(MCMD_UPDT);

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
		Common::String saveName = g_vm->getSavegameName(a);
		Common::InSaveFile *saveFile = saveFileMan->openForLoading(saveName);

		if (saveFile && saveFile->readByte() == NlVer) {
			saveFile->read(&savename[a], 40);
			uint16 *iconBuf = g_vm->Icone + (READICON + 1 + a) * ICONDX * ICONDY;
			for (int i = 0; i < ICONDX * ICONDY; i++)
				iconBuf[i] = saveFile->readUint16LE();
			g_vm->_graphicsMgr->updatePixelFormat(iconBuf, ICONDX * ICONDY);

			g_vm->_inventory[a] = LASTICON + a;
		} else {
			strcpy(savename[a], g_vm->_sysText[10]);
			g_vm->_inventory[a] = iEMPTYSLOT;
		}

		delete saveFile;
	}

	g_vm->refreshInventory(0, 0);

	bool skipLoad = false;

	for (;;) {
		g_vm->checkSystem();
		Mouse(MCMD_UPDT);

		GetKey();

		if ((my >= FIRSTLINE) &&
				(my < (FIRSTLINE + ICONDY)) &&
				(mx >= ICONMARGSX) &&
				(mx < (SCREENLEN - ICONMARGDX))) {
			OldPos = CurPos;
			CurPos = ((mx - ICONMARGSX) / ICONDX);

			if (OldPos != CurPos) {
				for (int a = FIRSTLINE + ICONDY + 10; a < FIRSTLINE + ICONDY + 10 + CARHEI; a++)
					memset(g_vm->_screenBuffer + SCREENLEN * a, 0, SCREENLEN * 2);

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

				g_vm->_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
			}

			if (mleft && (g_vm->_inventory[CurPos] != iEMPTYSLOT))
				break;
		} else {
			if (OldPos != -1) {
				for (int a = FIRSTLINE + ICONDY + 10; a < FIRSTLINE + ICONDY + 10 + CARHEI; a++)
					memset(g_vm->_screenBuffer + SCREENLEN * a, 0, SCREENLEN * 2);

				g_vm->_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
			}

			OldPos = -1;
			CurPos = -1;

			if (mleft || mright) {
				retval = false;
				skipLoad = true;
				break;
			}
		}
	}

	if (!skipLoad) {
		for (int a = FIRSTLINE; a < MAXY; a++)
			memset(g_vm->_screenBuffer + SCREENLEN * a, 0, SCREENLEN * 2);

		syncSaveData(CurPos, savename[CurPos], false);
		
		FlagNoPaintScreen = true;
		g_vm->_curStack = 0;
		g_vm->_flagscriptactive = false;

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
	g_vm->checkSystem();

	for (int a = FIRSTLINE; a < MAXY; a++)
		memset(g_vm->_screenBuffer + SCREENLEN * a, 0, SCREENLEN * 2);

	g_vm->_graphicsMgr->copyToScreen(0, FIRSTLINE, MAXX, TOP);

	for (int a = TOP - 20; a < TOP - 20 + CARHEI; a++)
		memset(g_vm->_screenBuffer + SCREENLEN * a, 0, SCREENLEN * 2);

	g_vm->_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	memcpy(g_vm->_inventory, OldInv, MAXICON);

	g_vm->_curInventory = 0;
	g_vm->_iconBase = OldIconBase;
	g_vm->_inventorySize = OldInvLen;

	mleft = mright = false;
	Mouse(MCMD_UPDT);
	while (mleft || mright)
		Mouse(MCMD_UPDT);

	if (g_vm->_flagscriptactive) {
		g_vm->_flagMouseEnabled = false;
		Mouse(MCMD_OFF);
	}

	return retval;
}

/*-----------------09/02/96 20.57-------------------
					QuitGame
--------------------------------------------------*/
bool QuitGame() {
	for (int a = 0; a < TOP; a++)
		memcpy(g_vm->ZBuffer + a * SCREENLEN, g_vm->_screenBuffer + SCREENLEN * a, SCREENLEN * 2);

	for (int a = 0; a < TOP; a++)
		memset(g_vm->_screenBuffer + SCREENLEN * a, 0, SCREENLEN * 2);

	SDText SText;
	SText.set(0, TOP - 20, SCREENLEN, CARHEI, 0, 0, SCREENLEN, CARHEI, 0x7FFF, MASKCOL, g_vm->_sysText[13]);
	SText.DText();

	g_vm->_graphicsMgr->copyToScreen(0, 0, SCREENLEN, TOP);

	FreeKey();

	mleft = mright = false;
	Mouse(MCMD_UPDT);
	while (mleft || mright)
		Mouse(MCMD_UPDT);

	char ch = waitKey();

	bool exitFl = ((ch == 'y') || (ch == 'Y'));

	for (int a = 0; a < TOP; a++)
		memcpy(g_vm->_screenBuffer + SCREENLEN * a, g_vm->ZBuffer + a * SCREENLEN, SCREENLEN * 2);

	g_vm->_graphicsMgr->copyToScreen(0, 0, SCREENLEN, TOP);

	return exitFl;
}

/*-----------------09/02/96 20.57-------------------
					DemoOver
--------------------------------------------------*/
void DemoOver() {
	for (int a = 0; a < TOP; a++)
		memset(g_vm->_screenBuffer + SCREENLEN * a, 0, SCREENLEN * 2);

	SDText SText;
	SText.set(0, TOP - 20, SCREENLEN, CARHEI, 0, 0, SCREENLEN, CARHEI, 0x7FFF, MASKCOL, g_vm->_sysText[17]);
	SText.DText();

	g_vm->_graphicsMgr->copyToScreen(0, 0, SCREENLEN, TOP);

	FreeKey();

	mleft = mright = false;
	Mouse(MCMD_UPDT);
	while (mleft || mright)
		Mouse(MCMD_UPDT);

	waitKey();

	CloseSys(nullptr);
}

/* -----------------01/02/98 18.38-------------------
 * 					CheckFileInCD
 * --------------------------------------------------*/
void CheckFileInCD(Common::String name) {
	extern char CurCDSet;
	char str[200];
	SFileEntry fe;

	strncpy(fe.name, name.c_str(), 11);
	fe.name[11] = '\0';	
	SFileEntry *pfe = (SFileEntry *)bsearch(&fe, FileRef, NumFileRef, sizeof(SFileEntry), Compare);
	if (pfe == nullptr)
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

	memset(g_vm->_screenBuffer, 0, MAXX * MAXY * 2);
	VMouseOFF();

	for (int a = 0; a < TOP; a++)
		memset(g_vm->_screenBuffer + MAXX * a, 0, MAXX * 2);

	sprintf(str, "CD%c.bm", ncd + '0');
	ff = FastFileOpen(str);
	FastFileRead(ff, g_vm->_screenBuffer, FastFileLen(ff));
	FastFileClose(ff);
	g_vm->_graphicsMgr->updatePixelFormat(g_vm->_screenBuffer, MAXX * TOP);

	sprintf(str, g_vm->_sysText[4], ncd + '0');
	SDText SText;
	SText.set(0, TOP - 20, SCREENLEN, CARHEI, 0, 0, SCREENLEN, CARHEI, g_vm->_graphicsMgr->palTo16bit(255, 255, 255), MASKCOL, str);
	SText.DText();

	g_vm->_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

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
		memset(g_vm->_screenBuffer + MAXX * a, 0, MAXX * 2);

	g_vm->_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	CurCDSet = ncd;

	FastFileInit("NlData.cd0");
	SpeechFileInit("NlSpeech.cd0");
	Common::String fname = Common::String::format("NlAnim.cd%c", CurCDSet + '0');
	AnimFileInit(fname);

	VMouseON();
}

} // End of namespace Trecision
