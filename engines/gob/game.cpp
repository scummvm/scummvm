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
#include "gob/game.h"
#include "gob/helper.h"
#include "gob/global.h"
#include "gob/dataio.h"
#include "gob/variables.h"
#include "gob/script.h"
#include "gob/resources.h"
#include "gob/hotspots.h"
#include "gob/inter.h"
#include "gob/draw.h"
#include "gob/mult.h"
#include "gob/videoplayer.h"
#include "gob/sound/sound.h"

namespace Gob {

Environments::Environments(GobEngine *vm) : _vm(vm) {
	_environments = new Environment[kEnvironmentCount];

	for (uint i = 0; i < kEnvironmentCount; i++) {
		Environment &e = _environments[i];

		e.cursorHotspotX = 0;
		e.cursorHotspotY = 0;
		e.variables      = 0;
		e.script         = 0;
		e.resources      = 0;
		e.curTotFile[0]  = '\0';
	}
}

Environments::~Environments() {
	clear();

	delete[] _environments;
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


Game::Game(GobEngine *vm) : _vm(vm) {
	_captureCount = 0;

	_curTotFile[0] = 0;
	_totToLoad[0] = 0;

	_startTimeKey = 0;
	_mouseButtons = kMouseButtonsNone;

	_handleMouse = 0;
	_forceHandleMouse = 0;
	_menuLevel = 0;
	_noScroll = true;
	_preventScroll = false;
	_scrollHandleMouse = false;

	_noCd = false;

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
	_vm->_video->fillRect(*_vm->_draw->_frontSurface, 0, 0,
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

	_vm->_draw->initSpriteSurf(30 + _captureCount, right - left + 1, height, 0);

	_vm->_draw->_sourceSurface = 21;
	_vm->_draw->_destSurface = 30 + _captureCount;

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
		_vm->_draw->_sourceSurface = 30 + _captureCount;
		_vm->_draw->_destSurface = 21;
		_vm->_draw->_spriteLeft = _vm->_draw->_destSpriteX & 0xF;
		_vm->_draw->_spriteTop = 0;
		_vm->_draw->spriteOperation(0);
	}
	_vm->_draw->freeSprite(30 + _captureCount);
}

void Game::freeSoundSlot(int16 slot) {
	if (slot == -1)
		slot = _vm->_game->_script->readValExpr();

	_vm->_sound->sampleFree(_vm->_sound->sampleGetBySlot(slot));
}

void Game::evaluateScroll(int16 x, int16 y) {
	if (_preventScroll || !_scrollHandleMouse || (_menuLevel > 0))
		return;

	if (_noScroll ||
	   ((_vm->_global->_videoMode != 0x14) && (_vm->_global->_videoMode != 0x18)))
		return;

	if ((x == 0) && (_vm->_draw->_scrollOffsetX > 0)) {
		uint16 off;

		off = MIN(_vm->_draw->_cursorWidth, _vm->_draw->_scrollOffsetX);
		off = MAX(off / 2, 1);
		_vm->_draw->_scrollOffsetX -= off;
		_vm->_video->dirtyRectsAll();
	} else if ((y == 0) && (_vm->_draw->_scrollOffsetY > 0)) {
		uint16 off;

		off = MIN(_vm->_draw->_cursorHeight, _vm->_draw->_scrollOffsetY);
		off = MAX(off / 2, 1);
		_vm->_draw->_scrollOffsetY -= off;
		_vm->_video->dirtyRectsAll();
	}

	int16 cursorRight = x + _vm->_draw->_cursorWidth;
	int16 screenRight = _vm->_draw->_scrollOffsetX + _vm->_width;
	int16 cursorBottom = y + _vm->_draw->_cursorHeight;
	int16 screenBottom = _vm->_draw->_scrollOffsetY + _vm->_height;

	if ((cursorRight >= _vm->_width) &&
			(screenRight < _vm->_video->_surfWidth)) {
		uint16 off;

		off = MIN(_vm->_draw->_cursorWidth,
				(int16) (_vm->_video->_surfWidth - screenRight));
		off = MAX(off / 2, 1);

		_vm->_draw->_scrollOffsetX += off;
		_vm->_video->dirtyRectsAll();

		_vm->_util->setMousePos(_vm->_width - _vm->_draw->_cursorWidth, y);
	} else if ((cursorBottom >= (_vm->_height - _vm->_video->_splitHeight2)) &&
			(screenBottom < _vm->_video->_surfHeight)) {
		uint16 off;

		off = MIN(_vm->_draw->_cursorHeight,
				(int16) (_vm->_video->_surfHeight - screenBottom));
		off = MAX(off / 2, 1);

		_vm->_draw->_scrollOffsetY += off;
		_vm->_video->dirtyRectsAll();

		_vm->_util->setMousePos(x, _vm->_height - _vm->_video->_splitHeight2 -
				_vm->_draw->_cursorHeight);
	}

	_vm->_util->setScrollOffset();
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

void Game::start(void) {
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

	if (_numEnvironments >= Environments::kEnvironmentCount)
		return;

	_environments->set(_numEnvironments);

	curBackupPos = _curEnvironment;
	_numEnvironments++;
	_curEnvironment = _numEnvironments;

	_script = new Script(_vm);
	_resources = new Resources(_vm);
	if (flags & 1)
		_vm->_inter->_variables = 0;

	strncpy0(_curTotFile, newTotFile, 9);
//	if (_vm->getGameType() == kGameTypeGeisha)
//		strcat(_curTotFile, ".0OT");
//	else
	strcat(_curTotFile, ".TOT");

	if (_vm->_inter->_terminate != 0) {
		clearUnusedEnvironment();
		return;
	}

	_hotspots->push(0, true);

	if (flags & 2)
		playTot(-1);
	else
		playTot(0);

	if (_vm->_inter->_terminate != 2)
		_vm->_inter->_terminate = 0;

	_hotspots->pop();

	if ((flags & 1) && _vm->_inter->_variables) {
		_vm->_inter->delocateVars();
	}

	clearUnusedEnvironment();

	_numEnvironments--;
	_curEnvironment = curBackupPos;
	_environments->get(_numEnvironments);
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
