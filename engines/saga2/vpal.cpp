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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "common/debug.h"
#include "common/savefile.h"
#include "graphics/paletteman.h"

#include "saga2/saga2.h"
#include "saga2/fta.h"
#include "saga2/vpal.h"
#include "saga2/palette.h"
#include "saga2/display.h"
#include "saga2/hresmgr.h"
#include "saga2/saveload.h"

namespace Saga2 {

const uint32            paletteID   = MKTAG('P', 'A', 'L',  0);

/* ===================================================================== *
   Imports
 * ===================================================================== */

extern hResContext      *tileRes;           // tile resource handle
extern volatile int32   gameTime;

/* ===================================================================== *
   Functions
 * ===================================================================== */

PaletteManager::PaletteManager() {
	_midnightPalette = nullptr;
	_noonPalette = nullptr;
	_darkPalette = nullptr;
	_prevLightLevel = 0;

	memset(_newPalette.entry, 0, sizeof(_newPalette.entry));
	memset(_currentPalette.entry, 0, sizeof(_currentPalette.entry));
	memset(_oldPalette.entry, 0, sizeof(_oldPalette.entry));
	memset(_destPalette.entry, 0, sizeof(_destPalette.entry));
	memset(_quickPalette.entry, 0, sizeof(_quickPalette.entry));

	_startTime = _totalTime = 0;
}

void PaletteManager::assertCurrentPalette() {
	if (paletteChangesEnabled()) {
		byte palette[256 * 3];
		for (int i = 0; i < 256; i++) {
			palette[i * 3 + 0] = ((byte *)&_currentPalette)[i * 3 + 0] << 2;
			palette[i * 3 + 1] = ((byte *)&_currentPalette)[i * 3 + 1] << 2;
			palette[i * 3 + 2] = ((byte *)&_currentPalette)[i * 3 + 2] << 2;
		}
		g_system->getPaletteManager()->setPalette(palette, 0, 256);
	}
}

void gPalette::read(Common::InSaveFile *in) {
	for (int i = 0; i < 256; ++i) {
		entry[i].r = in->readByte();
		entry[i].g = in->readByte();
		entry[i].b = in->readByte();
	}
}

void gPalette::write(Common::MemoryWriteStreamDynamic *out) {
	for (int i = 0; i < 256; ++i) {
		out->writeByte(entry[i].r);
		out->writeByte(entry[i].g);
		out->writeByte(entry[i].b);
	}
}

//----------------------------------------------------------------------
//	Initialize global palette resources

static inline int16 bscale(int16 s) {
	if (s < 32) return s * 3 / 2;
	else return (63 + s) / 2;
}

void PaletteManager::loadPalettes() {
	int     i;

	//  Create a black palette for fades
	g_vm->_pal->_darkPalette = new gPalette;
	memset(g_vm->_pal->_darkPalette, 0, sizeof(gPalette));


	//  Load standard palette
	g_vm->_pal->_noonPalette = (gPalettePtr)LoadResource(tileRes, paletteID, "noon palette");

	//  Create a midnight palette for night time effect
	g_vm->_pal->_midnightPalette = new gPalette;

	gPalette    *dayPal = g_vm->_pal->_noonPalette;
	gPalette    *nightPal = g_vm->_pal->_midnightPalette;

	// these colors darkened
	for (i = 10; i < 240; i++) {
//		nightPal->entry[i].r = dayPal->entry[i].r / 2;
//		nightPal->entry[i].g = dayPal->entry[i].g * 2 / 3;
//		nightPal->entry[i].b = dayPal->entry[i].b;

		nightPal->entry[i].r = dayPal->entry[i].r / 3;
		nightPal->entry[i].g = dayPal->entry[i].g / 2;
		nightPal->entry[i].b = bscale(dayPal->entry[i].b);
	}

	// these colors are not
	for (i = 0; i < 10; i++) {
		nightPal->entry[i].r = dayPal->entry[i].r;
		nightPal->entry[i].g = dayPal->entry[i].g;
		nightPal->entry[i].b = dayPal->entry[i].b;
	}

	// and these colors are not
	for (i = 240; i < 256; i++) {
		nightPal->entry[i].r = dayPal->entry[i].r;
		nightPal->entry[i].g = dayPal->entry[i].g;
		nightPal->entry[i].b = dayPal->entry[i].b;
	}

	// single color additions to palette shift
	nightPal->entry[244].r = dayPal->entry[244].r / 3;
	nightPal->entry[244].g = dayPal->entry[244].g / 2;
	nightPal->entry[244].b = bscale(dayPal->entry[244].b);
}

//----------------------------------------------------------------------
//	Dump global palette resources

void PaletteManager::cleanupPalettes() {
	if (g_vm->_pal->_noonPalette) {
		free(g_vm->_pal->_noonPalette);
		g_vm->_pal->_noonPalette = nullptr;
	}

	if (g_vm->_pal->_darkPalette) {
		delete g_vm->_pal->_darkPalette;
		g_vm->_pal->_darkPalette = nullptr;
	}

	if (g_vm->_pal->_midnightPalette) {
		delete g_vm->_pal->_midnightPalette;
		g_vm->_pal->_midnightPalette = nullptr;
	}
}

//----------------------------------------------------------------------
//	Begin fade up/down

void PaletteManager::beginFade(gPalettePtr newPalette, int32 fadeDuration) {
	_startTime = gameTime;
	_totalTime = fadeDuration;

	//  Save the current palette for interpolation
	memcpy(&_oldPalette, &_currentPalette, sizeof(gPalette));

	//  Copy the destination palette
	memcpy(&_destPalette, newPalette, sizeof(gPalette));
}

//----------------------------------------------------------------------
//	Update state of palette fade up/down

bool PaletteManager::updatePalette() {
	int32           elapsedTime;

	elapsedTime = gameTime - _startTime;
	if (_totalTime == 0)
		return false;

	if (elapsedTime >= _totalTime) {
		//  Fade is completed
		_totalTime = 0;
		memcpy(&_currentPalette, &_destPalette, sizeof(gPalette));
		assertCurrentPalette();
		return false;
	} else {
		gPalette        tempPalette;

		debugC(2, kDebugPalettes, "Fade: %d/%d", elapsedTime, _totalTime);

		createPalette(
		    &tempPalette,
		    &_oldPalette,
		    &_destPalette,
		    elapsedTime,
		    _totalTime);

		if (memcmp(&tempPalette, &_currentPalette, sizeof(gPalette)) != 0) {
			debugC(2, kDebugPalettes, "Fade:*%d/%d", elapsedTime, _totalTime);

			memcpy(&_currentPalette, &tempPalette, sizeof(gPalette));
			assertCurrentPalette();

			g_system->updateScreen();
		}
		g_system->delayMillis(10);

		return true;
	}
}

//----------------------------------------------------------------------
//	Linearly interpolate between two specified palettes

void PaletteManager::createPalette(
    gPalettePtr newP,
    gPalettePtr srcP,
    gPalettePtr dstP,
    int32       elapsedTime,
    int32       totalTime) {
	assert(totalTime != 0);

	int             i;
	uint32          fadeProgress = (elapsedTime << 8) / totalTime;

	for (i = 0; i < (long)ARRAYSIZE(newP->entry); i++) {
		gPaletteEntry   *srcPal = &srcP->entry[i];
		gPaletteEntry   *dstPal = &dstP->entry[i];
		gPaletteEntry   *curPal = &newP->entry[i];

		//  Linearly interpolate between the source and end palette.
		curPal->r = srcPal->r + (((dstPal->r - srcPal->r) * fadeProgress) >> 8);
		curPal->g = srcPal->g + (((dstPal->g - srcPal->g) * fadeProgress) >> 8);
		curPal->b = srcPal->b + (((dstPal->b - srcPal->b) * fadeProgress) >> 8);
	}
}

//----------------------------------------------------------------------
//	Set the current palette

void PaletteManager::setCurrentPalette(gPalettePtr newPal) {
	memcpy(&_currentPalette, newPal, sizeof(gPalette));
	assertCurrentPalette();
}

//----------------------------------------------------------------------
//	Return the current palette

void PaletteManager::getCurrentPalette(gPalettePtr pal) {
	memcpy(pal, &_currentPalette, sizeof(gPalette));
}

//----------------------------------------------------------------------
//	Initialize the state of the current palette and fade up/down.

void PaletteManager::initPaletteState() {
	//setCurrentPalette( *noonPalette );
	_totalTime = _startTime = 0;
}

//----------------------------------------------------------------------
// routines to suspend & restore a palette state (during videos)

void PaletteManager::lightsOut() {
	memset(&_currentPalette, 0, sizeof(_currentPalette));
	assertCurrentPalette();
}

//----------------------------------------------------------------------
// routines to suspend & restore a palette state (during videos)

void PaletteManager::quickSavePalette() {
	memcpy(&_quickPalette, &_currentPalette, sizeof(gPalette));
}

void PaletteManager::quickRestorePalette() {
	memcpy(&_currentPalette, &_quickPalette, sizeof(gPalette));
	assertCurrentPalette();
}

void PaletteManager::savePaletteState(Common::OutSaveFile *outS) {
	debugC(2, kDebugSaveload, "Saving Palette States");

	outS->write("PALE", 4);

	CHUNK_BEGIN;
	_currentPalette.write(out);
	_oldPalette.write(out);
	_destPalette.write(out);
	out->writeSint32LE(_startTime);
	out->writeSint32LE(_totalTime);
	CHUNK_END;

	debugC(3, kDebugSaveload, "... _startTime = %d", _startTime);
	debugC(3, kDebugSaveload, "... _totalTime = %d", _totalTime);
}

void PaletteManager::loadPaletteState(Common::InSaveFile *in) {
	debugC(2, kDebugSaveload, "Loading Palette States");

	gPalette tempPalette;

	tempPalette.read(in);
	_oldPalette.read(in);
	_destPalette.read(in);
	_startTime = in->readSint32LE();
	_totalTime = in->readSint32LE();

	debugC(3, kDebugSaveload, "... _startTime = %d", _startTime);
	debugC(3, kDebugSaveload, "... _totalTime = %d", _totalTime);

	setCurrentPalette(&tempPalette);
}

void initPaletteState() {
	g_vm->_pal->initPaletteState();
}

void savePaletteState(Common::OutSaveFile *outS) {
	g_vm->_pal->savePaletteState(outS);
}

void loadPaletteState(Common::InSaveFile *in) {
	g_vm->_pal->loadPaletteState(in);
}

} // end of namespace Saga2
