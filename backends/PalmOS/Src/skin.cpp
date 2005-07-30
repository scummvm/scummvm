/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include <PalmOS.h>
#include <PalmOSGlue.h>
#include <SonyClie.h>

#include "start.h"
#include "games.h"
#include "globals.h"
#include "skin.h"
#include "StarterRsc.h"

UInt16 lastIndex = dmMaxRecordIndex;	// last select index in the list to prevent flash
static WinHandle winLockH = NULL;

MemPtr SknScreenLock(WinLockInitType initMode) {
	if (!OPTIONS_TST(kOptDeviceZodiac)) {
		WinSetDrawWindow(WinGetDisplayWindow());
		// WARNING : this doesn't work on < OS5 with 16bit mode
		return WinScreenLock(initMode);
	}

	Err e;
	RectangleType r;

	WinGetBounds(WinGetDisplayWindow(), &r);
	winLockH = WinCreateOffscreenWindow(r.extent.x, r.extent.y, nativeFormat, &e);
	WinSetDrawWindow(winLockH);

	if (initMode == winLockCopy)
		WinCopyRectangle(WinGetDisplayWindow(), winLockH, &r, 0, 0, winPaint);

	return BmpGetBits(WinGetBitmap(winLockH));
}

void SknScreenUnlock() {
	if (!OPTIONS_TST(kOptDeviceZodiac)) {
		WinSetDrawWindow(WinGetDisplayWindow());
		WinScreenUnlock();
		return;
	}

	RectangleType r;

	WinGetBounds(winLockH, &r);
	WinCopyRectangle(winLockH, WinGetDisplayWindow(), &r, 0, 0, winPaint);
	WinSetDrawWindow(WinGetDisplayWindow());
	WinDeleteWindow(winLockH, false);
	winLockH = NULL;
}

static void SknGetListColors(DmOpenRef skinDBP, UInt8 *text, UInt8 *selected, UInt8 *background) {
	UInt16 colIndex;
	MemHandle colH;
	UInt8 *colTemp;

	// default
	*text = UIColorGetTableEntryIndex (UIMenuForeground);
	*selected = UIColorGetTableEntryIndex (UIMenuSelectedForeground);
	*background = UIColorGetTableEntryIndex (UIMenuSelectedFill);

	if (skinDBP) {
		colIndex = DmFindResource (skinDBP, sknColorsRsc, skinColors, NULL);

		if (colIndex != (UInt16)-1) {
			colH = DmGetResourceIndex(skinDBP, colIndex);

			if (colH) {
				colTemp = (UInt8 *)MemHandleLock(colH);

				*text = colTemp[0];
				*selected = colTemp[1];
				*background = colTemp[2];

				MemPtrUnlock(colTemp);
				DmReleaseResource(colH);
			}
		}
	}
}

static void SknCopyBits(DmOpenRef skinDBP, DmResID bitmapID, const RectangleType *srcRect, Coord destX, Coord destY) {
	MemHandle hTemp;
	BitmapPtr bmpTemp;
	UInt16 index;

	Coord cx, cy, cw, ch, bw, bh;
	RectangleType copy, old;

	if (skinDBP) {
		// find the bitmap
		index = DmFindResource (skinDBP, bitmapRsc, bitmapID, NULL);

		if (index != (UInt16)-1) {
			hTemp = DmGetResourceIndex(skinDBP,index);

			if (hTemp) {
				bmpTemp = (BitmapType *)MemHandleLock(hTemp);
				BmpGlueGetDimensions(bmpTemp, &bw, &bh, 0);

				if (!srcRect) {
					cx = 0;
					cy = 0;
					cw = bw;
					ch = bh;
				} else {
					cx = srcRect->topLeft.x;
					cy = srcRect->topLeft.y;
					cw = srcRect->extent.x;
					ch = srcRect->extent.y;
				}

				if (ch) {
					WinGetClip(&old);
					if (gVars->HRrefNum != sysInvalidRefNum) {
						copy.topLeft.x = destX;
						copy.topLeft.y = destY;
						copy.extent.x = cw;
						copy.extent.y = ch;

						HRWinSetClip(gVars->HRrefNum, &copy);
						HRWinDrawBitmap(gVars->HRrefNum, bmpTemp, destX - cx, destY - cy);
					} else {
						Err e;
						BitmapTypeV3 *bmp2P;

						// create an uncompressed version of the bitmap
						WinHandle win = WinCreateOffscreenWindow(bw, bh, screenFormat, &e);
						WinHandle old = WinGetDrawWindow();
						WinSetDrawWindow(win);
						WinDrawBitmap(bmpTemp, 0, 0);
						WinSetDrawWindow(old);

 						bmp2P = BmpCreateBitmapV3(WinGetBitmap(win), kDensityDouble, BmpGetBits(WinGetBitmap(win)), NULL);

						copy.topLeft.x = destX / 2;
						copy.topLeft.y = destY / 2;
						copy.extent.x = cw / 2;
						copy.extent.y = ch / 2;

						WinSetClip(&copy);
						WinDrawBitmap((BitmapPtr)bmp2P, (destX - cx) / 2, (destY - cy) / 2);
						BmpDelete((BitmapPtr)bmp2P);
						WinDeleteWindow(win, false);
					}
					WinSetClip(&old);
				}

				MemPtrUnlock(bmpTemp);
				DmReleaseResource(hTemp);
			}
		}
	}
}

