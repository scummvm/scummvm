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
#include "common/str.h"

#include "gob/gob.h"
#include "gob/game.h"
#include "gob/global.h"
#include "gob/dataio.h"
#include "gob/variables.h"
#include "gob/script.h"
#include "gob/resources.h"
#include "gob/hotspots.h"
#include "gob/inter.h"
#include "gob/draw.h"
#include "gob/mult.h"
#include "gob/scenery.h"
#include "gob/videoplayer.h"
#include "gob/sound/sound.h"

namespace Gob {

Environments::Environments(GobEngine *vm) : _vm(vm) {
	for (uint i = 0; i < kEnvironmentCount; i++) {
		Environment &e = _environments[i];
		Media       &m = _media[i];

		e.cursorHotspotX = 0;
		e.cursorHotspotY = 0;
		e.variables      = 0;
		e.script         = 0;
		e.resources      = 0;
		e.curTotFile[0]  = '\0';

		for (int j = 0; j < 17; j++)
			m.fonts[j] = 0;
	}
}

Environments::~Environments() {
	clear();
}

void Environments::clear() {
	// Deleting unique variables, script and resources

	for (uint i = 0; i < kEnvironmentCount; i++) {
		if (_environments[i].variables == _vm->_inter->_variables)
			continue;

		if (!has(_environments[i].variables, i + 1))
			delete _environments[i].variables;
	}

	for (uint i = 0; i < kEnvironmentCount; i++) {
		if (_environments[i].script == _vm->_game->_script)
			continue;

		if (!has(_environments[i].script, i + 1))
			delete _environments[i].script;
	}

	for (uint i = 0; i < kEnvironmentCount; i++) {
		if (_environments[i].resources == _vm->_game->_resources)
			continue;

		if (!has(_environments[i].resources, i + 1))
			delete _environments[i].resources;
	}

	for (uint i = 0; i < kEnvironmentCount; i++)
		clearMedia(i);
}

void Environments::set(uint8 env) {
	if (env >= kEnvironmentCount)
		return;

	Environment &e = _environments[env];

	// If it already has a unique script or resource assigned, delete them
	if ((e.script != _vm->_game->_script) && !has(e.script, 0, env))
		delete e.script;
	if ((e.resources != _vm->_game->_resources) && !has(e.resources, 0, env))
		delete e.resources;

	e.cursorHotspotX = _vm->_draw->_cursorHotspotXVar;
	e.cursorHotspotY = _vm->_draw->_cursorHotspotYVar;
	e.script         = _vm->_game->_script;
	e.resources      = _vm->_game->_resources;
	e.variables      = _vm->_inter->_variables;
	strncpy(e.curTotFile, _vm->_game->_curTotFile, 14);
}

void Environments::get(uint8 env) const {
	if (env >= kEnvironmentCount)
		return;

	const Environment &e = _environments[env];

	_vm->_draw->_cursorHotspotXVar = e.cursorHotspotX;
	_vm->_draw->_cursorHotspotYVar = e.cursorHotspotY;
	_vm->_game->_script            = e.script;
	_vm->_game->_resources         = e.resources;
	_vm->_inter->_variables        = e.variables;
	strncpy(_vm->_game->_curTotFile, e.curTotFile, 14);
}

const char *Environments::getTotFile(uint8 env) const {
	if (env >= kEnvironmentCount)
		return "";

	return _environments[env].curTotFile;
}

bool Environments::has(Variables *variables, uint8 startEnv, int16 except) const {
	for (uint i = startEnv; i < kEnvironmentCount; i++) {
		if ((except >= 0) && (((uint16) except) == i))
			continue;

		if (_environments[i].variables == variables)
			return true;
	}

	return false;
}

bool Environments::has(Script *script, uint8 startEnv, int16 except) const {
	for (uint i = startEnv; i < kEnvironmentCount; i++) {
		if ((except >= 0) && (((uint16) except) == i))
			continue;

		if (_environments[i].script == script)
			return true;
	}

	return false;
}

bool Environments::has(Resources *resources, uint8 startEnv, int16 except) const {
	for (uint i = startEnv; i < kEnvironmentCount; i++) {
		if ((except >= 0) && (((uint16) except) == i))
			continue;

		if (_environments[i].resources == resources)
			return true;
	}

	return false;
}

bool Environments::clearMedia(uint8 env) {
	if (env >= kEnvironmentCount)
		return false;

	Media &m = _media[env];

	for (int i = 0; i < 10; i++)
		m.sprites[i].reset();

	for (int i = 0; i < 10; i++)
		m.sounds[i].free();

	for (int i = 0; i < 17; i++) {
		delete m.fonts[i];
		m.fonts[i] = 0;
	}

	return true;
}

bool Environments::setMedia(uint8 env) {
	if (env >= kEnvironmentCount)
		return false;

	clearMedia(env);

	Media &m = _media[env];

	for (int i = 0; i < 10; i++) {
		m.sprites[i] = _vm->_draw->_spritesArray[i];
		_vm->_draw->_spritesArray[i].reset();
	}

	for (int i = 0; i < 10; i++) {
		SoundDesc *sound = _vm->_sound->sampleGetBySlot(i);
		if (sound)
			m.sounds[i].swap(*sound);
	}

	int n = MIN(Draw::kFontCount, 17);
	for (int i = 0; i < n; i++) {
		m.fonts[i] = _vm->_draw->_fonts[i];
		_vm->_draw->_fonts[i] = 0;
	}

	return true;
}

bool Environments::getMedia(uint8 env) {
	if (env >= kEnvironmentCount)
		return false;

	Media &m = _media[env];

	for (int i = 0; i < 10; i++) {
		_vm->_draw->_spritesArray[i] = m.sprites[i];
		m.sprites[i].reset();
	}

	for (int i = 0; i < 10; i++) {
		SoundDesc *sound = _vm->_sound->sampleGetBySlot(i);
		if (sound)
			m.sounds[i].swap(*sound);
		m.sounds[i].free();
	}

	int n = MIN(Draw::kFontCount, 17);
	for (int i = 0; i < n; i++) {
		delete _vm->_draw->_fonts[i];
		_vm->_draw->_fonts[i] = m.fonts[i];
		m.fonts[i]= 0;
	}

	return true;
}


Game::Game(GobEngine *vm) : _vm(vm) {
	_captureCount = 0;

	_curTotFile[0] = 0;
	_totToLoad[0] = 0;

	_startTimeKey = 0;
	_mouseButtons = kMouseButtonsNone;

	_handleMouse = 0;
	_forceHandleMouse = 0;
	_noScroll = true;
	_preventScroll = false;

	_wantScroll  = false;
	_wantScrollX = 0;
	_wantScrollY = 0;

	_tempStr[0] = 0;

	_numEnvironments = 0;
	_curEnvironment = 0;

	_environments = new Environments(_vm);
	_script       = new Script(_vm);
	_resources    = new Resources(_vm);
	_hotspots     = new Hotspots(_vm);
}

Game::~Game() {
	delete _environments;
	delete _script;
	delete _resources;
	delete _hotspots;
}

void Game::prepareStart() {
	_vm->_global->_pPaletteDesc->unused2 = _vm->_draw->_unusedPalette2;
	_vm->_global->_pPaletteDesc->unused1 = _vm->_draw->_unusedPalette1;
	_vm->_global->_pPaletteDesc->vgaPal = _vm->_draw->_vgaPalette;

	_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);

