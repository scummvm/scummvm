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
#include "gob/demos/scnplayer.h"
#include "gob/demos/batplayer.h"
#include "gob/sound/sound.h"

namespace Gob {

const char *Init::_fontNames[] = { "jeulet1.let", "jeulet2.let", "jeucar1.let", "jeumath.let" };

Init::Init(GobEngine *vm) : _vm(vm) {
	_palDesc = 0;
}

void Init::cleanup() {
	_vm->_video->freeDriver();
	_vm->_global->_primarySurfDesc.reset();

	_vm->_sound->speakerOff();
	_vm->_sound->blasterStop(0);
	_vm->_dataIO->closeDataFile();
}

void Init::doDemo() {
	if (_vm->isSCNDemo()) {
		// This is a non-interactive demo with a SCN script and VMD videos

		_vm->_video->setPrePalette();

		SCNPlayer scnPlayer(_vm);

		if (_vm->_demoIndex > 0)
			scnPlayer.play(_vm->_demoIndex - 1);
	}

	if (_vm->isBATDemo()) {
		// This is a non-interactive demo with a BAT script and videos

		BATPlayer batPlayer(_vm);

		if (_vm->_demoIndex > 0)
			batPlayer.play(_vm->_demoIndex - 1);
	}
}

void Init::initGame() {
	byte *infBuf;
	char *infPtr;
	char *infEnd;
	char buffer[128];

	initVideo();

	if (!_vm->isDemo()) {
		if (_vm->_dataIO->existData(_vm->_startStk.c_str()))
			_vm->_dataIO->openDataFile(_vm->_startStk.c_str());
	}

	_vm->_util->initInput();

	_vm->_video->initPrimary(_vm->_global->_videoMode);
	_vm->_global->_mouseXShift = 1;
	_vm->_global->_mouseYShift = 1;

	_vm->_game->_totTextData = 0;
	_palDesc = new Video::PalDesc;

	_vm->validateVideoMode(_vm->_global->_videoMode);

	_vm->_global->_setAllPalette = true;
	_palDesc->vgaPal = _vm->_draw->_vgaPalette;
	_palDesc->unused1 = _vm->_draw->_unusedPalette1;
	_palDesc->unused2 = _vm->_draw->_unusedPalette2;
	_vm->_video->setFullPalette(_palDesc);

	for (int i = 0; i < 8; i++)
		_vm->_draw->_fonts[i] = 0;

	if (_vm->isDemo()) {
		doDemo();
		delete _palDesc;
		_vm->_video->initPrimary(-1);
		cleanup();
		return;
	}

	if (!_vm->_dataIO->existData("intro.inf")) {

		for (int i = 0; i < 4; i++)
			if (_vm->_dataIO->existData(_fontNames[i]))
				_vm->_draw->_fonts[i] = _vm->_util->loadFont(_fontNames[i]);

	} else {
		infBuf = _vm->_dataIO->getData("intro.inf");
		infPtr = (char *) infBuf;

		infEnd = (char *) (infBuf + _vm->_dataIO->getDataSize("intro.inf"));

		for (int i = 0; i < 8; i++, infPtr++) {
			int j;

			for (j = 0; infPtr < infEnd && *infPtr >= ' '; j++, infPtr++)
				buffer[j] = *infPtr;
			buffer[j] = 0;

			strcat(buffer, ".let");
			if (_vm->_dataIO->existData(buffer))
				_vm->_draw->_fonts[i] = _vm->_util->loadFont(buffer);

			if ((infPtr + 1) >= infEnd)
				break;

			infPtr++;
		}
		delete[] infBuf;
	}

	if (_vm->_dataIO->existData(_vm->_startTot.c_str())) {
		DataStream *stream = _vm->_dataIO->getDataStream(_vm->_startTot.c_str());

		stream->seek(0x2C);
		_vm->_inter->allocateVars(stream->readUint16LE());

		delete stream;

		strcpy(_vm->_game->_curTotFile, _vm->_startTot.c_str());

		_vm->_sound->cdTest(1, "GOB");
		_vm->_sound->cdLoadLIC("gob.lic");

		// Search for a Coktel logo animation or image to display
		if (_vm->_dataIO->existData("coktel.imd")) {
			_vm->_draw->initScreen();
			_vm->_draw->_cursorIndex = -1;

			_vm->_util->longDelay(200); // Letting everything settle

			if (_vm->_vidPlayer->primaryOpen("coktel.imd")) {
				_vm->_vidPlayer->primaryPlay();
				_vm->_vidPlayer->primaryClose();
			}

			_vm->_draw->closeScreen();
		} else if (_vm->_dataIO->existData("coktel.clt")) {
			_vm->_draw->initScreen();
			_vm->_util->clearPalette();

			stream = _vm->_dataIO->getDataStream("coktel.clt");
			stream->read((byte *) _vm->_draw->_vgaPalette, 768);
			delete stream;

			if (_vm->_dataIO->existData("coktel.ims")) {
				byte *sprBuf;

				sprBuf = _vm->_dataIO->getData("coktel.ims");
				_vm->_video->drawPackedSprite(sprBuf, 320, 200, 0, 0, 0,
						*_vm->_draw->_frontSurface);
				_vm->_palAnim->fade(_palDesc, 0, 0);
				_vm->_util->delay(500);

				delete[] sprBuf;
			}
			_vm->_draw->closeScreen();
		}

		_vm->_game->start();

		_vm->_sound->cdStop();
		_vm->_sound->cdUnloadLIC();

	}

	delete _palDesc;
	_vm->_dataIO->closeDataFile();
	_vm->_video->initPrimary(-1);
	cleanup();
}

} // End of namespace Gob
