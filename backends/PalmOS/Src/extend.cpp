/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002-2006 Chris Apers - PalmOS Backend
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
 * $URL$
 * $Id$
 *
 */
 
#include <stdlib.h>
#include "globals.h"

#include "modulesrsc.h"

const Char *SCUMMVM_SAVEPATH = "/PALM/Programs/ScummVM/Saved";

void PalmFatalError(const Char *err) {
	WinSetDrawWindow(WinGetDisplayWindow());
	WinPalette(winPaletteSetToDefault,0,0,0);
	WinSetBackColor(0);
	WinEraseWindow();
	FrmCustomAlert(FrmFatalErrorAlert, err, 0,0);
}

void DrawStatus(Boolean show) {
	if (OPTIONS_TST(kOptDisableOnScrDisp))
		return;

	UInt8 x,y;	
	UInt32 depth, d1;
	Boolean d2;
	WinScreenMode(winScreenModeGet, &d1, &d1, &depth, &d2);
	Int16 color = (show ? gVars->indicator.on : gVars->indicator.off);

	if (depth == 8) {
		UInt8 *src = (UInt8 *)BmpGetBits(WinGetBitmap(WinGetDisplayWindow()));
		src += gVars->screenPitch + 1;
		for(y=0; y < 4; y++) {
			for(x=0; x < 4; x++)
				src[x] = color;

			src += gVars->screenPitch;
		}

	} else if (depth == 16) {
		Int16 *src = (Int16 *)BmpGetBits(WinGetBitmap(WinGetDisplayWindow()));
		src += gVars->screenPitch + 1;
		for(y=0; y < 4; y++) {
			for(x=0; x < 4; x++)
				src[x] = color;

			src += gVars->screenPitch;
		}
	}
}

#ifndef PALMOS_ARM

// This is now required since some classes are now very big :)
#include "MemGlue.h"
void *operator new(UInt32 size) {
	void *ptr = MemGluePtrNew(size);
	MemSet(ptr, 0, size);
	return ptr;
}

void *operator new [] (UInt32 size) {
	void *ptr = MemGluePtrNew(size);
	MemSet(ptr, 0, size);
	return ptr;
}
#elif defined(COMPILE_OS5)

__inline void *operator new(UInt32 size) {
	void *ptr = malloc(size);
	MemSet(ptr, 0, size);
	return ptr;
}

__inline void *operator new [] (UInt32 size) {
	void *ptr = malloc(size);
	MemSet(ptr, 0, size);
	return ptr;
}

__inline void operator delete(void *ptr) throw() {
	if (ptr) free(ptr);
}

__inline void operator delete[](void *ptr) throw() {
	if (ptr) free(ptr);
}
#endif
