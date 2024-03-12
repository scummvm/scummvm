/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "neverhood/gamemodule.h"
#include "neverhood/smackerplayer.h"
#include "neverhood/palette.h"
#include "neverhood/resourceman.h"
#include "neverhood/scene.h"

namespace Neverhood {

// SmackerSurface

SmackerSurface::SmackerSurface(NeverhoodEngine *vm)
	: BaseSurface(vm, 0, 0, 0, "smacker"), _smackerFrame(nullptr) {
}

void SmackerSurface::draw() {
	if (_smackerFrame && _visible && _drawRect.width > 0 && _drawRect.height > 0) {
		_vm->_screen->drawSurface2(_smackerFrame, _drawRect, _clipRect, false, ++_version);
		if (_subtitles && _subtitles->isValid()) {
			_subtitles->renderFrame(frameNumber, 160);
			const Graphics::Surface *bottom = _subtitles->getBottomSubs();
			if (bottom) {
				NDrawRect subDrawRect;
				subDrawRect.x = _drawRect.x;
				subDrawRect.y = _drawRect.y + _drawRect.height - 17;
				subDrawRect.width = _drawRect.width;
				subDrawRect.height = 16;
				_vm->_screen->drawSurface2(bottom, subDrawRect, _clipRect, true, ++_version, nullptr, _subtitles->getSubtitleAlpha());
			}
			const Graphics::Surface *top = _subtitles->getTopSubs();
			if (top) {
				NDrawRect subDrawRect;
				subDrawRect.x = _drawRect.x;
				subDrawRect.y = _drawRect.y + 1;
				subDrawRect.width = _drawRect.width;
				subDrawRect.height = 16;
				_vm->_screen->drawSurface2(top, subDrawRect, _clipRect, true, ++_version, nullptr, _subtitles->getSubtitleAlpha());
			}
		}
	}
}

void SmackerSurface::setSmackerFrame(const Graphics::Surface *smackerFrame) {
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = smackerFrame->w;
	_drawRect.height = smackerFrame->h;
	_sysRect.x = 0;
	_sysRect.y = 0;
	_sysRect.width = (smackerFrame->w + 3) & 0xFFFC; // align by 4 bytes
	_sysRect.height = smackerFrame->h;
	_smackerFrame = smackerFrame;
}

void SmackerSurface::unsetSmackerFrame() {
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = 0;
	_drawRect.height = 0;
	_sysRect.x = 0;
	_sysRect.y = 0;
	_sysRect.width = 0;
	_sysRect.height = 0;
	_smackerFrame = nullptr;
}

// SmackerDoubleSurface

SmackerDoubleSurface::SmackerDoubleSurface(NeverhoodEngine *vm)
	: SmackerSurface(vm) {
}

void SmackerDoubleSurface::draw() {
	if (_smackerFrame && _visible && _drawRect.width > 0 && _drawRect.height > 0) {
		_vm->_screen->drawDoubleSurface2(_smackerFrame, _drawRect);
		if (_subtitles && _subtitles->isValid()) {
			_subtitles->renderFrame(frameNumber, 160);
			const Graphics::Surface *bottom = _subtitles->getBottomSubs();
			if (bottom) {
				NDrawRect subDrawRect;
				subDrawRect.x = _drawRect.x;
				subDrawRect.y = _drawRect.y + _drawRect.height * 2 - 34;
				subDrawRect.width = _drawRect.width;
				subDrawRect.height = 16;
				_vm->_screen->drawDoubleSurface2Alpha(bottom, subDrawRect, _subtitles->getSubtitleAlpha());
			}
			const Graphics::Surface *top = _subtitles->getTopSubs();
			if (top) {
				NDrawRect subDrawRect;
				subDrawRect.x = _drawRect.x;
				subDrawRect.y = _drawRect.y + 2;
				subDrawRect.width = _drawRect.width;
				subDrawRect.height = 16;
				_vm->_screen->drawDoubleSurface2Alpha(top, subDrawRect, _subtitles->getSubtitleAlpha());
			}
		}
	}
}

// NeverhoodSmackerDecoder

void NeverhoodSmackerDecoder::forceSeekToFrame(uint frame) {
	if (!isVideoLoaded())
		return;

	if (frame >= getFrameCount())
		error("Can't force Smacker seek to invalid frame %d", frame);

	if (_header.audioInfo[0].hasAudio)
		error("Can't force Smacker frame seek with audio");
	if (!rewind())
		error("Failed to rewind");

	SmackerVideoTrack *videoTrack = (SmackerVideoTrack *)getTrack(0);
	uint32 offset = 0;
	for (uint32 i = 0; i < frame; i++) {
		videoTrack->increaseCurFrame();
		offset += _frameSizes[i] & ~3;
	}

	_fileStream->seek(offset, SEEK_CUR);
}

// SmackerPlayer

SmackerPlayer::SmackerPlayer(NeverhoodEngine *vm, Scene *scene, uint32 fileHash, bool doubleSurface, bool flag, bool paused)
	: Entity(vm, 0), _scene(scene), _doubleSurface(doubleSurface), _videoDone(false), _paused(paused),
	_palette(nullptr), _smackerDecoder(nullptr), _smackerSurface(nullptr), _stream(nullptr), _smackerFirst(true),
	_drawX(-1), _drawY(-1) {

	SetUpdateHandler(&SmackerPlayer::update);

	if (_doubleSurface) {
		_smackerSurface.reset(new SmackerDoubleSurface(_vm));
	} else {
		_smackerSurface.reset(new SmackerSurface(_vm));
	}

	open(fileHash, flag);
}

SmackerPlayer::~SmackerPlayer() {
	close();
	_smackerSurface.reset();
}

void SmackerPlayer::open(uint32 fileHash, bool keepLastFrame) {
	debug(0, "SmackerPlayer::open(%08X)", fileHash);

	_fileHash = fileHash;
	_keepLastFrame = keepLastFrame;

	close();

	_smackerFirst = true;

	_stream = _vm->_res->createStream(fileHash);

	_smackerSurface->_subtitles.reset(new SubtitlePlayer(_vm, fileHash, 320));

	_smackerDecoder = new NeverhoodSmackerDecoder();
	_smackerDecoder->loadStream(_stream);

	_palette = new Palette(_vm);
	_palette->usePalette();

	if (!_paused)
		_smackerDecoder->start();

}

void SmackerPlayer::close() {
	if (_smackerDecoder)
		_smackerDecoder->stop();
	delete _smackerDecoder;
	delete _palette;
	// NOTE The SmackerDecoder deletes the _stream
	_smackerDecoder = nullptr;
	_palette = nullptr;
	_stream = nullptr;
	_smackerSurface->unsetSmackerFrame();
}

void SmackerPlayer::gotoFrame(int frameNumber) {
	if (_smackerDecoder) {
		_smackerDecoder->forceSeekToFrame(frameNumber);
		updateFrame();
	}
}

uint32 SmackerPlayer::getFrameCount() {
	return _smackerDecoder ? _smackerDecoder->getFrameCount() : 0;
}

uint32 SmackerPlayer::getFrameNumber() {
	return _smackerDecoder ? _smackerDecoder->getCurFrame() : 0;
}

uint SmackerPlayer::getStatus() {
	return 0;
}

void SmackerPlayer::setDrawPos(int16 x, int16 y) {
	_drawX = x;
	_drawY = y;
	if (_smackerSurface) {
		_smackerSurface->getDrawRect().x = _drawX;
		_smackerSurface->getDrawRect().y = _drawY;
	}
}

void SmackerPlayer::rewind() {
	if (_smackerDecoder)
		_smackerDecoder->rewind();
}

void SmackerPlayer::update() {

	if (!_smackerDecoder)
		return;

	if (_paused) {
		if (_smackerFirst)
			updateFrame();
	} else {
		if (!_smackerDecoder->endOfVideo()) {
			updateFrame();
		} else if (!_keepLastFrame) {
			// Inform the scene about the end of the video playback
			if (_scene)
				sendMessage(_scene, NM_ANIMATION_STOP, 0);
			_videoDone = true;
		} else {
			rewind();
			updateFrame();
			_videoDone = false;
		}
	}

}

void SmackerPlayer::updateFrame() {

	if (!_smackerDecoder || !_smackerSurface)
		return;

	const Graphics::Surface *smackerFrame = _smackerDecoder->decodeNextFrame();

	_smackerSurface->frameNumber = _smackerDecoder->getCurFrame();

	if (_smackerFirst) {
		_smackerSurface->setSmackerFrame(smackerFrame);
		if (_drawX < 0 || _drawY < 0) {
			if (_doubleSurface) {
				_drawX = 320 - _smackerDecoder->getWidth();
				_drawY = 240 - _smackerDecoder->getHeight();
			} else {
				_drawX = (640 - _smackerDecoder->getWidth()) / 2;
				_drawY = (480 - _smackerDecoder->getHeight()) / 2;
			}
		}
		_smackerSurface->getDrawRect().x = _drawX;
		_smackerSurface->getDrawRect().y = _drawY;
		_smackerFirst = false;
	}

	if (_smackerDecoder->hasDirtyPalette())
		updatePalette();

}

void SmackerPlayer::updatePalette() {
	byte tempPalette[1024];
	const byte *smackerPalette = _smackerDecoder->getPalette();
	for (int i = 0; i < 256; i++) {
		tempPalette[i * 4 + 0] = smackerPalette[i * 3 + 0];
		tempPalette[i * 4 + 1] = smackerPalette[i * 3 + 1];
		tempPalette[i * 4 + 2] = smackerPalette[i * 3 + 2];
	}

	// WORKAROUND: Scene 3, module 3000 defines a black color 255 instead of
	// white, which results in the mouse cursor showing black. I'm not sure if
	// color 255 is always supposed to be white. It's not feasible to check
	// all scenes for a glitch that only seems to manifest in one, therefore
	// we define color 255 to be white only for that scene.
	if (_vm->_gameModule->getCurrentModuleNum() == 3000 && _vm->_gameState.sceneNum == 3)
			tempPalette[255 * 4 + 0] = tempPalette[255 * 4 + 1] = tempPalette[255 * 4 + 2] = 0xFF;

	_palette->copyPalette(tempPalette, 0, 256, 0);
}

} // End of namespace Neverhood