void SknApplySkin() {
	DmOpenRef skinDBP;
	RectangleType r;
	FormPtr frmP = FrmGetActiveForm();

	// draw skin
	FrmDrawForm(frmP);
	SknScreenLock(winLockCopy);

	skinDBP = SknOpenSkin();

	if (gPrefs->card.volRefNum != sysInvalidRefNum)
		FrmShowObject(frmP, FrmGetObjectIndex (frmP, MainMSBitMap));
	else
		FrmShowObject(frmP, FrmGetObjectIndex (frmP, MainMSNoneBitMap));

	WinSetForeColor(255);
	WinSetDrawMode(winPaint);
	WinDrawLine (0, 14, 159, 14);
	WinDrawLine (0, 13, 159, 13);

	SknGetObjectBounds(skinDBP, skinBackgroundImageTop, &r);
	SknCopyBits(skinDBP, skinBackgroundImageTop, 0, r.topLeft.x, r.topLeft.y);
	SknGetObjectBounds(skinDBP, skinBackgroundImageBottom, &r);
	SknCopyBits(skinDBP, skinBackgroundImageBottom, 0, r.topLeft.x, r.topLeft.y);

	for (UInt16 resID = 1100; resID <= 7000; resID += 100) {
		SknSetState(skinDBP, resID, sknStateNormal);
		SknShowObject(skinDBP, resID);
	}

	SknCloseSkin(skinDBP);
	SknScreenUnlock();
	SknUpdateList();
}

void SknGetObjectBounds(DmOpenRef skinDBP, DmResID resID, RectangleType *rP) {

	UInt16 bmpIndex, strIndex;
	MemHandle hBmp, hStr;
	BitmapType *bmpTemp;
	UInt8 *strTemp;

	RctSetRectangle(rP, 0, 0, 0, 0);

	if (skinDBP) {
		bmpIndex = DmFindResource (skinDBP, bitmapRsc, resID, NULL);

		if (bmpIndex != (UInt16)-1) {						// if bmp exists
			strIndex = DmFindResource (skinDBP, sknPosRsc, resID, NULL);

			if (strIndex != (UInt16)-1) {					// if params exist
				hBmp = DmGetResourceIndex(skinDBP,bmpIndex);

				if (hBmp) {
					hStr = DmGetResourceIndex(skinDBP,strIndex);

					if (hStr) {
					//	buttons : state|x|y|w/h slider|draw mode|x1/y1 keep|x2/y2 keep slider
					//	list (160mode) : state|x|y|w|h|
						bmpTemp = (BitmapType *)MemHandleLock(hBmp);
						strTemp = (UInt8 *)MemHandleLock(hStr);

						BmpGlueGetDimensions(bmpTemp, &(rP->extent.x), &(rP->extent.y), 0);
						rP->topLeft.x = strTemp[sknInfoPosX] * 2;
						rP->topLeft.y = strTemp[sknInfoPosY] * 2;

						MemPtrUnlock(strTemp);
						DmReleaseResource(hStr);
					}

					MemPtrUnlock(bmpTemp);
					DmReleaseResource(hBmp);
				}
			}
		}
	}
}

