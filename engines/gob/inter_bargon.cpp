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
#include "gob/inter.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/palanim.h"
#include "gob/video.h"
#include "gob/videoplayer.h"
#include "gob/sound/sound.h"

namespace Gob {

#define OPCODEVER Inter_Bargon
#define OPCODEDRAW(i, x)  _opcodesDraw[i]._OPCODEDRAW(OPCODEVER, x)
#define OPCODEFUNC(i, x)  _opcodesFunc[i]._OPCODEFUNC(OPCODEVER, x)
#define OPCODEGOB(i, x)   _opcodesGob[i]._OPCODEGOB(OPCODEVER, x)

Inter_Bargon::Inter_Bargon(GobEngine *vm) : Inter_v2(vm) {
}

void Inter_Bargon::setupOpcodesDraw() {
	Inter_v2::setupOpcodesDraw();
}

void Inter_Bargon::setupOpcodesFunc() {
	Inter_v2::setupOpcodesFunc();
}

void Inter_Bargon::setupOpcodesGob() {
	OPCODEGOB( 1, oBargon_intro0);
	OPCODEGOB( 2, oBargon_intro1);
	OPCODEGOB( 3, oBargon_intro2);
	OPCODEGOB( 4, oBargon_intro3);

	OPCODEGOB( 5, oBargon_intro4);
	OPCODEGOB( 6, oBargon_intro5);
	OPCODEGOB( 7, oBargon_intro6);
	OPCODEGOB( 8, oBargon_intro7);

	OPCODEGOB( 9, oBargon_intro8);
	OPCODEGOB(10, oBargon_intro9);
	OPCODEGOB(11, o_gobNOP);
}

void Inter_Bargon::oBargon_intro0(OpGobParams &params) {
	if (_vm->_vidPlayer->primaryOpen("scaa", 0, 160)) {
		_vm->_vidPlayer->primaryPlay(0, 92, 27, 0, 0, 0);
		_vm->_vidPlayer->primaryClose();
	}
}

void Inter_Bargon::oBargon_intro1(OpGobParams &params) {
	if (_vm->_vidPlayer->primaryOpen("scaa", 0, 160)) {
		_vm->_vidPlayer->primaryPlay(0, -1, 27, 0, 0, 0, 0, 0, true, 23);
		_vm->_vidPlayer->primaryClose();
	}
}

void Inter_Bargon::oBargon_intro2(OpGobParams &params) {
	int i;
	int16 mouseX;
	int16 mouseY;
	int16 buttons;
	SurfaceDescPtr surface;
	SoundDesc samples[4];
	int16 comp[5] = { 0, 1, 2, 3, -1 };
	static const char *sndFiles[] = {"1INTROII.snd", "2INTROII.snd", "1INTRO3.snd", "2INTRO3.snd"};

	surface = _vm->_video->initSurfDesc(_vm->_global->_videoMode, 320, 200, 0);
	_vm->_video->drawPackedSprite("2ille.ims", *surface);
	_vm->_video->drawSprite(*surface, *_vm->_draw->_frontSurface, 0, 0, 319, 199, 0, 0, 0);
	_vm->_video->drawPackedSprite("2ille4.ims", *surface);
	_vm->_video->drawSprite(*surface, *_vm->_draw->_frontSurface, 0, 0, 319, 199, 320, 0, 0);
	_vm->_util->setScrollOffset(320, 0);
	_vm->_video->dirtyRectsAll();
	_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, -2, 0);
	_vm->_util->longDelay(1000);
	for (i = 320; i >= 0; i--) {
		_vm->_util->setScrollOffset(i, 0);
		_vm->_video->dirtyRectsAll();
		if ((_vm->_game->checkKeys(&mouseX, &mouseY, &buttons, 0) == 0x11B) ||
				_vm->shouldQuit()) {
			_vm->_palAnim->fade(0, -2, 0);
			_vm->_video->clearSurf(*_vm->_draw->_frontSurface);
			memset((char *) _vm->_draw->_vgaPalette, 0, 768);
			WRITE_VAR(4, buttons);
			WRITE_VAR(0, 0x11B);
			WRITE_VAR(57, (uint32) -1);
			break;
		}
	}
	if (!_vm->shouldQuit()) {
		_vm->_util->setScrollOffset(0, 0);
		_vm->_video->dirtyRectsAll();
	}
	surface.reset();
	if (VAR(57) == ((uint32) -1))
		return;

