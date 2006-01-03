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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

const char *Init::fontNames[] = { "jeulet1.let", "jeulet2.let", "jeucar1.let", "jeumath.let" };

Init::Init(GobEngine *vm) : _vm(vm) {
	palDesc = 0;
}

void Init::findBestCfg(void) {
	_vm->_global->videoMode = VIDMODE_VGA;
	_vm->_global->useMouse = _vm->_global->mousePresent;
	if (_vm->_global->presentSound & BLASTER_FLAG)
		_vm->_global->soundFlags = BLASTER_FLAG | SPEAKER_FLAG | MIDI_FLAG;
	else if (_vm->_global->presentSound & PROAUDIO_FLAG)
		_vm->_global->soundFlags = PROAUDIO_FLAG | SPEAKER_FLAG | MIDI_FLAG;
	else if (_vm->_global->presentSound & ADLIB_FLAG)
		_vm->_global->soundFlags = ADLIB_FLAG | SPEAKER_FLAG | MIDI_FLAG;
	else if (_vm->_global->presentSound & INTERSOUND_FLAG)
		_vm->_global->soundFlags = INTERSOUND_FLAG | SPEAKER_FLAG;
	else if (_vm->_global->presentSound & SPEAKER_FLAG)
		_vm->_global->soundFlags = SPEAKER_FLAG;
	else
		_vm->_global->soundFlags = 0;
}

void Init::soundVideo(int32 smallHeap, int16 flag) {
	if (_vm->_global->videoMode != 0x13 && _vm->_global->videoMode != 0)
		error("soundVideo: Video mode 0x%x is not supported!",
		    _vm->_global->videoMode);

	//if ((flag & 4) == 0)
	//	_vm->_video->findVideo();

	_vm->_global->mousePresent = 1;

	_vm->_global->inVM = 0;

	_vm->_global->presentSound = 0; // FIXME: sound is not supported yet

	_vm->_global->sprAllocated = 0;
	_vm->_gtimer->enableTimer();

	// _vm->_snd->setResetTimerFlag(debugFlag); // TODO

	if (_vm->_global->videoMode == 0x13)
		_vm->_global->colorCount = 256;

	_vm->_global->pPaletteDesc = &_vm->_global->paletteStruct;
	_vm->_global->pPaletteDesc->vgaPal = _vm->_global->vgaPalette;
	_vm->_global->pPaletteDesc->unused1 = _vm->_global->unusedPalette1;
	_vm->_global->pPaletteDesc->unused2 = _vm->_global->unusedPalette2;
	_vm->_global->pPrimarySurfDesc = &_vm->_global->primarySurfDesc;

	if (_vm->_global->videoMode != 0)
		_vm->_video->initSurfDesc(_vm->_global->videoMode, 320, 200, PRIMARY_SURFACE);

	if (_vm->_global->soundFlags & MIDI_FLAG) {
		_vm->_global->soundFlags &= _vm->_global->presentSound;
		if (_vm->_global->presentSound & ADLIB_FLAG)
			_vm->_global->soundFlags |= MIDI_FLAG;
	} else {
		_vm->_global->soundFlags &= _vm->_global->presentSound;
	}
}

void Init::cleanup(void) {
	if (_vm->_global->debugFlag == 0)
		_vm->_gtimer->disableTimer();

	_vm->_video->freeDriver();
	if (_vm->_global->curPrimaryDesc != 0) {
		_vm->_video->freeSurfDesc(_vm->_global->curPrimaryDesc);
		_vm->_video->freeSurfDesc(_vm->_global->allocatedPrimary);
		_vm->_global->allocatedPrimary = 0;
		_vm->_global->curPrimaryDesc = 0;
	}
	_vm->_global->pPrimarySurfDesc = 0;
	if (_vm->_snd->cleanupFunc != 0 && _vm->_snd->playingSound != 0) {
		(*_vm->_snd->cleanupFunc) (0);
		_vm->_snd->cleanupFunc = 0;
	}
	_vm->_snd->speakerOff();

	_vm->_dataio->closeDataFile();

	if (_vm->_global->sprAllocated != 0)
		error("cleanup: Error! Allocated sprites left: %d",
		    _vm->_global->sprAllocated);

	_vm->_snd->stopSound(0);
	_vm->_util->keyboard_release();
	g_system->quit();
}

