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
#include "common/stream.h"

#include "gob/gob.h"
#include "gob/game.h"
#include "gob/helper.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/script.h"
#include "gob/resources.h"
#include "gob/draw.h"
#include "gob/goblin.h"
#include "gob/inter.h"
#include "gob/mult.h"
#include "gob/scenery.h"
#include "gob/video.h"
#include "gob/videoplayer.h"
#include "gob/sound/sound.h"

namespace Gob {

Game_v2::Game_v2(GobEngine *vm) : Game_v1(vm) {
}

void Game_v2::playTot(int16 skipPlay) {
	char savedTotName[20];
	int16 *oldCaptureCounter;
	int16 *oldBreakFrom;
	int16 *oldNestLevel;
	int16 _captureCounter;
	int16 breakFrom;
	int16 nestLevel;

	oldNestLevel = _vm->_inter->_nestLevel;
	oldBreakFrom = _vm->_inter->_breakFromLevel;
	oldCaptureCounter = _vm->_scenery->_pCaptureCounter;

	_script->push();

	_vm->_inter->_nestLevel = &nestLevel;
	_vm->_inter->_breakFromLevel = &breakFrom;
	_vm->_scenery->_pCaptureCounter = &_captureCounter;
	strcpy(savedTotName, _curTotFile);

	if (skipPlay <= 0) {
		while (!_vm->shouldQuit()) {
			if (_vm->_inter->_variables)
				_vm->_draw->animateCursor(4);

			if (skipPlay != -1) {
				_vm->_inter->initControlVars(1);

				for (int i = 0; i < 4; i++) {
					_vm->_draw->_fontToSprite[i].sprite = -1;
					_vm->_draw->_fontToSprite[i].base = -1;
					_vm->_draw->_fontToSprite[i].width = -1;
					_vm->_draw->_fontToSprite[i].height = -1;
				}

				_vm->_mult->initAll();
				_vm->_mult->zeroMultData();

				_vm->_draw->_spritesArray[20] = _vm->_draw->_frontSurface;
				_vm->_draw->_spritesArray[21] = _vm->_draw->_backSurface;
				_vm->_draw->_cursorSpritesBack = _vm->_draw->_cursorSprites;
			} else
				_vm->_inter->initControlVars(0);

			_vm->_draw->_cursorHotspotXVar = -1;
			_totToLoad[0] = 0;

			if ((_curTotFile[0] == 0) && (!_script->isLoaded()))
				break;

			if (skipPlay == -2) {
				_vm->_vidPlayer->primaryClose();
				skipPlay = 0;
			}

			if (!_script->load(_curTotFile)) {
				_vm->_draw->blitCursor();
				_vm->_inter->_terminate = 2;
				break;
			}

			_resources->load(_curTotFile);

			_vm->_global->_inter_animDataSize = _script->getAnimDataSize();
			if (!_vm->_inter->_variables)
				_vm->_inter->allocateVars(_script->getVariablesCount() & 0xFFFF);

			_script->seek(_script->getFunctionOffset(TOTFile::kFunctionStart));

			_vm->_inter->renewTimeInVars();

			WRITE_VAR(13, _vm->_global->_useMouse);
			WRITE_VAR(14, _vm->_global->_soundFlags);
			WRITE_VAR(15, _vm->_global->_fakeVideoMode);
			WRITE_VAR(16, _vm->_global->_language);

			_vm->_inter->callSub(2);

			if (_totToLoad[0] != 0)
				_vm->_inter->_terminate = 0;

			_vm->_draw->blitInvalidated();

			_script->unload();

			_resources->unload();

			for (int i = 0; i < *_vm->_scenery->_pCaptureCounter; i++)
				capturePop(0);

			if (skipPlay != -1) {
				_vm->_goblin->freeObjects();

				_vm->_sound->blasterStop(0);

				for (int i = 0; i < Sound::kSoundsCount; i++) {
					SoundDesc *sound = _vm->_sound->sampleGetBySlot(i);

					if (sound &&
					   ((sound->getType() == SOUND_SND) || (sound->getType() == SOUND_WAV)))
						_vm->_sound->sampleFree(sound);
				}
			}

			if (_totToLoad[0] == 0)
				break;

			strcpy(_curTotFile, _totToLoad);
		}
	} else {
		_vm->_inter->initControlVars(0);
		_vm->_scenery->_pCaptureCounter = oldCaptureCounter;
		_script->seek(_script->getFunctionOffset(skipPlay + 1));

		_menuLevel++;
		_vm->_inter->callSub(2);
		_menuLevel--;

		if (_vm->_inter->_terminate != 0)
			_vm->_inter->_terminate = 2;
	}

	strcpy(_curTotFile, savedTotName);

	_vm->_inter->_nestLevel = oldNestLevel;
	_vm->_inter->_breakFromLevel = oldBreakFrom;
	_vm->_scenery->_pCaptureCounter = oldCaptureCounter;

	_script->pop();
}

} // End of namespace Gob