DmOpenRef SknOpenSkin() {
	return DmOpenDatabase(gPrefs->skin.cardNo, gPrefs->skin.dbID, dmModeReadOnly);
}

void SknCloseSkin(DmOpenRef skinDBP) {
	if (skinDBP)
		DmCloseDatabase(skinDBP);
}

UInt8 SknSetState(DmOpenRef skinDBP, DmResID resID, UInt8 newState) {

	UInt16 index;
	MemHandle hStr;
	UInt8 *strTemp;
	UInt8 oldState = 0;

	if (skinDBP) {
		index = DmFindResource (skinDBP, sknPosRsc, resID, NULL);

		if (index != (UInt16)-1) {
			hStr = DmGetResourceIndex(skinDBP, index);

			if (hStr) {
				strTemp = (UInt8 *)MemHandleLock(hStr);
				oldState = strTemp[sknInfoState];

				if (oldState != newState) {
					DmWrite(strTemp, 0, &newState, 1);
				}

				MemPtrUnlock(strTemp);
				DmReleaseResource(hStr);
			}
		}
	}

	return oldState;
}

UInt8 SknGetDepth(DmOpenRef skinDBP) {
	UInt16 index;
	MemHandle hStr;
	UInt8 *strTemp;
	UInt8 depth = 8;

	if (skinDBP) {
		index = DmFindResource (skinDBP, sknDepthRsc, skinDepth, NULL);

		if (index != (UInt16)-1) {
			hStr = DmGetResourceIndex(skinDBP, index);

			if (hStr) {
				strTemp = (UInt8 *)MemHandleLock(hStr);
				depth = *strTemp;
				MemPtrUnlock(strTemp);
				DmReleaseResource(hStr);
			}
		}
	}

	return depth;
}

UInt8 SknGetState(DmOpenRef skinDBP, DmResID resID) {

	UInt16 index;
	MemHandle hStr;
	UInt8 *strTemp;
	UInt8 oldState = sknStateDisabled;

	if (skinDBP) {
		index = DmFindResource (skinDBP, sknPosRsc, resID, NULL);

		if (index != (UInt16)-1) {
			hStr = DmGetResourceIndex(skinDBP, index);

			if (hStr) {
				strTemp = (UInt8 *)MemHandleLock(hStr);
				oldState = strTemp[sknInfoState];
				MemPtrUnlock(strTemp);
				DmReleaseResource(hStr);
			}
		}
	}

	return oldState;
}

void SknShowObject(DmOpenRef skinDBP, DmResID resID) {

	RectangleType r;
	UInt8 state = SknGetState(skinDBP, resID);
	SknGetObjectBounds(skinDBP, resID, &r);
	SknCopyBits(skinDBP, resID + state, NULL, r.topLeft.x, r.topLeft.y);
}

void SknGetListBounds(RectangleType *rAreaP, RectangleType *rArea2xP) {
	DmOpenRef skinDBP;
	UInt16 strIndex;
	MemHandle hStr;
	UInt8 *strTemp;
	UInt16 x,y,w,h;

	skinDBP = DmOpenDatabase(gPrefs->skin.cardNo, gPrefs->skin.dbID, dmModeReadOnly);
	if (skinDBP) {
		strIndex = DmFindResource (skinDBP, sknPosRsc, skinList, NULL);

		if (strIndex != 0xFFFF) {					// if params exist
			hStr = DmGetResourceIndex(skinDBP,strIndex);
			if (hStr) {
				strTemp = (UInt8 *)MemHandleLock(hStr);

				x = strTemp[sknInfoPosX];
				y = strTemp[sknInfoPosY];
				w = strTemp[sknInfoListWidth];
				h = strTemp[sknInfoListSize] * sknInfoListItemSize;

				if (rAreaP)
					RctSetRectangle(rAreaP ,x, y, w, h);
				if (rArea2xP)
					RctSetRectangle(rArea2xP, x+x, y+y, w+w, h+h);

				MemHandleUnlock(hStr);
				DmReleaseResource(hStr);

			}
		}

		DmCloseDatabase(skinDBP);
	}
}