	for (i = 0; i < 4; i++)
		_vm->_sound->sampleLoad(&samples[i], SOUND_SND, sndFiles[i]);
	_vm->_sound->blasterPlayComposition(comp, 0, samples, 4);
	_vm->_sound->blasterWaitEndPlay(true, false);
	_vm->_palAnim->fade(0, 0, 0);
	_vm->_video->clearSurf(*_vm->_draw->_frontSurface);
}

void Inter_Bargon::oBargon_intro3(OpGobParams &params) {
	int16 mouseX;
	int16 mouseY;
	int16 buttons;
	Video::Color *palBak;
	SoundDesc samples[2];
	int16 comp[3] = { 0, 1, -1 };
	byte *palettes[4];
	static const char *sndFiles[] = {"1INTROIV.snd", "2INTROIV.snd"};
	static const char *palFiles[] = {"2ou2.clt", "2ou3.clt", "2ou4.clt", "2ou5.clt"};

	for (int i = 0; i < 2; i++)
		_vm->_sound->sampleLoad(&samples[i], SOUND_SND, sndFiles[i]);
	for (int i = 0; i < 4; i++)
		palettes[i] = _vm->_dataIO->getData(palFiles[i]);
	palBak = _vm->_global->_pPaletteDesc->vgaPal;

	_vm->_sound->blasterPlayComposition(comp, 0, samples, 2);
	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 4; j++) {
			_vm->_global->_pPaletteDesc->vgaPal = (Video::Color *) palettes[j];
			_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
			_vm->_util->longDelay(_vm->_util->getRandom(200));
		}
		if ((_vm->_game->checkKeys(&mouseX, &mouseY, &buttons, 0) == 0x11B) ||
				_vm->shouldQuit()) {
			_vm->_sound->blasterStop(10);
			_vm->_palAnim->fade(0, -2, 0);
			_vm->_video->clearSurf(*_vm->_draw->_frontSurface);
			memset(_vm->_draw->_vgaPalette, 0, 768);
			WRITE_VAR(4, buttons);
			WRITE_VAR(0, 0x11B);
			WRITE_VAR(57, (uint32) -1);
			break;
		}
	}
	_vm->_sound->blasterWaitEndPlay(false, false);

	_vm->_global->_pPaletteDesc->vgaPal = palBak;
	for (int i = 0; i < 4; i++)
		delete[] palettes[i];
}

void Inter_Bargon::oBargon_intro4(OpGobParams &params) {
	if (_vm->_vidPlayer->primaryOpen("scba", 191, 54)) {
		_vm->_vidPlayer->primaryPlay(0, -1, 27, 0, 0, 0, 0, 0, true);
		_vm->_vidPlayer->primaryClose();
	}
}

void Inter_Bargon::oBargon_intro5(OpGobParams &params) {
	if (_vm->_vidPlayer->primaryOpen("scbb", 191, 54)) {
		_vm->_vidPlayer->primaryPlay(0, -1, 27, 0, 0, 0);
		_vm->_vidPlayer->primaryClose();
	}
}

void Inter_Bargon::oBargon_intro6(OpGobParams &params) {
	if (_vm->_vidPlayer->primaryOpen("scbc", 191, 54)) {
		_vm->_vidPlayer->primaryPlay(0, -1, 27, 0, 0, 0);
		_vm->_vidPlayer->primaryClose();
	}
}

void Inter_Bargon::oBargon_intro7(OpGobParams &params) {
	if (_vm->_vidPlayer->primaryOpen("scbf", 191, 54)) {
		_vm->_vidPlayer->primaryPlay(0, -1, 27, 0, 0, 0);
		_vm->_vidPlayer->primaryClose();
	}
}

void Inter_Bargon::oBargon_intro8(OpGobParams &params) {
	if (_vm->_vidPlayer->primaryOpen("scbc", 191, 54)) {
		_vm->_vidPlayer->primaryPlay(0, -1, 27, 0, 0, 0);
		_vm->_vidPlayer->primaryClose();
	}
}

void Inter_Bargon::oBargon_intro9(OpGobParams &params) {
	if (_vm->_vidPlayer->primaryOpen("scbd", 191, 54)) {
		_vm->_vidPlayer->primaryPlay(0, -1, 27, 0, 0, 0);
		_vm->_vidPlayer->primaryClose();
	}
}

} // End of namespace Gob
