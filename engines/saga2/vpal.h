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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_VPAL_H
#define SAGA2_VPAL_H

#include "saga2/idtypes.h"

namespace Saga2 {

//  Stores an RGB value

struct gPaletteEntry {
	uint8           r,
	                g,
	                b;
};

//  An entire palette of 256 colors

struct gPalette {
	gPaletteEntry   entry[256];

	void read(Common::InSaveFile *in);
	void write(Common::MemoryWriteStreamDynamic *out);
};

typedef gPalette *gPalettePtr;

/* ===================================================================== *
                             Prototypes
 * ===================================================================== */

void LoadPalette(gPalette &palette);
int32 ColorDistance(gPaletteEntry &c1, gPaletteEntry &c2);
void CalcPens(gPalette &, gPaletteEntry *, gPen *, int16, bool);

class PaletteManager {
private:
	gPalette _currentPalette;

	gPalette _oldPalette,
             _destPalette,
             _quickPalette;

	int32 _startTime,
		  _totalTime;


public:
	gPalette _newPalette;
	gPalettePtr _midnightPalette,
			    _noonPalette,
			    _darkPalette;

	uint32 _prevLightLevel;

	PaletteManager();
	//~PaletteManager() {}

	void assertCurrentPalette();
	void loadPalettes();
	void cleanupPalettes();
	void beginFade(gPalettePtr newPalette, int32 fadeDuration);
	bool updatePalette();
	void createPalette(gPalettePtr newP, gPalettePtr srcP, gPalettePtr dstP,
	                   int32 elapsedTime, int32 totalTime_);
	void setCurrentPalette(gPalettePtr newPal);
	void getCurrentPalette(gPalettePtr pal);
	void initPaletteState();
	void lightsOut();
	void quickSavePalette();
	void quickRestorePalette();
	void savePaletteState(Common::OutSaveFile *outS);
	void loadPaletteState(Common::InSaveFile *in);
};

} // end of namespace Saga2

#endif
