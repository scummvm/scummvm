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
 */

#include "neverhood/smackerplayer.h"
#include "neverhood/palette.h"
#include "neverhood/resourceman.h"
#include "neverhood/scene.h"

namespace Neverhood {

// SmackerSurface

SmackerSurface::SmackerSurface(NeverhoodEngine *vm)
	: BaseSurface(vm, 0, 0, 0), _smackerFrame(NULL) {
}

void SmackerSurface::draw() {
	if (_smackerFrame && _visible && _drawRect.width > 0 && _drawRect.height > 0) {
		_vm->_screen->drawSurface2(_smackerFrame, _drawRect, _clipRect, false);
	}
}

void SmackerSurface::setSmackerFrame(const Graphics::Surface *smackerFrame) {
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = smackerFrame->w;
	_drawRect.height = smackerFrame->h;
	// TODO: Check if _sysRect is needed at all in the reimplementation...
	_sysRect.x = 0;
	_sysRect.y = 0;
	_sysRect.width = (smackerFrame->w + 3) & 0xFFFC; // align by 4 bytes
	_sysRect.height = smackerFrame->h;
	_smackerFrame = smackerFrame;
}

// SmackerDoubleSurface

SmackerDoubleSurface::SmackerDoubleSurface(NeverhoodEngine *vm)
	: SmackerSurface(vm) {
}

void SmackerDoubleSurface::draw() {
	if (_smackerFrame && _visible && _drawRect.width > 0 && _drawRect.height > 0) {
		_vm->_screen->drawDoubleSurface2(_smackerFrame, _drawRect);
	}
}

// SmackerPlayer

SmackerPlayer::SmackerPlayer(NeverhoodEngine *vm, Scene *scene, uint32 fileHash, bool doubleSurface, bool flag)
	: Entity(vm, 0), _scene(scene), _doubleSurface(doubleSurface), _dirtyFlag(false), _flag2(false),
	_palette(NULL), _smackerDecoder(NULL), _smackerSurface(NULL), _stream(NULL) {

	SetUpdateHandler(&SmackerPlayer::update);
	open(fileHash, flag);
}

SmackerPlayer::~SmackerPlayer() {
	close();
}

void SmackerPlayer::open(uint32 fileHash, bool flag1) {
	debug("SmackerPlayer::open(%08X)", fileHash);
	
	_flag1 = flag1;

	close();

	_stream = _vm->_res->createStream(fileHash);

	// TODO: _flag1 stuff

	_smackerDecoder = new Video::SmackerDecoder(_vm->_mixer);
	_smackerDecoder->loadStream(_stream);
	
	_palette = new Palette(_vm);
	_palette->usePalette();
	
}

void SmackerPlayer::close() {
	delete _smackerDecoder;
	delete _palette;
	// NOTE: The SmackerDecoder deletes the _stream
	delete _smackerSurface;
	_smackerDecoder = NULL;
	_palette = NULL;
	_stream = NULL;
	_smackerSurface = NULL;
}

void SmackerPlayer::gotoFrame(uint frameNumber) {
}

uint SmackerPlayer::getStatus() {
	return 0;
}

void SmackerPlayer::update() {

	if (!_smackerDecoder)
		return;

	if (_dirtyFlag) {
		// TODO _vm->_screen->resetDirtyRects();
		_dirtyFlag = false;
	}

	if (!_smackerDecoder->endOfVideo()) {

		const Graphics::Surface *smackerFrame = _smackerDecoder->decodeNextFrame();

		if (!_smackerSurface) {
			if (_doubleSurface) {
				// TODO: Use SmackerDoubleSurface
				_smackerSurface = new SmackerDoubleSurface(_vm);
				_smackerSurface->getDrawRect().x = 320 - _smackerDecoder->getWidth();
				_smackerSurface->getDrawRect().y = 240 - _smackerDecoder->getHeight();
				// TODO DoubleDrawSurface.field_28 = false;
				_smackerSurface->setSmackerFrame(smackerFrame);
			} else {
				_smackerSurface = new SmackerSurface(_vm);
				_smackerSurface->getDrawRect().x = (640 - _smackerDecoder->getWidth()) / 2;
				_smackerSurface->getDrawRect().y = (480 - _smackerDecoder->getHeight()) / 2;
				_smackerSurface->setSmackerFrame(smackerFrame);
			}
		}
		
		if (_doubleSurface) {
			// TODO .text:0046422D				 mov	 byte ptr [eax+28h], 1
		}

		// TODO _vm->_screen->_skipUpdate = true;
		_dirtyFlag = true;

		if (_smackerDecoder->hasDirtyPalette()) {
			updatePalette();
		}

		if (_smackerDecoder->endOfVideo() && !_flag1) {
			if (_scene) {
				_scene->sendMessage(0x3002, 0, this);
			}
			_flag2 = true;
		} else {
			_flag2 = false;
		}
		
	}

}

void SmackerPlayer::updatePalette() {
	byte tempPalette[1024];
	const byte *smackerPalette = _smackerDecoder->getPalette();
	for (int i = 0; i < 256; i++) {
		tempPalette[i * 4 + 0] = smackerPalette[i * 3 + 0];
		tempPalette[i * 4 + 1] = smackerPalette[i * 3 + 1];
		tempPalette[i * 4 + 2] = smackerPalette[i * 3 + 2];
	}
	_palette->copyPalette(tempPalette, 0, 256, 0);
}

} // End of namespace Neverhood