	_vm->_draw->initScreen();
	_vm->_draw->_frontSurface->fillRect(0, 0,
	_vm->_video->_surfWidth - 1, _vm->_video->_surfHeight - 1, 1);

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

	_vm->_draw->_renderFlags = 0;
	_vm->_draw->_backDeltaX = 0;
	_vm->_draw->_backDeltaY = 0;

	_startTimeKey = _vm->_util->getTimeKey();
}

void Game::playTot(int16 skipPlay) {
	char savedTotName[20];
	int16 *oldCaptureCounter;
	int16 *oldBreakFrom;
	int16 *oldNestLevel;
	int16 captureCounter = 0;
	int16 breakFrom;
	int16 nestLevel;

	oldNestLevel = _vm->_inter->_nestLevel;
	oldBreakFrom = _vm->_inter->_breakFromLevel;
	oldCaptureCounter = _vm->_scenery->_pCaptureCounter;

	_script->push();

	_vm->_inter->_nestLevel = &nestLevel;
	_vm->_inter->_breakFromLevel = &breakFrom;
	_vm->_scenery->_pCaptureCounter = &captureCounter;
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

				// Gobliiins music stopping
				if (_vm->getGameType() == kGameTypeGob1) {
					_vm->_sound->adlibStop();
					_vm->_sound->cdStop();
				}

				_vm->_mult->initAll();
				_vm->_mult->zeroMultData();

				_vm->_draw->_spritesArray[Draw::kFrontSurface] = _vm->_draw->_frontSurface;
				_vm->_draw->_spritesArray[Draw::kBackSurface ] = _vm->_draw->_backSurface;
				_vm->_draw->_cursorSpritesBack = _vm->_draw->_cursorSprites;
			} else
				_vm->_inter->initControlVars(0);

			_vm->_draw->_cursorHotspotXVar = -1;
			_totToLoad[0] = 0;

			if ((_curTotFile[0] == 0) && (!_script->isLoaded()))
				break;

			if (skipPlay == -2) {
				_vm->_vidPlayer->closeVideo();
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

			if (_vm->_inter->_variables) {
				WRITE_VAR(13, _vm->_global->_useMouse);
				WRITE_VAR(14, _vm->_global->_soundFlags);
				WRITE_VAR(15, _vm->_global->_fakeVideoMode);
				WRITE_VAR(16, _vm->_global->_language);

				// WORKAROUND: Inca2 seems to depend on that variable to be cleared
				if (_vm->getGameType() == kGameTypeInca2)
					WRITE_VAR(59, 0);
			}

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

			_vm->_draw->closeAllWin();

			if (_totToLoad[0] == 0)
				break;

			Common::strlcpy(_curTotFile, _totToLoad, 14);

		}
	} else {
		_vm->_inter->initControlVars(0);
		_vm->_scenery->_pCaptureCounter = oldCaptureCounter;

		if (skipPlay > 13) {
			warning("Addy: skipPlay = %d", skipPlay);
			_script->skip(skipPlay);
		} else
			_script->seek(_script->getFunctionOffset(skipPlay + 1));

		_vm->_inter->callSub(2);

		if (_vm->_inter->_terminate != 0)
			_vm->_inter->_terminate = 2;
	}

	Common::strlcpy(_curTotFile, savedTotName, 14);

	_vm->_inter->_nestLevel = oldNestLevel;
	_vm->_inter->_breakFromLevel = oldBreakFrom;
	_vm->_scenery->_pCaptureCounter = oldCaptureCounter;

	_script->pop();
}

