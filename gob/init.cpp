/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#include "gob/gob.h"
#include "gob/dataio.h"
#include "gob/global.h"
#include "gob/init.h"
#include "gob/video.h"
#include "gob/sound.h"
#include "gob/timer.h"
#include "gob/sound.h"
#include "gob/game.h"
#include "gob/draw.h"
#include "gob/util.h"
#include "gob/cdrom.h"

namespace Gob {

void game_start(void);

extern int16 debugFlag;
extern int16 inVM;
extern int16 colorCount;

PalDesc *init_palDesc;

static const char *init_fontNames[] =
    { "jeulet1.let", "jeulet2.let", "jeucar1.let", "jeumath.let" };

void init_findBestCfg(void) {
	videoMode = VIDMODE_VGA;
	useMouse = mousePresent;
	if (presentSound & BLASTER_FLAG)
		soundFlags = BLASTER_FLAG | SPEAKER_FLAG | MIDI_FLAG;
	else if (presentSound & PROAUDIO_FLAG)
		soundFlags = PROAUDIO_FLAG | SPEAKER_FLAG | MIDI_FLAG;
	else if (presentSound & ADLIB_FLAG)
		soundFlags = ADLIB_FLAG | SPEAKER_FLAG | MIDI_FLAG;
	else if (presentSound & INTERSOUND_FLAG)
		soundFlags = INTERSOUND_FLAG | SPEAKER_FLAG;
	else if (presentSound & SPEAKER_FLAG)
		soundFlags = SPEAKER_FLAG;
	else
		soundFlags = 0;
}

void init_soundVideo(int32 smallHeap, int16 flag) {
	if (videoMode != 0x13 && videoMode != 0)
		error("init_soundVideo: Video mode 0x%x is not supported!",
		    videoMode);

	//if ((flag & 4) == 0)
	//	vid_findVideo();

	mousePresent = 1;

	inVM = 0;

	presentSound = 0; // FIXME: sound is not supported yet

	sprAllocated = 0;
	timer_enableTimer();

	// snd_setResetTimerFlag(debugFlag); // TODO

	if (videoMode == 0x13)
		colorCount = 256;

	pPaletteDesc = &paletteStruct;
	pPaletteDesc->vgaPal = vgaPalette;
	pPaletteDesc->unused1 = unusedPalette1;
	pPaletteDesc->unused2 = unusedPalette2;
	pPrimarySurfDesc = &primarySurfDesc;

	if (videoMode != 0)
		vid_initSurfDesc(videoMode, 320, 200, PRIMARY_SURFACE);

	if (soundFlags & MIDI_FLAG) {
		soundFlags &= presentSound;
		if (presentSound & ADLIB_FLAG)
			soundFlags |= MIDI_FLAG;
	} else {
		soundFlags &= presentSound;
	}
}

void init_cleanup(void) {
	if (debugFlag == 0)
		timer_disableTimer();

	vid_freeDriver();
	if (curPrimaryDesc != 0) {
		vid_freeSurfDesc(curPrimaryDesc);
		vid_freeSurfDesc(allocatedPrimary);
		allocatedPrimary = 0;
		curPrimaryDesc = 0;
	}
	pPrimarySurfDesc = 0;
	if (snd_cleanupFunc != 0 && snd_playingSound != 0) {
		(*snd_cleanupFunc) (0);
		snd_cleanupFunc = 0;
	}
	snd_speakerOff();

	data_closeDataFile();

	if (sprAllocated != 0)
		error("init_cleanup: Error! Allocated sprites left: %d",
		    sprAllocated);

	snd_stopSound(0);
	keyboard_release();
	g_system->quit();
}

void init_initGame(char *totName) {
	int16 handle2;
	int16 i;
	int16 handle;
	char *infBuf;
	char *infPtr;
	char *infEnd;
	int16 j;
	char buffer[20];
	int32 varsCount;
/*
src		= byte ptr -2Eh
var_1A		= word ptr -1Ah
var_18		= word ptr -18h
var_16		= dword	ptr -16h
var_12		= word ptr -12h
var_10		= word ptr -10h
handle2		= word ptr -0Eh
fileHandle	= word ptr -0Ch
numFromTot	= word ptr -0Ah
memAvail	= dword	ptr -6
memBlocks	= word ptr -2*/

	disableVideoCfg = 0x11;
	disableMouseCfg = 0x15;
	init_soundVideo(1000, 1);

	handle2 = data_openData("intro.stk");
	if (handle2 >= 0) {
		data_closeData(handle2);
		data_openDataFile("intro.stk");
	}

	util_initInput();

	vid_setHandlers();
	vid_initPrimary(videoMode);
	mouseXShift = 1;
	mouseYShift = 1;

	game_totTextData = 0;
	game_totFileData = 0;
	game_totResourceTable = 0;
	inter_variables = 0;
	init_palDesc = (PalDesc *)malloc(12);

	if (videoMode != 0x13)
		error("init_initGame: Only 0x13 video mode is supported!");

	init_palDesc->vgaPal = draw_vgaPalette;
	init_palDesc->unused1 = draw_unusedPalette1;
	init_palDesc->unused2 = draw_unusedPalette2;
	vid_setFullPalette(init_palDesc);

	for (i = 0; i < 4; i++)
		draw_fonts[i] = 0;

	handle = data_openData("intro.inf");

	if (handle < 0) {
		for (i = 0; i < 4; i++) {
			handle2 = data_openData(init_fontNames[i]);
			if (handle2 >= 0) {
				data_closeData(handle2);
				draw_fonts[i] =
				    util_loadFont(init_fontNames[i]);
			}
		}
	} else {
		data_closeData(handle);

		infPtr = data_getData("intro.inf");
		infBuf = infPtr;

		infEnd = infBuf + data_getDataSize("intro.inf");

		for (i = 0; i < 4; i++, infPtr++) {
			for (j = 0; *infPtr >= ' ' && infPtr != infEnd;
			    j++, infPtr++)
				buffer[j] = *infPtr;

			buffer[j] = 0;
			strcat(buffer, ".let");
			handle2 = data_openData(buffer);
			if (handle2 >= 0) {
				data_closeData(handle2);
				draw_fonts[i] = util_loadFont(buffer);
			}

			if (infPtr == infEnd)
				break;

			infPtr++;
			if (infPtr == infEnd)
				break;
		}

		free(infBuf);
	}

	if (totName != 0) {
		strcpy(buffer, totName);
		strcat(buffer, ".tot");
	} else {
		strcpy(buffer, "intro.tot");
	}

	handle = data_openData(buffer);

	if (handle >= 0) {
		// Get variables count
		data_seekData(handle, 0x2c, SEEK_SET);
		data_readData(handle, (char *)&varsCount, 4);
		varsCount = FROM_LE_32(varsCount);
		data_closeData(handle);

		inter_variables = (char *)malloc(varsCount * 4);
		memset(inter_variables, 0, varsCount * 4);

		strcpy(game_curTotFile, buffer);

		cd_testCD(1, "GOB");
		cd_readLIC("gob.lic");
		game_start();

		cd_stopPlaying();
		cd_freeLICbuffer();

		free(inter_variables);
		free(game_totFileData);
		free(game_totTextData);
		free(game_totResourceTable);
	}

	for (i = 0; i < 4; i++) {
		if (draw_fonts[i] != 0)
			util_freeFont(draw_fonts[i]);
	}

	free(init_palDesc);
	data_closeDataFile();
	vid_initPrimary(-1);
	init_cleanup();
}

}				// End of namespace Gob