static void SknRedrawTools(DmOpenRef skinDBP) {
	if (GamGetSelected() == dmMaxRecordIndex) {
		if (SknGetState(skinDBP, skinButtonGameDelete) == sknStateNormal) {
			SknSetState(skinDBP, skinButtonGameDelete,sknStateDisabled);
			SknShowObject(skinDBP, skinButtonGameDelete);
		}
		if (SknGetState(skinDBP, skinButtonGameEdit) == sknStateNormal) {
			SknSetState(skinDBP, skinButtonGameEdit,sknStateDisabled);
			SknShowObject(skinDBP, skinButtonGameEdit);
		}
		if (SknGetState(skinDBP, skinButtonGameAudio) == sknStateNormal) {
			SknSetState(skinDBP, skinButtonGameAudio,sknStateDisabled);
			SknShowObject(skinDBP, skinButtonGameAudio);
		}

	} else {
		if (SknGetState(skinDBP, skinButtonGameDelete) == sknStateDisabled) {
			SknSetState(skinDBP, skinButtonGameDelete,sknStateNormal);
			SknShowObject(skinDBP, skinButtonGameDelete);
		}
		if (SknGetState(skinDBP, skinButtonGameEdit) == sknStateDisabled) {
			SknSetState(skinDBP, skinButtonGameEdit,sknStateNormal);
			SknShowObject(skinDBP, skinButtonGameEdit);
		}
		if (SknGetState(skinDBP, skinButtonGameAudio) == sknStateDisabled) {
			SknSetState(skinDBP, skinButtonGameAudio,sknStateNormal);
			SknShowObject(skinDBP, skinButtonGameAudio);
		}
	}
}

static void SknRedrawSlider(DmOpenRef skinDBP, UInt16 index, UInt16 maxIndex, UInt16 perPage) {
	if (maxIndex <= perPage) {
		if (SknGetState(skinDBP,skinSliderUpArrow) != sknStateDisabled) {
			SknSetState(skinDBP,skinSliderUpArrow,sknStateDisabled);
			SknShowObject(skinDBP,skinSliderUpArrow);
		}
		if (SknGetState(skinDBP,skinSliderDownArrow) != sknStateDisabled) {
			SknSetState(skinDBP,skinSliderDownArrow,sknStateDisabled);
			SknShowObject(skinDBP,skinSliderDownArrow);
		}

	} else {
		if (SknGetState(skinDBP,skinSliderUpArrow) == sknStateDisabled) {
			SknSetState(skinDBP,skinSliderUpArrow,sknStateNormal);
			SknShowObject(skinDBP,skinSliderUpArrow);
		}
		if (SknGetState(skinDBP,skinSliderDownArrow) == sknStateDisabled) {
			SknSetState(skinDBP,skinSliderDownArrow,sknStateNormal);
			SknShowObject(skinDBP,skinSliderDownArrow);
		}
	}
}