void Init::initGame(char *totName) {
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

	_vm->_global->disableVideoCfg = 0x11;
	_vm->_global->disableMouseCfg = 0x15;
	soundVideo(1000, 1);

	handle2 = _vm->_dataio->openData("intro.stk");
	if (handle2 >= 0) {
		_vm->_dataio->closeData(handle2);
		_vm->_dataio->openDataFile("intro.stk");
	}

	_vm->_util->initInput();

	_vm->_video->setHandlers();
	_vm->_video->initPrimary(_vm->_global->videoMode);
	_vm->_global->mouseXShift = 1;
	_vm->_global->mouseYShift = 1;

	_vm->_game->totTextData = 0;
	_vm->_game->totFileData = 0;
	_vm->_game->totResourceTable = 0;
	_vm->_global->inter_variables = 0;
	palDesc = (Video::PalDesc *)malloc(12);

	if (_vm->_global->videoMode != 0x13)
		error("initGame: Only 0x13 video mode is supported!");

	palDesc->vgaPal = _vm->_draw->vgaPalette;
	palDesc->unused1 = _vm->_draw->unusedPalette1;
	palDesc->unused2 = _vm->_draw->unusedPalette2;
	_vm->_video->setFullPalette(palDesc);

	for (i = 0; i < 4; i++)
		_vm->_draw->fonts[i] = 0;

	handle = _vm->_dataio->openData("intro.inf");

	if (handle < 0) {
		for (i = 0; i < 4; i++) {
			handle2 = _vm->_dataio->openData(fontNames[i]);
			if (handle2 >= 0) {
				_vm->_dataio->closeData(handle2);
				_vm->_draw->fonts[i] =
				    _vm->_util->loadFont(fontNames[i]);
			}
		}
	} else {
		_vm->_dataio->closeData(handle);

		infPtr = _vm->_dataio->getData("intro.inf");
		infBuf = infPtr;

		infEnd = infBuf + _vm->_dataio->getDataSize("intro.inf");

		for (i = 0; i < 4; i++, infPtr++) {
			for (j = 0; *infPtr >= ' ' && infPtr != infEnd;
			    j++, infPtr++)
				buffer[j] = *infPtr;

			buffer[j] = 0;
			strcat(buffer, ".let");
			handle2 = _vm->_dataio->openData(buffer);
			if (handle2 >= 0) {
				_vm->_dataio->closeData(handle2);
				_vm->_draw->fonts[i] = _vm->_util->loadFont(buffer);
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

	handle = _vm->_dataio->openData(buffer);

	if (handle >= 0) {
		// Get variables count
		_vm->_dataio->seekData(handle, 0x2c, SEEK_SET);
		_vm->_dataio->readData(handle, (char *)&varsCount, 4);
		varsCount = FROM_LE_32(varsCount);
		_vm->_dataio->closeData(handle);

		_vm->_global->inter_variables = (char *)malloc(varsCount * 4);
		memset(_vm->_global->inter_variables, 0, varsCount * 4);

		strcpy(_vm->_game->curTotFile, buffer);

		_vm->_cdrom->testCD(1, "GOB");
		_vm->_cdrom->readLIC("gob.lic");
		_vm->_game->start();

		_vm->_cdrom->stopPlaying();
		_vm->_cdrom->freeLICbuffer();

		free(_vm->_global->inter_variables);
		free(_vm->_game->totFileData);
		free(_vm->_game->totTextData);
		free(_vm->_game->totResourceTable);
	}

	for (i = 0; i < 4; i++) {
		if (_vm->_draw->fonts[i] != 0)
			_vm->_util->freeFont(_vm->_draw->fonts[i]);
	}

	free(palDesc);
	_vm->_dataio->closeDataFile();
	_vm->_video->initPrimary(-1);
	cleanup();
}

}				// End of namespace Gob
