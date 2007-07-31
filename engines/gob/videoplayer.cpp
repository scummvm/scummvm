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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "gob/videoplayer.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/video.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/palanim.h"
#include "gob/inter.h"

namespace Gob {

const char *VideoPlayer::_extensions[] = { "IMD", "VMD" };

VideoPlayer::VideoPlayer(GobEngine *vm) : _vm(vm) {
	_curFile[0] = 0;
	_stream = 0;
	_video = 0;
	_backSurf = false;
}

VideoPlayer::~VideoPlayer() {
	closeVideo();
}

bool VideoPlayer::openVideo(const char *video, int16 x, int16 y, int16 flags, Type which) {
	char fileName[256];

	strncpy0(fileName, video, 250);

	char *extStart = strchr(fileName, '.');
	if (extStart) {
		// The requested file already has an extension. Verifying.

		int i;
		for (i = 0; i < ARRAYSIZE(_extensions); i++) {
			if (!scumm_stricmp(extStart + 1, _extensions[i])) {
				if ((which != kVideoTypeTry) && (which == ((Type) i))) {
					warning("Attempted to open video \"%s\", "
							"but requested a different type", fileName);
					return false;
				}
				which = (Type) i;
				break;
			}
		}
		if (i >= ARRAYSIZE(_extensions))
			extStart = 0;

	}

	if (!extStart) {
		// No or unrecognized extension. Probing.

		int len = strlen(fileName);

		int i;
		for (i = 0; i < ARRAYSIZE(_extensions); i++) {
			if ((which == kVideoTypeTry) || (which == ((Type) i))) {
				int16 handle;

				fileName[len] = '.';
				fileName[len + 1] = 0;
				strcat(fileName, _extensions[i]);

				handle = _vm->_dataIO->openData(fileName);
				if (handle >= 0) {
					_vm->_dataIO->closeData(handle);
					which = (Type) i;
					break;
				}
			}
		}
		if ((i >= ARRAYSIZE(_extensions)) || (which == kVideoTypeTry)) {
			fileName[len] = 0;
			warning("Couldn't open video \"%s\"", fileName);
			return false;
		}

	}

	if (scumm_strnicmp(_curFile, fileName, strlen(fileName))) {
		closeVideo();

		int16 handle = _vm->_dataIO->openData(fileName);

		if (handle < 0) {
			warning("Couldn't open video \"%s\": No such file", fileName);
			return false;
		}

		_stream = _vm->_dataIO->openAsStream(handle, true);

		if (which == kVideoTypeIMD) {
			_video = new Imd();
		} else if (which == kVideoTypeVMD) {
			_video = new Vmd();
		} else {
			warning("Couldn't open video \"%s\": Invalid video Type", fileName);
			closeVideo();
			return false;
		}

		if (!_video->load(*_stream)) {
			warning("While loading video \"%s\"", fileName);
			closeVideo();
			return false;
		}

		_video->setXY(x, y);

		if (!(flags & kFlagNoVideo)) {
			_backSurf = ((flags & kFlagFrontSurface) == 0);
			SurfaceDesc::Ptr surf = _vm->_draw->_spritesArray[_backSurf ? 21 : 20];
			_video->setVideoMemory(surf->getVidMem(), surf->getWidth(), surf->getHeight());
		} else
			_video->setVideoMemory();

		_video->enableSound(*_vm->_mixer);
	}

	if (!_video)
		return false;

	WRITE_VAR(7, _video->getFramesCount());

	return true;
}

void VideoPlayer::play(int16 startFrame, int16 lastFrame, int16 breakKey,
		uint16 palCmd, int16 palStart, int16 palEnd,
		int16 palFrame, int16 endFrame, bool fade, int16 reverseTo) {

	if (!_video)
		return;

	breakKey = 27;
	if (startFrame < 0)
		startFrame = _video->getCurrentFrame();
	if (lastFrame < 0)
		lastFrame = _video->getFramesCount() - 1;
	if (palFrame < 0)
		palFrame = startFrame;
	if (endFrame < 0)
		endFrame = lastFrame;
	palCmd &= 0x3F;

	if (_video->getCurrentFrame() != startFrame)
		_video->seekFrame(startFrame);

	_vm->_draw->_showCursor = 0;
	_vm->_util->setFrameRate(12);

	if (fade)
		_vm->_palAnim->fade(0, -2, 0);

	while (startFrame <= lastFrame) {
		if (doPlay(startFrame, breakKey, palCmd, palStart, palEnd, palFrame, endFrame))
			break;

		if (fade) {
			_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, -2, 0);
			fade = false;
		}

		_video->waitEndFrame();
		startFrame++;
	}