void SknUpdateList() {
	MemHandle record;
	Int32 index, maxIndex, maxView;
	GameInfoType *game;
	RectangleType rArea, rField, rCopy, rArea2x;
	DmOpenRef skinDBP;

	UInt8 txtColor, norColor, selColor, bkgColor;
	UInt16 x,y;

	SknScreenLock(winLockCopy);

	SknGetListBounds(&rArea, &rArea2x);
	skinDBP = SknOpenSkin();
	// set default bg
	WinSetForeColor(UIColorGetTableEntryIndex (UIFormFill));
	WinDrawRectangle(&rArea,0);
	// copy top bg
	SknGetObjectBounds(skinDBP, skinBackgroundImageTop, &rField);
	RctGetIntersection(&rArea2x, &rField, &rCopy);
	x = rCopy.topLeft.x;
	y = rCopy.topLeft.y;
	rCopy.topLeft.x	-= rField.topLeft.x;
	rCopy.topLeft.y	-= rField.topLeft.y;
	SknCopyBits(skinDBP, skinBackgroundImageTop, &rCopy, x, y);
	// copy bottom bg
	SknGetObjectBounds(skinDBP, skinBackgroundImageBottom, &rField);
	RctGetIntersection(&rArea2x, &rField, &rCopy);
	x = rCopy.topLeft.x;
	y = rCopy.topLeft.y;
	rCopy.topLeft.x	-= rField.topLeft.x;
	rCopy.topLeft.y	-= rField.topLeft.y;
	SknCopyBits(skinDBP, skinBackgroundImageBottom, &rCopy, x, y);

	FntSetFont(stdFont);

	index = gPrefs->listPosition;
	maxIndex = DmNumRecords(gameDB);
	maxView = rArea.extent.y / sknInfoListItemSize;

	if (index > 0 && (index + maxView) > maxIndex)
		index -= (index + maxView) - maxIndex;

	if (index < 0)
		index = 0;

	gPrefs->listPosition = index;

	SknRedrawSlider(skinDBP, index, maxIndex, maxView);
	SknRedrawTools(skinDBP);
	SknGetListColors(skinDBP, &norColor, &selColor, &bkgColor);

	SknCloseSkin(skinDBP);

	while (index < (gPrefs->listPosition + maxView) && index < maxIndex) {
		record = DmQueryRecord(gameDB, index);
		game = (GameInfoType *)MemHandleLock(record);

		// text box
		RctSetRectangle(&rField, rArea.topLeft.x, (rArea.topLeft.y + sknInfoListItemSize * (index - gPrefs->listPosition)), rArea.extent.x, sknInfoListItemSize);
		WinSetClip(&rField);

		if (game->selected) {
			WinSetDrawMode(winPaint);
			WinSetForeColor(bkgColor);
			WinDrawRectangle(&rField,0);
			txtColor = selColor;
		}
		else
			txtColor = norColor;

		// clipping
		rField.topLeft.x += 2;
		rField.extent.x -= 4;
		WinSetClip(&rField);
		// draw text mask
		WinSetTextColor(255);
		WinSetDrawMode(winMask);
		WinPaintChars(game->nameP, StrLen(game->nameP), rField.topLeft.x, rField.topLeft.y);
		// draw text
		if (txtColor) {
			WinSetTextColor(txtColor);
			WinSetDrawMode(winOverlay);
			WinPaintChars(game->nameP, StrLen(game->nameP), rField.topLeft.x, rField.topLeft.y);
		}

		MemHandleUnlock(record);
		index++;
	}

	RctSetRectangle(&rArea,0,0,160,160);
	WinSetClip(&rArea);
	SknScreenUnlock();
}

UInt16 SknCheckClick(DmOpenRef skinDBP, Coord mx, Coord my) {
	UInt16 resID;
	RectangleType r;

	mx *= 2;
	my *= 2;

	if (skinDBP) {
		for (resID = 1100; resID <= 7000; resID += 100) {
			if (SknGetState(skinDBP, resID) != sknStateDisabled) {
				SknGetObjectBounds(skinDBP, resID, &r);
				if (RctPtInRectangle(mx, my, &r)) {
					return resID;
				}
			}
		}

	}

	return 0;
}

void SknSelect(Coord x, Coord y) {
	RectangleType rArea;
	SknGetListBounds(&rArea,0);

	if (RctPtInRectangle(x, y, &rArea)) {
		UInt16 index;
		MemHandle record;
		GameInfoType *game;
		UInt16 oldIndex;

		index = (y - rArea.topLeft.y) / sknInfoListItemSize + gPrefs->listPosition;

		if (index == lastIndex)
			return;

		if (index < DmNumRecords(gameDB)) {
			Boolean newValue;

			oldIndex = GamGetSelected();

			if (oldIndex != index && oldIndex != dmMaxRecordIndex)
				GamUnselect();

			record = DmGetRecord(gameDB, index);
			game = (GameInfoType *)MemHandleLock(record);

			newValue = !game->selected;
			DmWrite(game, OffsetOf(GameInfoType,selected), &newValue, sizeof(Boolean));

			MemHandleUnlock(record);
			DmReleaseRecord (gameDB, index, 0);

			lastIndex = index;
			SknUpdateList();
		}
	}
}

Boolean SknProcessArrowAction(UInt16 button) {
	Boolean handled = false;

	switch (button) {
		case skinSliderUpArrow:
			if (gPrefs->listPosition > 0) {
				gPrefs->listPosition--;
				SknUpdateList();
			}
			handled = true;
			break;

		case skinSliderDownArrow:
			RectangleType rArea;
			UInt16 maxView;

			SknGetListBounds(&rArea, 0);
			maxView = rArea.extent.y / sknInfoListItemSize;

			if (gPrefs->listPosition < DmNumRecords(gameDB)-maxView) {
				gPrefs->listPosition++;
				SknUpdateList();
			}
			handled = true;
			break;
	}

	return handled;
}