void Game::capturePush(int16 left, int16 top, int16 width, int16 height) {
	int16 right;

	if (_captureCount == 20)
		error("Game::capturePush(): Capture stack overflow");

	_captureStack[_captureCount].left = left;
	_captureStack[_captureCount].top = top;
	_captureStack[_captureCount].right = left + width;
	_captureStack[_captureCount].bottom = top + height;

	_vm->_draw->_spriteTop = top;
	_vm->_draw->_spriteBottom = height;

	right = left + width - 1;
	left &= 0xFFF0;
	right |= 0xF;

	_vm->_draw->initSpriteSurf(Draw::kCaptureSurface + _captureCount, right - left + 1, height, 0);

	_vm->_draw->_sourceSurface = Draw::kBackSurface;
	_vm->_draw->_destSurface = Draw::kCaptureSurface + _captureCount;

	_vm->_draw->_spriteLeft = left;
	_vm->_draw->_spriteRight = right - left + 1;
	_vm->_draw->_destSpriteX = 0;
	_vm->_draw->_destSpriteY = 0;
	_vm->_draw->_transparency = 0;
	_vm->_draw->spriteOperation(0);
	_captureCount++;
}

void Game::capturePop(char doDraw) {
	if (_captureCount <= 0)
		return;

	_captureCount--;
	if (doDraw) {
		_vm->_draw->_destSpriteX = _captureStack[_captureCount].left;
		_vm->_draw->_destSpriteY = _captureStack[_captureCount].top;
		_vm->_draw->_spriteRight =
		    _captureStack[_captureCount].width();
		_vm->_draw->_spriteBottom =
		    _captureStack[_captureCount].height();

		_vm->_draw->_transparency = 0;
		_vm->_draw->_sourceSurface = Draw::kCaptureSurface + _captureCount;
		_vm->_draw->_destSurface = Draw::kBackSurface;
		_vm->_draw->_spriteLeft = _vm->_draw->_destSpriteX & 0xF;
		_vm->_draw->_spriteTop = 0;
		_vm->_draw->spriteOperation(0);
	}
	_vm->_draw->freeSprite(Draw::kCaptureSurface + _captureCount);
}