	if (reverseTo >= 0) {
		int16 toFrame = _video->getFramesCount() - reverseTo;
		for (int i = _video->getCurrentFrame(); i >= toFrame; i--) {
			_video->seekFrame(i, SEEK_SET, true);
			if (doPlay(i, breakKey, 0, 0, 0, 0, 0)) {
				_vm->_palAnim->fade(0, -2, 0);
				memset((char *) _vm->_draw->_vgaPalette, 0, 768);
			}
			_video->waitEndFrame();
		}
	}
}

int16 VideoPlayer::getFramesCount() const {
	if (!_video)
		return 0;

	return _video->getFramesCount();
}

int16 VideoPlayer::getCurrentFrame() const {
	if (!_video)
		return 0;

	return _video->getCurrentFrame();
}

bool VideoPlayer::doPlay(int16 frame, int16 breakKey,
		uint16 palCmd, int16 palStart, int16 palEnd,
		int16 palFrame, int16 endFrame) {

	bool modifiedPal = false;

	if ((frame == palFrame) || ((frame == endFrame) && (palCmd == 8))) {
		modifiedPal = true;
		_vm->_draw->_applyPal = true;

		if (palCmd >= 4)
			copyPalette(palStart, palEnd);
	}

	if (modifiedPal && (palCmd == 8) && !_backSurf)
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);


	CoktelVideo::State state = _video->nextFrame();
	WRITE_VAR(11, frame);


	if (modifiedPal && (palCmd == 16)) {
		if (_backSurf)
			_vm->_draw->forceBlit();
		_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, -2, 0);
		_vm->_draw->_noInvalidated = true;
	}

	if (state.flags & CoktelVideo::kStatePalette) {
		copyPalette(palStart, palEnd);

		if (!_backSurf)
			_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
		else
			_vm->_draw->_applyPal = true;
	}

	if (modifiedPal && (palCmd == 8) && _backSurf)
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);


	if (_backSurf) {
		_vm->_draw->invalidateRect(state.left, state.top, state.right, state.bottom);
		_vm->_draw->blitInvalidated();
	}
	_vm->_video->retrace();


	if (modifiedPal && ((palCmd == 2) || (palCmd == 4)))
		_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, -2, 0);


	_vm->_util->processInput();

	if (_vm->_quitRequested) {
		_video->disableSound();
		return true;
	}

	if (breakKey != 0) {
		_vm->_util->getMouseState(&_vm->_global->_inter_mouseX,
				&_vm->_global->_inter_mouseY, &_vm->_game->_mouseButtons);

		_vm->_inter->storeKey(_vm->_util->checkKey());
		if (VAR(0) == (unsigned) breakKey) {
			_video->disableSound();
			return true;
		}
	}

	return false;
}

void VideoPlayer::copyPalette(int16 palStart, int16 palEnd) {
	if ((palStart == -1) || (palEnd == -1))
		memcpy((char *) _vm->_global->_pPaletteDesc->vgaPal,
				_video->getPalette(), 768);
	else
		memcpy(((char *) (_vm->_global->_pPaletteDesc->vgaPal)) +
				palStart * 3, _video->getPalette() + palStart * 3,
				(palEnd - palStart + 1) * 3);
}

void VideoPlayer::writeVideoInfo(const char *video, int16 varX, int16 varY,
		int16 varFrames, int16 varWidth, int16 varHeight) {

	if (openVideo(video)) {
		WRITE_VAR_OFFSET(varX, _video->getX());
		WRITE_VAR_OFFSET(varY, _video->getY());
		WRITE_VAR_OFFSET(varFrames, _video->getFramesCount());
		WRITE_VAR_OFFSET(varWidth, _video->getWidth());
		WRITE_VAR_OFFSET(varHeight, _video->getHeight());
		closeVideo();
	} else {
		WRITE_VAR_OFFSET(varX, -1);
		WRITE_VAR_OFFSET(varY, -1);
		WRITE_VAR_OFFSET(varFrames, -1);
		WRITE_VAR_OFFSET(varWidth, -1);
		WRITE_VAR_OFFSET(varHeight, -1);
	}
}

void VideoPlayer::closeVideo() {
	delete _video;
	delete _stream;

	_video = 0;
	_stream = 0;
}

} // End of namespace Gob
