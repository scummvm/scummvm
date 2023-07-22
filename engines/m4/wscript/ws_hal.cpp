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

#include "common/savefile.h"
#include "common/textconsole.h"
#include "m4/wscript/ws_hal.h"
#include "m4/wscript/wst_regs.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/dbg/debug.h"
#include "m4/graphics/gr_sprite.h"
#include "m4/graphics/rend.h"
#include "m4/gui/gui_vmng.h"
#include "m4/vars.h"
#include "m4/m4.h"

namespace M4 {

bool ws_InitHAL() {
	_GWS(deadRectList) = nullptr;
	return true;
}

void ws_KillHAL() {
	vmng_DisposeRectList(&_GWS(deadRectList));
}

void ws_DumpMachine(machine *m, Common::WriteStream *logFile) {
	Anim8 *myAnim8;
	CCB *myCCB;
	frac16 *myRegs;
	int32 i;
	double   tempFloat;

	if (!m || !logFile)
		return;

	// Print out the machine name, hash, and physical address
	logFile->writeString(Common::String::format("Machine Name: %s\n\tHash: %ld\n\tAddress: 0x%08lx\n\n", m->machName, m->myHash, (int32)m));

	// If we have an anim8 for this machine
	if (m->myAnim8) {
		myAnim8 = m->myAnim8;

		// Print out the anim8 hash, and physical address
		logFile->writeString(Common::String::format("Sequence Hash: %ld\n\tAddress: 0x%08lx\n\n", myAnim8->sequHash, (int32)myAnim8));

		// And if this anim8 has registers
		if (myAnim8->myRegs) {
			myRegs = myAnim8->myRegs;
			logFile->writeString("Registers:\n");

			// Loop through the main set of registers, and dump out the contents
			for (i = 0; i < IDX_COUNT; i++) {
				tempFloat = (float)(myRegs[i] >> 16) + (float)((float)(myRegs[i] & 0xffff) / (float)65536);
				logFile->writeString(Common::String::format("\t%ld\t%s:\t\t%.2f\t\t0x%08lx\n", i, myRegLabels[i], tempFloat, myRegs[i]));
			}

			// If the anim8 has extra local regs
			if (myAnim8->numLocalVars > 0) {
				for (i = IDX_COUNT; i < IDX_COUNT + myAnim8->numLocalVars; i++) {
					tempFloat = (float)(myRegs[i] >> 16) + (float)((float)(myRegs[i] & 0xffff) / (float)65536);
					logFile->writeString(Common::String::format("\t%ld\tlocal.%ld:\t\t%.2f\t\t0x%08lx\n", i, i - IDX_COUNT, tempFloat, myRegs[i]));
				}
			}
			logFile->writeString(Common::String::format("\n"));
		}

		// If this anim8 has a CCB
		if (myAnim8->myCCB) {
			myCCB = myAnim8->myCCB;

			logFile->writeString(Common::String::format("Sprite Series Name: %s\tAddress:0x%08lx\tFlags0x%08lx\n", myCCB->seriesName, (uint32)myCCB, myCCB->flags));
			logFile->writeString(Common::String::format("\tCurrent Location: (%ld, %ld), (%ld, %ld)\n", myCCB->currLocation->x1, myCCB->currLocation->y1,
				myCCB->currLocation->x2, myCCB->currLocation->y2));
			logFile->writeString(Common::String::format("\tNew Location: (%ld, %ld), (%ld, %ld)\n", myCCB->newLocation->x1, myCCB->newLocation->y1,
				myCCB->newLocation->x2, myCCB->newLocation->y2));
			logFile->writeString(Common::String::format("\tscale: %ld\n", myCCB->scaleX));
			logFile->writeString(Common::String::format("\tlayer: %ld\n", myCCB->layer));
		}
	}
}

void ws_Error(machine *m, int32 errorType, trigraph errorCode, const char *errMsg) {
	Common::OutSaveFile *logFile;
	char  description[MAX_STRING_SIZE];

	// Find the error description
	error_look_up(errorCode, description);

	// Open the logFile
	logFile = g_system->getSavefileManager()->openForSaving("ws_mach.log");

	// Give the WS debugger a chance to indicate the error to the apps programmer
	dbg_WSError(logFile, m, errorType, description, errMsg, _GWS(pcOffsetOld));

	// Dump out the machine to the logFile
	ws_DumpMachine(m, logFile);

	// Close the logFile
	if (logFile)
		f_io_close(logFile);

	// Now we fatal abort
	error_show(FL, errorCode, errMsg);
}

void ws_LogErrorMsg(char *sourceFile, int32 lineNum, const char *fmt, ...) {
	Common::OutSaveFile *logFile;
	Common::String msgBuff;
	va_list ap;

	va_start(ap, fmt);
	msgBuff == Common::String::vformat(fmt, ap);
	va_end(ap);

	logFile = g_system->getSavefileManager()->openForSaving("ws_mach.log");
	if (logFile) {
		logFile->writeString(Common::String::format("Source Code Index: %s:%ld\n", sourceFile, lineNum));
		logFile->writeString(Common::String::format("%s\n\n", msgBuff.c_str()));

		logFile->finalize();
		delete logFile;
	}

	term_message("Source Code Index: %s:%ld", sourceFile, lineNum);
	term_message("%s", msgBuff.c_str());
}

machine *kernel_timer_callback(int32 ticks, int16 trigger, MessageCB callMe) {
	_GWS(ws_globals)[GLB_TEMP_1] = (frac16)(ticks << 16);
	_GWS(ws_globals)[GLB_TEMP_2] = (frac16)trigger;

	return (TriggerMachineByHash(1, nullptr, -1, -1, callMe, false, "timer callback"));
}

static void DrawSprite(CCB *myCCB, Anim8 *myAnim8, Buffer *halScrnBuf, GrBuff *screenCodeBuff,
		uint8 *myPalette, uint8 *ICT, M4Rect *clipRect, M4Rect *updateRect) {
	M4sprite *source;

	// Temporary var to prevent excessive dereferences
	source = myCCB->source;

	if (!(myCCB->flags & CCB_DISC_STREAM)) {
		// Make sure the sprite is still in memory
		if (!source->sourceHandle || !*(source->sourceHandle)) {
			ws_LogErrorMsg(FL, "Sprite series is no longer in memory.");
			ws_Error(myAnim8->myMachine, ERR_INTERNAL, 0x02ff, "Error during ws_DoDisplay()");
		}

		// Lock the sprite handle
		HLock(source->sourceHandle);
		source->data = (uint8 *)((byte *)*(source->sourceHandle) + source->sourceOffset);
	}

	Buffer Destination;
	DrawRequest dr;
	RendCell Frame;

	Destination.w = source->w;
	Destination.h = source->h;
	Destination.stride = source->w;
	Destination.encoding = (myPalette && ICT) ? source->encoding : source->encoding & 0x7f;
	Destination.data = source->data;

	dr.Src = &Destination;
	dr.Dest = halScrnBuf;
	dr.x = myAnim8->myRegs[IDX_X] >> 16;
	dr.y = myAnim8->myRegs[IDX_Y] >> 16;
	dr.scaleX = myCCB->scaleX;
	dr.scaleY = myCCB->scaleY;
	dr.depthCode = screenCodeBuff->get_pixmap();		// TODO: Confirm this
	dr.Pal = myPalette;
	dr.ICT = ICT;
	dr.srcDepth = myCCB->layer >> 8;

	Frame.hot_x = myCCB->source->xOffset;
	Frame.hot_y = myCCB->source->yOffset;
	Frame.Width = source->w;
	Frame.Height = source->h;

	if ((!myPalette) || (!ICT)) {
		Frame.Comp = (uint32)(source->encoding & 0x7f);
	} else {
		Frame.Comp = (uint32)source->encoding;
	}
	Frame.data = source->data;

	// And draw the sprite
	gr_sprite_draw(&dr);
	myCCB->flags &= ~CCB_REDRAW;

	if (!(myCCB->flags & CCB_DISC_STREAM))
		// Unlock the sprite's handle
		HUnLock(source->sourceHandle);
}

void ws_DoDisplay(Buffer *background, int16 *depth_table, GrBuff *screenCodeBuff,
		uint8 *myPalette, uint8 *ICT, bool updateVideo) {
	CCB *myCCB;
	ScreenContext *myScreen;
	RectList *myRect;
	RectList *drawRectList = nullptr;
	int32 status, scrnX1, scrnY1;
	int32 restoreBgndX1, restoreBgndY1, restoreBgndX2, restoreBgndY2;
	Anim8 *myAnim8;
	M4Rect *currRect, intersectRect, noClipRect, dummyRect;
	bool greyMode;

	if (((myScreen = vmng_screen_find(_G(gameDrawBuff), &status)) == nullptr) || (status != SCRN_ACTIVE)) {
		return;
	}

	Buffer *halScrnBuf = _G(gameDrawBuff)->get_buffer();
	noClipRect.x1 = 0;
	noClipRect.y1 = 0;
	noClipRect.x2 = halScrnBuf->w - 1;
	noClipRect.y2 = halScrnBuf->h - 1;

	scrnX1 = myScreen->x1;
	scrnY1 = myScreen->y1;

	greyMode = krn_GetGreyMode();

	// Intialize the drawRectList to the deadRectList
	drawRectList = _GWS(deadRectList);
	_GWS(deadRectList) = nullptr;

	// The drawRectList already contains all the areas of the screen that need the
	// background updated
	// Update the background behind the current rect list - if we are in greyMode, we do this later
	if (background && background->data) {
		myRect = drawRectList;
		while (myRect) {
			restoreBgndX1 = imath_max(myRect->x1, 0);
			restoreBgndY1 = imath_max(myRect->y1, 0);
			restoreBgndX2 = imath_min(myRect->x2, background->w - 1);
			restoreBgndY2 = imath_min(myRect->y2, background->h - 1);
			gr_buffer_rect_copy(background, halScrnBuf, restoreBgndX1, restoreBgndY1,
				restoreBgndX2 - restoreBgndX1 + 1, restoreBgndY2 - restoreBgndY1 + 1);
			myRect = myRect->next;
		}
	}

	// Now we loop back to front and update the area of each sprite that intersects the update list,
	// or simply draw the sprite if it has been marked for redraw
	myAnim8 = _GWS(myCruncher)->backLayerAnim8;
	while (myAnim8) {
		myCCB = myAnim8->myCCB;
		currRect = myCCB->currLocation;
		if (myCCB && myCCB->source && (!(myCCB->flags & CCB_NO_DRAW))) {
			if (myCCB->flags & CCB_REDRAW) {
				// Draw the sprite
				DrawSprite(myCCB, myAnim8, halScrnBuf, screenCodeBuff, myPalette, ICT, &noClipRect, currRect);

				// Add it's new location to the update list
				vmng_AddRectToRectList(&drawRectList, currRect->x1, currRect->y1, currRect->x2, currRect->y2);
			} else {
				// Loop through the update list, intersect each rect with the sprites
				// current location, and update redraw all overlapping areas
				myRect = drawRectList;
				while (myRect) {
					intersectRect.x1 = imath_max(myRect->x1, currRect->x1);
					intersectRect.y1 = imath_max(myRect->y1, currRect->y1);
					intersectRect.x2 = imath_min(myRect->x2, currRect->x2);
					intersectRect.y2 = imath_min(myRect->y2, currRect->y2);

					// Now see if there was an intersection
					if ((intersectRect.x1 <= intersectRect.x2) && (intersectRect.y1 <= intersectRect.y2)) {
						// Draw just the intersected region
						DrawSprite(myCCB, myAnim8, halScrnBuf, screenCodeBuff, myPalette, ICT, &intersectRect, &dummyRect);
					}

					// Next rect
					myRect = myRect->next;
				}
			}
		}
		myAnim8 = myAnim8->infront;
	}

	myRect = drawRectList;
	while (myRect) {
		if (updateVideo) {
			if (greyMode) {
				krn_UpdateGreyArea(halScrnBuf, scrnX1, scrnY1, myRect->x1, myRect->y1, myRect->x2, myRect->y2);
			}
			RestoreScreens(scrnX1 + myRect->x1, scrnY1 + myRect->y1, scrnX1 + myRect->x2, scrnY1 + myRect->y2);
		}
		myRect = myRect->next;
	}

	_G(gameDrawBuff)->release();

	// Turf the drawRectList
	vmng_DisposeRectList(&drawRectList);
}

void ws_hal_RefreshWoodscriptBuffer(cruncher *myCruncher, Buffer *background, int16 *depth_table, GrBuff *screenCodes, uint8 *myPalette, uint8 *ICT) {
	error("TODO: ws_hal_RefreshWoodscriptBuffer");
}

void GetBezCoeffs(frac16 *ctrlPoints, frac16 *coeffs) {
	frac16 x0, x0mult3, x1mult3, x1mult6, x2mult3, x3;
	frac16 y0, y0mult3, y1mult3, y1mult6, y2mult3, y3;

	x0 = ctrlPoints[0];
	x0mult3 = (x0 << 1) + x0;
	x1mult3 = (ctrlPoints[2] << 1) + ctrlPoints[2];
	x1mult6 = x1mult3 << 1;
	x2mult3 = (ctrlPoints[4] << 1) + ctrlPoints[4];
	x3 = ctrlPoints[6];

	y0 = ctrlPoints[1];
	y0mult3 = (y0 << 1) + y0;
	y1mult3 = (ctrlPoints[3] << 1) + ctrlPoints[3];
	y1mult6 = y1mult3 << 1;
	y2mult3 = (ctrlPoints[5] << 1) + ctrlPoints[5];
	y3 = ctrlPoints[7];

	coeffs[0] = -(int)x0 + x1mult3 - x2mult3 + x3;
	coeffs[2] = x0mult3 - x1mult6 + x2mult3;
	coeffs[4] = -(int)x0mult3 + x1mult3;
	coeffs[6] = x0;

	coeffs[1] = -(int)y0 + y1mult3 - y2mult3 + y3;
	coeffs[3] = y0mult3 - y1mult6 + y2mult3;
	coeffs[5] = -(int)y0mult3 + y1mult3;
	coeffs[7] = y0;

	return;
}

void GetBezPoint(frac16 *x, frac16 *y, frac16 *coeffs, frac16 tVal) {

	*x = coeffs[6] +
		MulSF16(tVal, (coeffs[4] +
			MulSF16(tVal, (coeffs[2] +
				MulSF16(tVal, coeffs[0])))));

	*y = coeffs[7] +
		MulSF16(tVal, (coeffs[5] +
			MulSF16(tVal, (coeffs[3] +
				MulSF16(tVal, coeffs[1])))));

	return;
}

bool InitCCB(CCB *myCCB) {
	myCCB->flags = CCB_SKIP;
	myCCB->source = nullptr;
	if ((myCCB->currLocation = (M4Rect *)mem_alloc(sizeof(M4Rect), "Rectangle")) == nullptr) {
		return false;
	}
	myCCB->currLocation->x1 = -1;
	myCCB->currLocation->y1 = -1;
	myCCB->currLocation->x2 = -1;
	myCCB->currLocation->y2 = -1;
	if ((myCCB->newLocation = (M4Rect *)mem_alloc(sizeof(M4Rect), "Rectangle")) == nullptr) {
		return false;
	}
	myCCB->newLocation->x1 = -1;
	myCCB->newLocation->y1 = -1;
	myCCB->newLocation->x2 = -1;
	myCCB->newLocation->y2 = -1;

	myCCB->maxArea = nullptr;
	myCCB->scaleX = 0;
	myCCB->scaleY = 0;
	myCCB->layer = 0;
	myCCB->streamSSHeader = nullptr;
	myCCB->streamSpriteSource = nullptr;
	myCCB->myStream = nullptr;
	myCCB->seriesName = nullptr;

	return true;
}

void HideCCB(CCB *myCCB) {
	if (!myCCB)
		return;
	myCCB->flags |= CCB_HIDE;

	if ((myCCB->flags & CCB_STREAM) && myCCB->maxArea) {
		vmng_AddRectToRectList(&_GWS(deadRectList), myCCB->maxArea->x1, myCCB->maxArea->y1, myCCB->maxArea->x2, myCCB->maxArea->y2);
		mem_free(myCCB->maxArea);
		myCCB->maxArea = nullptr;
	} else {
		vmng_AddRectToRectList(&_GWS(deadRectList), myCCB->currLocation->x1, myCCB->currLocation->y1, myCCB->currLocation->x2, myCCB->currLocation->y2);
	}
}

void ShowCCB(CCB *myCCB) {
	if (!myCCB)
		return;

	myCCB->flags &= ~CCB_HIDE;
}

void MoveCCB(CCB *myCCB, frac16 deltaX, frac16 deltaY) {
	if (!myCCB || !myCCB->source) {
		error_show(FL, 'WSIC');
	}

	myCCB->newLocation->x1 = myCCB->currLocation->x1 + (deltaX >> 16);
	myCCB->newLocation->y1 = myCCB->currLocation->y1 + (deltaY >> 16);
	myCCB->newLocation->x2 = myCCB->currLocation->x2 + (deltaX >> 16);
	myCCB->newLocation->y2 = myCCB->currLocation->y2 + (deltaY >> 16);

	if (myCCB->flags & CCB_STREAM) {
		if (!myCCB->maxArea) {
			if ((myCCB->maxArea = (M4Rect *)mem_alloc(sizeof(M4Rect), "Rectangle")) == nullptr) {
				error_show(FL, 'OOM!');
			}
			myCCB->maxArea->x1 = myCCB->newLocation->x1;
			myCCB->maxArea->y1 = myCCB->newLocation->y1;
			myCCB->maxArea->x2 = myCCB->newLocation->x2;
			myCCB->maxArea->y2 = myCCB->newLocation->y2;
		} else {
			myCCB->maxArea->x1 = imath_min(myCCB->maxArea->x1, myCCB->newLocation->x1);
			myCCB->maxArea->y1 = imath_min(myCCB->maxArea->y1, myCCB->newLocation->y1);
			myCCB->maxArea->x2 = imath_max(myCCB->maxArea->x2, myCCB->newLocation->x2);
			myCCB->maxArea->y2 = imath_max(myCCB->maxArea->y2, myCCB->newLocation->y2);
		}
	}

	if ((myCCB->source->w != 0) && (myCCB->source->h != 0)) {
		myCCB->flags |= CCB_REDRAW;
	}
}

void KillCCB(CCB *myCCB, bool restoreFlag) {
	if (!myCCB) {
		error_show(FL, 'WSIC');
	}
	if (restoreFlag && (!(myCCB->flags & CCB_SKIP)) && (!(myCCB->flags & CCB_HIDE))) {
		if ((myCCB->flags & CCB_STREAM) && myCCB->maxArea) {
			vmng_AddRectToRectList(&_GWS(deadRectList), myCCB->maxArea->x1, myCCB->maxArea->y1,
				myCCB->maxArea->x2, myCCB->maxArea->y2);
		} else {
			vmng_AddRectToRectList(&_GWS(deadRectList), myCCB->currLocation->x1, myCCB->currLocation->y1,
				myCCB->currLocation->x2, myCCB->currLocation->y2);
		}
	}
	if (myCCB->flags & CCB_DISC_STREAM) {
		ws_CloseSSstream(myCCB);
	}
	if (myCCB->currLocation) {
		mem_free(myCCB->currLocation);
	}
	if (myCCB->newLocation) {
		mem_free(myCCB->newLocation);
	}
	if (myCCB->maxArea) {
		mem_free(myCCB->maxArea);
	}
	if (myCCB->source) {
		mem_free(myCCB->source);
	}

	mem_free(myCCB);
}

void Cel_msr(Anim8 *myAnim8) {
	CCB *myCCB;
	frac16 *myRegs;
	int32    scaler;

	if (!myAnim8) {
		error_show(FL, 'WSAI');
	}

	myCCB = myAnim8->myCCB;
	if ((!myCCB) || (!myCCB->source)) {
		error_show(FL, 'WSIC');
	}

	if ((myCCB->source->w == 0) || (myCCB->source->h == 0)) {
		return;
	}

	myRegs = myAnim8->myRegs;
	if (!myRegs) {
		error_show(FL, 'WSAI');
	}

	scaler = FixedMul(myRegs[IDX_S], 100 << 16) >> 16;

	myCCB->scaleX = myRegs[IDX_W] < 0 ? -scaler : scaler;
	myCCB->scaleY = scaler;

	GetUpdateRectangle(myRegs[IDX_X] >> 16, myRegs[IDX_Y] >> 16, myCCB->source->xOffset, myCCB->source->yOffset,
		myCCB->scaleX, myCCB->scaleY, myCCB->source->w, myCCB->source->h, myCCB->newLocation);
	if (myCCB->flags & CCB_STREAM) {
		if (!myCCB->maxArea) {
			if ((myCCB->maxArea = (M4Rect *)mem_alloc(sizeof(M4Rect), "Rectangle")) == nullptr) {
				error_show(FL, 'OOM!');
			}
			myCCB->maxArea->x1 = myCCB->newLocation->x1;
			myCCB->maxArea->y1 = myCCB->newLocation->y1;
			myCCB->maxArea->x2 = myCCB->newLocation->x2;
			myCCB->maxArea->y2 = myCCB->newLocation->y2;
		} else {
			myCCB->maxArea->x1 = imath_min(myCCB->maxArea->x1, myCCB->newLocation->x1);
			myCCB->maxArea->y1 = imath_min(myCCB->maxArea->y1, myCCB->newLocation->y1);
			myCCB->maxArea->x2 = imath_max(myCCB->maxArea->x2, myCCB->newLocation->x2);
			myCCB->maxArea->y2 = imath_max(myCCB->maxArea->y2, myCCB->newLocation->y2);
		}
	} else {
		vmng_AddRectToRectList(&_GWS(deadRectList), myCCB->currLocation->x1, myCCB->currLocation->y1,
			myCCB->currLocation->x2, myCCB->currLocation->y2);
	}

	myAnim8->flags &= ~(TAG_MAP_CEL | TAG_MOVE_CEL);
	myCCB->layer = imath_max(0, myAnim8->myLayer);
	myCCB->flags &= ~CCB_SKIP;
	myCCB->flags |= CCB_REDRAW;
	return;
}

void ws_OverrideCrunchTime(machine *m) {
	if ((!m) || (!m->myAnim8)) {
		return;
	}
	m->myAnim8->switchTime = 0;
}

} // End of namespace M4