void Game::freeSoundSlot(int16 slot) {
	if (slot == -1)
		slot = _vm->_game->_script->readValExpr();

	_vm->_sound->sampleFree(_vm->_sound->sampleGetBySlot(slot));
}

void Game::wantScroll(int16 x, int16 y) {
	_wantScroll  = true;
	_wantScrollX = x;
	_wantScrollY = y;
}

void Game::evaluateScroll() {
	if (_noScroll || _preventScroll || !_wantScroll)
		return;

	if ((_vm->_global->_videoMode != 0x14) && (_vm->_global->_videoMode != 0x18))
		return;

	if ((_wantScrollX == 0) && (_vm->_draw->_scrollOffsetX > 0)) {
		uint16 off;

		off = MIN(_vm->_draw->_cursorWidth, _vm->_draw->_scrollOffsetX);
		off = MAX(off / 2, 1);
		_vm->_draw->_scrollOffsetX -= off;
		_vm->_video->dirtyRectsAll();
	} else if ((_wantScrollY == 0) && (_vm->_draw->_scrollOffsetY > 0)) {
		uint16 off;

		off = MIN(_vm->_draw->_cursorHeight, _vm->_draw->_scrollOffsetY);
		off = MAX(off / 2, 1);
		_vm->_draw->_scrollOffsetY -= off;
		_vm->_video->dirtyRectsAll();
	}

	int16 cursorRight  = _wantScrollX + _vm->_draw->_cursorWidth;
	int16 screenRight  = _vm->_draw->_scrollOffsetX + _vm->_width;
	int16 cursorBottom = _wantScrollY + _vm->_draw->_cursorHeight;
	int16 screenBottom = _vm->_draw->_scrollOffsetY + _vm->_height;

	if ((cursorRight >= _vm->_width) &&
			(screenRight < _vm->_video->_surfWidth)) {
		uint16 off;

		off = MIN(_vm->_draw->_cursorWidth,
				(int16) (_vm->_video->_surfWidth - screenRight));
		off = MAX(off / 2, 1);

		_vm->_draw->_scrollOffsetX += off;
		_vm->_video->dirtyRectsAll();

		_vm->_util->setMousePos(_vm->_width - _vm->_draw->_cursorWidth, _wantScrollY);
	} else if ((cursorBottom >= (_vm->_height - _vm->_video->_splitHeight2)) &&
			(screenBottom < _vm->_video->_surfHeight)) {
		uint16 off;

		off = MIN(_vm->_draw->_cursorHeight,
				(int16) (_vm->_video->_surfHeight - screenBottom));
		off = MAX(off / 2, 1);

		_vm->_draw->_scrollOffsetY += off;
		_vm->_video->dirtyRectsAll();

		_vm->_util->setMousePos(_wantScrollX,
				_vm->_height - _vm->_video->_splitHeight2 - _vm->_draw->_cursorHeight);
	}

	_vm->_util->setScrollOffset();

	_wantScroll = false;
}

int16 Game::checkKeys(int16 *pMouseX, int16 *pMouseY,
		MouseButtons *pButtons, char handleMouse) {

	_vm->_util->processInput(true);

	if (_vm->_mult->_multData && _vm->_inter->_variables &&
			(VAR(58) != 0)) {
		if (_vm->_mult->_multData->frameStart != (int) VAR(58) - 1)
			_vm->_mult->_multData->frameStart++;
		else
			_vm->_mult->_multData->frameStart = 0;

		_vm->_mult->playMult(_vm->_mult->_multData->frameStart + VAR(57),
				_vm->_mult->_multData->frameStart + VAR(57), 1, handleMouse);
	}

	if ((_vm->_inter->_soundEndTimeKey != 0) &&
	    (_vm->_util->getTimeKey() >= _vm->_inter->_soundEndTimeKey)) {
		_vm->_sound->blasterStop(_vm->_inter->_soundStopVal);
		_vm->_inter->_soundEndTimeKey = 0;
	}

	if (pMouseX && pMouseY && pButtons) {
		_vm->_util->getMouseState(pMouseX, pMouseY, pButtons);

		if (*pButtons == kMouseButtonsBoth)
			*pButtons = kMouseButtonsNone;
	}

	return _vm->_util->checkKey();
}

