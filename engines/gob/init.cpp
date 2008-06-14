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
 * $URL$
 * $Id$
 *
 */

#include "common/endian.h"

#include "gob/gob.h"
#include "gob/init.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/palanim.h"
#include "gob/inter.h"
#include "gob/video.h"
#include "gob/videoplayer.h"
#include "gob/sound/sound.h"

namespace Gob {

const char *Init::_fontNames[] = { "jeulet1.let", "jeulet2.let", "jeucar1.let", "jeumath.let" };

Init::Init(GobEngine *vm) : _vm(vm) {
	_palDesc = 0;
}

void Init::cleanup(void) {
	_vm->_video->freeDriver();
	_vm->_global->_primarySurfDesc = 0;

	_vm->_sound->speakerOff();
	_vm->_sound->blasterStop(0);
	_vm->_dataIO->closeDataFile();
}

void Init::initGame(const char *totName) {
	int16 handle2;
	int16 handle;
	int16 imdHandle;
	byte *infBuf;
	char *infPtr;
	char *infEnd;
	char buffer[128];

	initVideo();

	// The Lost In Time demo uses different file prefix
	if (_vm->getGameType() == kGameTypeLostInTime) {
		handle2 = _vm->_dataIO->openData("demo.stk");
		if (handle2 >= 0) {
			_vm->_dataIO->closeData(handle2);
			_vm->_dataIO->openDataFile("demo.stk");
		}
	}

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

		infBuf = _vm->_dataIO->getData("intro.inf");
		infPtr = (char *) infBuf;

		infEnd = (char *) (infBuf + _vm->_dataIO->getDataSize("intro.inf"));

		for (int i = 0; i < 8; i++, infPtr++) {
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
		DataStream *stream = _vm->_dataIO->openAsStream(handle, true);

		stream->seek(0x2C);
		_vm->_inter->allocateVars(stream->readUint16LE());

		delete stream;

		strcpy(_vm->_game->_curTotFile, buffer);

		_vm->_sound->cdTest(1, "GOB");
		_vm->_sound->cdLoadLIC("gob.lic");

		// Search for a Coktel logo animation or image to display
		imdHandle = _vm->_dataIO->openData("coktel.imd");
		if (imdHandle >= 0) {
			_vm->_dataIO->closeData(imdHandle);
			_vm->_draw->initScreen();
			_vm->_draw->_cursorIndex = -1;

			_vm->_util->longDelay(200); // Letting everything settle

			if (_vm->_vidPlayer->primaryOpen("coktel.imd")) {
				_vm->_vidPlayer->primaryPlay();
				_vm->_vidPlayer->primaryClose();
			}

			_vm->_draw->closeScreen();
		} else if ((imdHandle = _vm->_dataIO->openData("coktel.clt")) >= 0) {
			_vm->_draw->initScreen();

			stream = _vm->_dataIO->openAsStream(imdHandle, true);
			_vm->_util->clearPalette();
			stream->read((byte *) _vm->_draw->_vgaPalette, 768);
			delete stream;

			imdHandle = _vm->_dataIO->openData("coktel.ims");
			if (imdHandle >= 0) {
				byte *sprBuf;

				_vm->_dataIO->closeData(imdHandle);
				sprBuf = _vm->_dataIO->getData("coktel.ims");
				_vm->_video->drawPackedSprite(sprBuf, 320, 200, 0, 0, 0,
						_vm->_draw->_frontSurface);
				_vm->_palAnim->fade(_palDesc, 0, 0);
				_vm->_util->delay(500);

				delete[] sprBuf;
			}
			_vm->_draw->closeScreen();
		}

		_vm->_game->start();

		_vm->_sound->cdStop();
		_vm->_sound->cdUnloadLIC();

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
