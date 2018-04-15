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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sludge/allfiles.h"
#include "sludge/cursors.h"
#include "sludge/event.h"
#include "sludge/freeze.h"
#include "sludge/graphics.h"
#include "sludge/newfatal.h"
#include "sludge/people.h"
#include "sludge/sprites.h"
#include "sludge/sprbanks.h"
#include "sludge/sludge.h"
#include "sludge/sludger.h"

namespace Sludge {

CursorManager::CursorManager(SludgeEngine *vm) {
	_vm = vm;
	init();
}

CursorManager::~CursorManager() {
	kill();
}

void CursorManager::init() {
	_mouseCursorAnim = new PersonaAnimation();
	_mouseCursorFrameNum = 0;
	_mouseCursorCountUp = 0;
}

void CursorManager::kill() {
	if (_mouseCursorAnim) {
		delete _mouseCursorAnim;
		_mouseCursorAnim = nullptr;
	}
	_mouseCursorAnim = nullptr;
}

void CursorManager::pickAnimCursor(PersonaAnimation  *pp) {
	if (_mouseCursorAnim) {
		delete _mouseCursorAnim;
		_mouseCursorAnim = nullptr;
	}
	_mouseCursorAnim = pp;
	_mouseCursorFrameNum = 0;
	_mouseCursorCountUp = 0;
}

void CursorManager::displayCursor() {
	if (_mouseCursorAnim && _mouseCursorAnim->numFrames) {

		int spriteNum = _mouseCursorAnim->frames[_mouseCursorFrameNum].frameNum;
		int flipMe = 0;

		if (spriteNum < 0) {
			spriteNum = -spriteNum;
			flipMe = 1;
			if (spriteNum >= _mouseCursorAnim->theSprites->bank.total)
				spriteNum = 0;
		} else {
			if (spriteNum >= _mouseCursorAnim->theSprites->bank.total)
				flipMe = 2;
		}

		if (flipMe != 2) {
			if (flipMe) {
				_vm->_gfxMan->flipFontSprite(
						_vm->_evtMan->mouseX(), _vm->_evtMan->mouseY(),
						_mouseCursorAnim->theSprites->bank.sprites[spriteNum],
						_mouseCursorAnim->theSprites->bank.myPalette /* ( spritePalette&) NULL*/);
			} else {
				_vm->_gfxMan->fontSprite(
						_vm->_evtMan->mouseX(), _vm->_evtMan->mouseY(),
						_mouseCursorAnim->theSprites->bank.sprites[spriteNum],
						_mouseCursorAnim->theSprites->bank.myPalette /* ( spritePalette&) NULL*/);
			}
		}

		if (++_mouseCursorCountUp >= _mouseCursorAnim->frames[_mouseCursorFrameNum].howMany) {
			_mouseCursorCountUp = 0;
			_mouseCursorFrameNum++;
			_mouseCursorFrameNum %= _mouseCursorAnim->numFrames;
		}
	}
}

void CursorManager::pasteCursor(int x, int y, PersonaAnimation  *c) {
	if (c->numFrames)
		_vm->_gfxMan->pasteSpriteToBackDrop(x, y, c->theSprites->bank.sprites[c->frames[0].frameNum], c->theSprites->bank.myPalette);
}

void CursorManager::freeze(FrozenStuffStruct *frozenStuff) {
	frozenStuff->mouseCursorAnim = _mouseCursorAnim;
	frozenStuff->mouseCursorFrameNum = _mouseCursorFrameNum;
	_mouseCursorAnim = new PersonaAnimation();
	_mouseCursorFrameNum = 0;
}

void CursorManager::resotre(FrozenStuffStruct *frozenStuff) {
	if (_mouseCursorAnim) {
		delete _mouseCursorAnim;
		_mouseCursorAnim = nullptr;
	}
	_mouseCursorAnim = frozenStuff->mouseCursorAnim;
	_mouseCursorFrameNum = frozenStuff->mouseCursorFrameNum;
}

void CursorManager::saveCursor(Common::WriteStream *stream) {
	_mouseCursorAnim->save(stream);
	stream->writeUint16BE(_mouseCursorFrameNum);
}

bool CursorManager::loadCursor(Common::SeekableReadStream *stream) {
	_mouseCursorAnim = new PersonaAnimation;
	if (!checkNew(_mouseCursorAnim))
		return false;
	if (!_mouseCursorAnim->load(stream))
		return false;
	_mouseCursorFrameNum = stream->readUint16BE();
	return true;
}

} // End of namespace Sludge