void Game::start() {
	prepareStart();
	playTot(-2);

	_vm->_draw->closeScreen();

	for (int i = 0; i < SPRITES_COUNT; i++)
		_vm->_draw->freeSprite(i);
	_vm->_draw->_scummvmCursor.reset();
}

// flagbits: 0 = freeInterVariables, 1 = skipPlay
void Game::totSub(int8 flags, const char *newTotFile) {
	int8 curBackupPos;

	if ((flags == 16) || (flags == 17)) {
		// Prefetch tot data + delete prefetched data
		return;
	}

	if (_numEnvironments >= Environments::kEnvironmentCount)
		error("Game::totSub(): Environments overflow");

	_environments->set(_numEnvironments);

	if (flags == 18) {
		warning("Backuping media to %d", _numEnvironments);
		_environments->setMedia(_numEnvironments);
	}

	curBackupPos = _curEnvironment;
	_numEnvironments++;
	_curEnvironment = _numEnvironments;

	_script = new Script(_vm);
	_resources = new Resources(_vm);

	if (flags & 0x80)
		warning("Addy Stub: Game::totSub(), flags & 0x80");

	if (flags & 5)
		_vm->_inter->_variables = 0;

	Common::strlcpy(_curTotFile, newTotFile, 10);
	strcat(_curTotFile, ".TOT");

	if (_vm->_inter->_terminate != 0) {
		clearUnusedEnvironment();
		return;
	}

	if (!(flags & 0x20))
		_hotspots->push(0, true);

	if ((flags == 18) || (flags & 0x06))
		playTot(-1);
	else
		playTot(0);

	if (_vm->_inter->_terminate != 2)
		_vm->_inter->_terminate = 0;

	if (!(flags & 0x20)) {
		_hotspots->clear();
		_hotspots->pop();
	}

	if ((flags & 5) && _vm->_inter->_variables)
		_vm->_inter->delocateVars();

	clearUnusedEnvironment();

	_numEnvironments--;
	_curEnvironment = curBackupPos;
	_environments->get(_numEnvironments);

	if (flags == 18) {
		warning("Restoring media from %d", _numEnvironments);
		_environments->getMedia(_numEnvironments);
	}

	_vm->_global->_inter_animDataSize = _script->getAnimDataSize();
}

void Game::switchTotSub(int16 index, int16 skipPlay) {
	int16 backupedCount;
	int16 curBackupPos;

	if ((_numEnvironments - index) < 1)
		return;

	int16 newPos = _curEnvironment - index - ((index >= 0) ? 1 : 0);
	if (newPos >= Environments::kEnvironmentCount)
		return;

	// WORKAROUND: Some versions don't make the MOVEMENT menu item unselectable
	// in the dreamland screen, resulting in a crash when it's clicked.
	if ((_vm->getGameType() == kGameTypeGob2) && (index == -1) && (skipPlay == 7) &&
	    !scumm_stricmp(_environments->getTotFile(newPos), "gob06.tot"))
		return;

	curBackupPos = _curEnvironment;
	backupedCount = _numEnvironments;
	if (_curEnvironment == _numEnvironments)
		_environments->set(_numEnvironments++);

	_curEnvironment -= index;
	if (index >= 0)
		_curEnvironment--;

	clearUnusedEnvironment();

	_environments->get(_curEnvironment);

	if (_vm->_inter->_terminate != 0) {
		clearUnusedEnvironment();
		return;
	}

	_hotspots->push(0, true);
	playTot(skipPlay);

	if (_vm->_inter->_terminate != 2)
		_vm->_inter->_terminate = 0;

	_hotspots->pop();

	clearUnusedEnvironment();

	_curEnvironment = curBackupPos;
	_numEnvironments = backupedCount;
	_environments->get(_curEnvironment);
}

void Game::clearUnusedEnvironment() {
	if (!_environments->has(_script)) {
		delete _script;
		_script = 0;
	}
	if (!_environments->has(_resources)) {
		delete _resources;
		_resources = 0;
	}
}

} // End of namespace Gob
