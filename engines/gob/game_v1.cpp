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
#include "gob/inter.h"
#include "gob/mult.h"
#include "gob/video.h"
#include "gob/scenery.h"
#include "gob/sound/sound.h"

namespace Gob {

Game_v1::Game_v1(GobEngine *vm) : Game(vm) {
}

void Game_v1::playTot(int16 skipPlay) {
	int16 _captureCounter;
	int16 breakFrom;
	int16 nestLevel;

	int16 *oldNestLevel = _vm->_inter->_nestLevel;
	int16 *oldBreakFrom = _vm->_inter->_breakFromLevel;
	int16 *oldCaptureCounter = _vm->_scenery->_pCaptureCounter;

	_script->push();

	_vm->_inter->_nestLevel = &nestLevel;
	_vm->_inter->_breakFromLevel = &breakFrom;
	_vm->_scenery->_pCaptureCounter = &_captureCounter;

	char savedTotName[20];
	strcpy(savedTotName, _curTotFile);

	if (skipPlay <= 0) {
		while (!_vm->shouldQuit()) {
			for (int i = 0; i < 4; i++) {
				_vm->_draw->_fontToSprite[i].sprite = -1;
				_vm->_draw->_fontToSprite[i].base = -1;
				_vm->_draw->_fontToSprite[i].width = -1;
				_vm->_draw->_fontToSprite[i].height = -1;
			}

			if ((_vm->getPlatform() == Common::kPlatformMacintosh)||
				  (_vm->getPlatform() == Common::kPlatformWindows))
				_vm->_sound->adlibStop();
			else
				_vm->_sound->cdStop();

			_vm->_draw->animateCursor(4);
			_vm->_inter->initControlVars(1);
			_vm->_mult->initAll();
			_vm->_mult->zeroMultData();

			for (int i = 0; i < SPRITES_COUNT; i++)
				_vm->_draw->freeSprite(i);

			_vm->_draw->_spritesArray[20] = _vm->_draw->_frontSurface;
			_vm->_draw->_spritesArray[21] = _vm->_draw->_backSurface;
			_vm->_draw->_spritesArray[23] = _vm->_draw->_cursorSprites;

			for (int i = 0; i < 20; i++)
				freeSoundSlot(i);

			_totToLoad[0] = 0;

			if ((_curTotFile[0] == 0) && !_script->isLoaded())
				break;

			if (!_script->load(_curTotFile)) {
				_vm->_draw->blitCursor();
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
			WRITE_VAR(15, _vm->_global->_videoMode);
			WRITE_VAR(16, _vm->_global->_language);

			_vm->_inter->callSub(2);
			_script->setFinished(false);

			if (_totToLoad[0] != 0)
				_vm->_inter->_terminate = 0;

			_vm->_draw->blitInvalidated();

			_script->unload();

			_resources->unload();

			for (int i = 0; i < *_vm->_scenery->_pCaptureCounter; i++)
				capturePop(0);

			_vm->_mult->checkFreeMult();
			_vm->_mult->freeAll();

			for (int i = 0; i < SPRITES_COUNT; i++)
				_vm->_draw->freeSprite(i);
			_vm->_sound->blasterStop(0);

			for (int i = 0; i < 20; i++)
				freeSoundSlot(i);

			if (_totToLoad[0] == 0)
				break;

			strcpy(_curTotFile, _totToLoad);
		}
	}

	strcpy(_curTotFile, savedTotName);

	_vm->_inter->_nestLevel = oldNestLevel;
	_vm->_inter->_breakFromLevel = oldBreakFrom;
	_vm->_scenery->_pCaptureCounter = oldCaptureCounter;

	_script->pop();
}

void Game_v1::prepareStart(void) {
	_vm->_global->_pPaletteDesc->unused2 = _vm->_draw->_unusedPalette2;
	_vm->_global->_pPaletteDesc->unused1 = _vm->_draw->_unusedPalette1;
	_vm->_global->_pPaletteDesc->vgaPal = _vm->_draw->_vgaPalette;

	_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);

	_vm->_draw->initScreen();
	_vm->_video->fillRect(*_vm->_draw->_backSurface, 0, 0, 319, 199, 1);
	_vm->_draw->_frontSurface = _vm->_global->_primarySurfDesc;
	_vm->_video->fillRect(*_vm->_draw->_frontSurface, 0, 0, 319, 199, 1);

	_vm->_util->setMousePos(152, 92);
	_vm->_draw->_cursorX = _vm->_global->_inter_mouseX = 152;
	_vm->_draw->_cursorY = _vm->_global->_inter_mouseY = 92;

	_vm->_draw->_invalidatedCount = 0;
	_vm->_draw->_noInvalidated = true;
	_vm->_draw->_applyPal = false;
	_vm->_draw->_paletteCleared = false;
	_vm->_draw->_cursorWidth = 16;
	_vm->_draw->_cursorHeight = 16;
	_vm->_draw->_transparentCursor = 1;

	for (int i = 0; i < 40; i++) {
		_vm->_draw->_cursorAnimLow[i] = -1;
		_vm->_draw->_cursorAnimDelays[i] = 0;
		_vm->_draw->_cursorAnimHigh[i] = 0;
	}

	_vm->_draw->_cursorAnimLow[1] = 0;
	_vm->_draw->_cursorSprites =
		_vm->_video->initSurfDesc(_vm->_global->_videoMode, 32, 16, 2);
	_vm->_draw->_scummvmCursor =
		_vm->_video->initSurfDesc(_vm->_global->_videoMode, 16, 16, SCUMMVM_CURSOR);
	_vm->_draw->_renderFlags = 0;
	_vm->_draw->_backDeltaX = 0;
	_vm->_draw->_backDeltaY = 0;

	_startTimeKey = _vm->_util->getTimeKey();
}

} // End of namespace Gob
