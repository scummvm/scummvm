/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"

#include "gob/gob.h"
#include "gob/init.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/cdrom.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/sound.h"
#include "gob/video.h"
#include "gob/imd.h"

namespace Gob {

const char *Init::_fontNames[] = { "jeulet1.let", "jeulet2.let", "jeucar1.let", "jeumath.let" };

Init::Init(GobEngine *vm) : _vm(vm) {
	_palDesc = 0;
}

void Init::cleanup(void) {
	_vm->_video->freeDriver();
	_vm->_global->_primarySurfDesc = 0;

	_vm->_snd->speakerOff();
	_vm->_snd->stopSound(0);
	_vm->_dataIO->closeDataFile();
}

void Init::initGame(const char *totName) {
	int16 handle2;
	int16 handle;
	int16 imdHandle;
	char *infBuf;
	char *infPtr;
	char *infEnd;
	char buffer[20];
	int32 varsCount;

	initVideo();

	handle2 = _vm->_dataIO->openData("intro.stk");
	if (handle2 >= 0) {
		_vm->_dataIO->closeData(handle2);
		_vm->_dataIO->openDataFile("intro.stk");
	}

	_vm->_util->initInput();

	_vm->_video->initPrimary(_vm->_global->_videoMode);
	_vm->_global->_mouseXShift = 1;
	_vm->_global->_mouseYShift = 1;

	_vm->_game->_totTextData = 0;
	_vm->_game->_totFileData = 0;
	_vm->_game->_totResourceTable = 0;
	_vm->_global->_inter_variables = 0;
	_vm->_global->_inter_variablesSizes = 0;
	_palDesc = new Video::PalDesc;

	_vm->validateVideoMode(_vm->_global->_videoMode);

	_vm->_global->_setAllPalette = true;
	_palDesc->vgaPal = _vm->_draw->_vgaPalette;
	_palDesc->unused1 = _vm->_draw->_unusedPalette1;
	_palDesc->unused2 = _vm->_draw->_unusedPalette2;
	_vm->_video->setFullPalette(_palDesc);

	for (int i = 0; i < 8; i++)
		_vm->_draw->_fonts[i] = 0;

	handle = _vm->_dataIO->openData("intro.inf");

	if (handle < 0) {
		for (int i = 0; i < 4; i++) {
			handle2 = _vm->_dataIO->openData(_fontNames[i]);
			if (handle2 >= 0) {
				_vm->_dataIO->closeData(handle2);
				_vm->_draw->_fonts[i] = _vm->_util->loadFont(_fontNames[i]);
			}
		}
	} else {
		_vm->_dataIO->closeData(handle);

		infPtr = (char *) _vm->_dataIO->getData("intro.inf");
		infBuf = infPtr;

		infEnd = infBuf + _vm->_dataIO->getDataSize("intro.inf");

		for (int i = 0; i < 4; i++, infPtr++) {
			int j;

			for (j = 0; infPtr < infEnd && *infPtr >= ' '; j++, infPtr++)
				buffer[j] = *infPtr;
			buffer[j] = 0;

			strcat(buffer, ".let");
			handle2 = _vm->_dataIO->openData(buffer);
			if (handle2 >= 0) {
				_vm->_dataIO->closeData(handle2);
				_vm->_draw->_fonts[i] = _vm->_util->loadFont(buffer);
			}

			if ((infPtr + 1) >= infEnd)
				break;

			infPtr++;
		}
		delete[] infBuf;
	}

	if (totName) {
		strncpy0(buffer, totName, 15);
		strcat(buffer, ".tot");
	} else
		strncpy0(buffer, _vm->_startTot, 19);

	handle = _vm->_dataIO->openData(buffer);

	if (handle >= 0) {
		// Get variables count
		_vm->_dataIO->seekData(handle, 0x2C, SEEK_SET);
		varsCount = _vm->_dataIO->readUint16(handle);
		_vm->_dataIO->closeData(handle);

		_vm->_global->_inter_variables = new byte[varsCount * 4];
		_vm->_global->_inter_variablesSizes = new byte[varsCount * 4];
		_vm->_global->clearVars(varsCount);

		strcpy(_vm->_game->_curTotFile, buffer);

		_vm->_cdrom->testCD(1, "GOB");
		_vm->_cdrom->readLIC("gob.lic");

		_vm->_draw->_cursorIndex = -1;
		imdHandle = _vm->_dataIO->openData("coktel.imd");
		if (imdHandle >= 0) {
			_vm->_dataIO->closeData(imdHandle);
			_vm->_draw->initScreen();
			_vm->_util->longDelay(200); // Letting everything settle
			_vm->_imdPlayer->play("coktel", -1, -1, true);
			_vm->_draw->closeScreen();
		}

		_vm->_game->start();

		_vm->_cdrom->stopPlaying();
		_vm->_cdrom->freeLICbuffer();

		delete[] _vm->_global->_inter_variables;
		delete[] _vm->_global->_inter_variablesSizes;
		delete[] _vm->_game->_totFileData;
		if (_vm->_game->_totTextData) {
			if (_vm->_game->_totTextData->items)
				delete[] _vm->_game->_totTextData->items;
			delete _vm->_game->_totTextData;
		}
		if (_vm->_game->_totResourceTable) {
			delete[] _vm->_game->_totResourceTable->items;
			delete _vm->_game->_totResourceTable;
		}
	}

	for (int i = 0; i < 4; i++) {
		if (_vm->_draw->_fonts[i] != 0)
			_vm->_util->freeFont(_vm->_draw->_fonts[i]);
	}

	delete _palDesc;
	_vm->_dataIO->closeDataFile();
	_vm->_video->initPrimary(-1);
	cleanup();
}

} // End of namespace Gob
