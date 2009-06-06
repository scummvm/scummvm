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


#include "gob/videoplayer.h"
#include "gob/helper.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/video.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/palanim.h"
#include "gob/inter.h"
#include "gob/map.h"
#include "gob/sound/sound.h"

namespace Gob {

const char *VideoPlayer::_extensions[] = { "IMD", "VMD", "RMD" };

VideoPlayer::Video::Video(GobEngine *vm) : _vm(vm), _stream(0), _video(0) {
}

VideoPlayer::Video::~Video() {
	close();
}

bool VideoPlayer::Video::open(const char *fileName, Type which) {
	close();

	int16 handle = _vm->_dataIO->openData(fileName);

	if (handle < 0) {
		warning("Couldn't open video \"%s\": No such file", fileName);
		return false;
	}

	_stream = _vm->_dataIO->openAsStream(handle, true);

	if (which == kVideoTypeIMD) {
		_video = new Graphics::Imd();
	} else if (which == kVideoTypeVMD) {
		_video = new Graphics::Vmd(_vm->_video->_palLUT);
	} else if (which == kVideoTypeRMD) {
		_video = new Graphics::Vmd(_vm->_video->_palLUT);
	} else {
		warning("Couldn't open video \"%s\": Invalid video Type", fileName);
		close();
		return false;
	}

	if (!_video->load(*_stream)) {
		warning("While loading video \"%s\"", fileName);
		close();
		return false;
	}

	_fileName = fileName;

	_defaultX = _video->getX();
	_defaultY = _video->getY();

	return true;
}

void VideoPlayer::Video::close() {
	delete _video;
	delete _stream;

	_video = 0;
	_stream = 0;
	_fileName.clear();
	memset(&_state, 0, sizeof(Graphics::CoktelVideo::State));
	_defaultX = _defaultY = 0;
}

bool VideoPlayer::Video::isOpen() const {
	return (_video != 0);
}

const char *VideoPlayer::Video::getFileName() const {
	return _fileName.c_str();
}

Graphics::CoktelVideo *VideoPlayer::Video::getVideo() {
	return _video;
}

const Graphics::CoktelVideo *VideoPlayer::Video::getVideo() const {
	return _video;
}

uint32 VideoPlayer::Video::getFeatures() const {
	return _video->getFeatures();
}

Graphics::CoktelVideo::State VideoPlayer::Video::getState() const {
	return _state;
}

int16 VideoPlayer::Video::getDefaultX() const {
	return _defaultX;
}

int16 VideoPlayer::Video::getDefaultY() const {
	return _defaultY;
}

bool VideoPlayer::Video::hasExtraData(const char *fileName) const {
	if (!_video)
		return false;

	return _video->hasExtraData(fileName);
}

Common::MemoryReadStream *VideoPlayer::Video::getExtraData(const char *fileName) {
	if (!_video)
		return 0;

	return _video->getExtraData(fileName);
}

Graphics::CoktelVideo::State VideoPlayer::Video::nextFrame() {
	if (_video)
		_state = _video->nextFrame();

	return _state;
}


VideoPlayer::VideoPlayer(GobEngine *vm) : _vm(vm) {
	_primaryVideo = new Video(vm);
	_ownSurf = false;
	_backSurf = false;
	_needBlit = false;
	_noCursorSwitch = false;
}

VideoPlayer::~VideoPlayer() {
	delete _primaryVideo;
	for (uint i = 0; i < _videoSlots.size(); i++)
		delete _videoSlots[i];
}

bool VideoPlayer::findFile(char *fileName, Type &which) {
	char *extStart = strrchr(fileName, '.');
	// There's no empty extension
	if (extStart == (fileName + strlen(fileName) - 1)) {
		*extStart = 0;
		extStart = 0;
	}

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

	return true;
}

bool VideoPlayer::primaryOpen(const char *videoFile, int16 x, int16 y,
		int32 flags, Type which) {

	char fileName[256];

	strncpy0(fileName, videoFile, 250);

	if (!findFile(fileName, which))
		return false;

	if (scumm_strnicmp(_primaryVideo->getFileName(), fileName, strlen(fileName))) {
		if (!_primaryVideo->open(fileName, which))
			return false;

		// WORKAROUND: In some rare cases, the cursor should still be
		// displayed while a video is playing.
		_noCursorSwitch = false;
		if (_vm->getGameType() == kGameTypeLostInTime) {
			if (!scumm_stricmp(fileName, "PORTA03.IMD") ||
			    !scumm_stricmp(fileName, "PORTA03A.IMD") ||
			    !scumm_stricmp(fileName, "CALE1.IMD") ||
			    !scumm_stricmp(fileName, "AMIL2.IMD") ||
			    !scumm_stricmp(fileName, "AMIL3B.IMD") ||
			    !scumm_stricmp(fileName, "DELB.IMD"))
				_noCursorSwitch = true;
		}

		_ownSurf = false;

		if (!(flags & kFlagNoVideo)) {
			SurfaceDescPtr surf;

			if (flags & kFlagOtherSurface) {
				_ownSurf = true;
				_backSurf = false;

				surf = _vm->_video->initSurfDesc(_vm->_global->_videoMode,
						_primaryVideo->getVideo()->getWidth(),
						_primaryVideo->getVideo()->getHeight(), 0);
				_vm->_draw->_spritesArray[x] = surf;

				x = 0;
			} else if (flags & kFlagScreenSurface) {
				_ownSurf = true;
				_backSurf = false;

				surf = _vm->_video->initSurfDesc(_vm->_global->_videoMode,
						_vm->_width, _vm->_height, 0);
				_vm->_draw->_spritesArray[0] = surf;
			} else {
				_backSurf = ((flags & kFlagFrontSurface) == 0);
				surf = _vm->_draw->_spritesArray[_backSurf ? 21 : 20];
			}

			_primaryVideo->getVideo()->setVideoMemory(surf->getVidMem(),
					surf->getWidth(), surf->getHeight());

		} else
			_primaryVideo->getVideo()->setVideoMemory();

		_needBlit = ((flags & kFlagUseBackSurfaceContent) != 0) && ((flags & kFlagFrontSurface) != 0);

		_primaryVideo->getVideo()->enableSound(*_vm->_mixer);
	}

	if (!_primaryVideo->isOpen())
		return false;

	_primaryVideo->getVideo()->setFrameRate(_vm->_util->getFrameRate());
	_primaryVideo->getVideo()->setXY(x, y);
	WRITE_VAR(7, _primaryVideo->getVideo()->getFramesCount());

	return true;
}

bool VideoPlayer::primaryPlay(int16 startFrame, int16 lastFrame, int16 breakKey,
		uint16 palCmd, int16 palStart, int16 palEnd,
		int16 palFrame, int16 endFrame, bool fade, int16 reverseTo, bool forceSeek) {

	if (!_primaryVideo->isOpen())
		return false;

	Graphics::CoktelVideo &video = *(_primaryVideo->getVideo());

	breakKey = 27;
	if (startFrame < 0)
		startFrame = video.getCurrentFrame();
	if (lastFrame < 0)
		lastFrame = video.getFramesCount() - 1;
	if (palFrame < 0)
		palFrame = startFrame;
	if (endFrame < 0)
		endFrame = lastFrame;
	palCmd &= 0x3F;

	if (video.getCurrentFrame() != startFrame) {
		if (!forceSeek && (video.getFeatures() & Graphics::CoktelVideo::kFeaturesSound))
			startFrame = video.getCurrentFrame();
		else
			video.seekFrame(startFrame);
	}

	_vm->_draw->_showCursor = _noCursorSwitch ? 3 : 0;

	if (fade)
		_vm->_palAnim->fade(0, -2, 0);

	bool canceled = false;

	while (startFrame <= lastFrame) {
		if (doPlay(startFrame, breakKey, palCmd, palStart, palEnd, palFrame, endFrame)) {
			canceled = true;
			break;
		}

		evalBgShading(video);

		if (fade) {
			_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, -2, 0);
			fade = false;
		}

		if (!_noCursorSwitch)
			video.waitEndFrame();
		startFrame++;
	}

	evalBgShading(video);

	if (reverseTo >= 0) {
		int16 toFrame = video.getFramesCount() - reverseTo;
		for (int i = video.getCurrentFrame(); i >= toFrame; i--) {
			video.seekFrame(i, SEEK_SET, true);

			bool b = doPlay(i, breakKey, 0, 0, 0, 0, 0);
			evalBgShading(video);

			if (b) {
				_vm->_palAnim->fade(0, -2, 0);
				memset((char *) _vm->_draw->_vgaPalette, 0, 768);
			}

			if (!_noCursorSwitch)
				video.waitEndFrame();
		}
	}

	evalBgShading(video);

	return canceled;
}

void VideoPlayer::primaryClose() {
	_primaryVideo->close();
}

int VideoPlayer::slotOpen(const char *videoFile, Type which) {
	Video *video = new Video(_vm);
	char fileName[256];

	strncpy0(fileName, videoFile, 250);

	if (!findFile(fileName, which)) {
		delete video;
		return -1;
	}

	if (!video->open(fileName, which)) {
		delete video;
		return -1;
	}

	video->getVideo()->setVideoMemory();
	video->getVideo()->enableSound(*_vm->_mixer);

	int slot = getNextFreeSlot();

	_videoSlots[slot] = video;

	WRITE_VAR(7, video->getVideo()->getFramesCount());

	return slot;
}

int VideoPlayer::getNextFreeSlot() {
	uint slot;

	for (slot = 0; slot < _videoSlots.size(); slot++)
		if (!_videoSlots[slot])
			break;

	if (slot == _videoSlots.size())
		_videoSlots.push_back(0);

	return slot;
}

void VideoPlayer::slotPlay(int slot, int16 frame) {
	if ((slot < 0) || (((uint) slot) >= _videoSlots.size()) || !_videoSlots[slot])
		return;

	Graphics::CoktelVideo &video = *(_videoSlots[slot]->getVideo());

	if (frame < 0)
		frame = video.getCurrentFrame();

	if (frame >= video.getFramesCount())
		return;

	if (video.getCurrentFrame() != frame)
		video.seekFrame(frame);

	_videoSlots[slot]->nextFrame();
	WRITE_VAR(11, frame);

	evalBgShading(video);
}

void VideoPlayer::slotClose(int slot) {
	if ((slot < 0) || (((uint) slot) >= _videoSlots.size()) || !_videoSlots[slot])
		return;

	delete _videoSlots[slot];
	_videoSlots[slot] = 0;
}

void VideoPlayer::slotCopyFrame(int slot, byte *dest,
		uint16 left, uint16 top, uint16 width, uint16 height,
		uint16 x, uint16 y, uint16 pitch, int16 transp) {

	if ((slot < 0) || (((uint) slot) >= _videoSlots.size()) || !_videoSlots[slot])
		return;

	_videoSlots[slot]->getVideo()->copyCurrentFrame(dest,
			left, top, width, height, x, y, pitch, transp);
}

void VideoPlayer::slotCopyPalette(int slot, int16 palStart, int16 palEnd) {
	if ((slot < 0) || (((uint) slot) >= _videoSlots.size()) || !_videoSlots[slot])
		return;

	copyPalette(*(_videoSlots[slot]->getVideo()), palStart, palEnd);
}

void VideoPlayer::slotWaitEndFrame(int slot, bool onlySound) {
	Video *video = getVideoBySlot(slot);

	if (video) {
		Graphics::CoktelVideo &cVideo = *video->getVideo();

		if (!onlySound || (cVideo.getFeatures() & Graphics::CoktelVideo::kFeaturesSound))
			cVideo.waitEndFrame();
	}
}

bool VideoPlayer::slotIsOpen(int slot) const {
	if ((slot >= 0) && (((uint) slot) < _videoSlots.size()) && _videoSlots[slot])
		return true;

	return false;
}

void VideoPlayer::slotSetDoubleMode(int slot, bool doubleMode) {
	Video *video = getVideoBySlot(slot);

	if (video)
		video->getVideo()->setDoubleMode(doubleMode);
}

const VideoPlayer::Video *VideoPlayer::getVideoBySlot(int slot) const {
	if (slot < 0) {
		if (_primaryVideo->isOpen())
			return _primaryVideo;
	} else if (((uint) slot) < _videoSlots.size() && _videoSlots[slot])
		return _videoSlots[slot];

	return 0;
}

VideoPlayer::Video *VideoPlayer::getVideoBySlot(int slot) {
	if (slot < 0) {
		if (_primaryVideo->isOpen())
			return _primaryVideo;
	} else if (((uint) slot) < _videoSlots.size() && _videoSlots[slot])
		return _videoSlots[slot];

	return 0;
}

const char *VideoPlayer::getFileName(int slot) const {
	const Video *video = getVideoBySlot(slot);

	if (video)
		return video->getFileName();

	return "";
}

uint16 VideoPlayer::getFlags(int slot) const {
	const Video *video = getVideoBySlot(slot);

	if (video)
		return video->getVideo()->getFlags();

	return 0;
}

int16 VideoPlayer::getFramesCount(int slot) const {
	const Video *video = getVideoBySlot(slot);

	if (video)
		return video->getVideo()->getFramesCount();

	return 0;
}

int16 VideoPlayer::getCurrentFrame(int slot) const {
	const Video *video = getVideoBySlot(slot);

	if (video)
		return video->getVideo()->getCurrentFrame();

	return 0;
}

int16 VideoPlayer::getWidth(int slot) const {
	const Video *video = getVideoBySlot(slot);

	if (video)
		return video->getVideo()->getWidth();

	return 0;
}

int16 VideoPlayer::getHeight(int slot) const {
	const Video *video = getVideoBySlot(slot);

	if (video)
		return video->getVideo()->getHeight();

	return 0;
}

int16 VideoPlayer::getDefaultX(int slot) const {
	const Video *video = getVideoBySlot(slot);

	if (video)
		return video->getDefaultX();

	return 0;
}

int16 VideoPlayer::getDefaultY(int slot) const {
	const Video *video = getVideoBySlot(slot);

	if (video)
		return video->getDefaultY();

	return 0;
}

uint32 VideoPlayer::getFeatures(int slot) const {
	const Video *video = getVideoBySlot(slot);

	if (video)
		return video->getFeatures();

	return 0;
}

Graphics::CoktelVideo::State VideoPlayer::getState(int slot) const {
	const Video *video = getVideoBySlot(slot);
	Graphics::CoktelVideo::State state;

	if (video)
		state = video->getState();

	return state;
}

bool VideoPlayer::hasExtraData(const char *fileName, int slot) const {
	const Video *video = getVideoBySlot(slot);

	if (video)
		return video->hasExtraData(fileName);

	return false;
}

Common::MemoryReadStream *VideoPlayer::getExtraData(const char *fileName, int slot) {
	Video *video = getVideoBySlot(slot);

	if (video)
		return video->getExtraData(fileName);

	return 0;
}

void VideoPlayer::playFrame(int16 frame, int16 breakKey,
		uint16 palCmd, int16 palStart, int16 palEnd,
		int16 palFrame, int16 endFrame) {

	if (!_primaryVideo)
		return;

	Video &video = *_primaryVideo;
	Graphics::CoktelVideo &cVideo = *video.getVideo();

	if (cVideo.getCurrentFrame() != frame)
		cVideo.seekFrame(frame);
	if (palFrame < 0)
		palFrame = 0;
	if (endFrame < 0)
		endFrame = cVideo.getFramesCount() - 1;


	bool modifiedPal = false;

	if ((frame == palFrame) || ((frame == endFrame) && (palCmd == 8))) {
		modifiedPal = true;
		_vm->_draw->_applyPal = true;

		if (palCmd >= 4)
			copyPalette(cVideo, palStart, palEnd);
	}

	if (modifiedPal && (palCmd == 8) && !_backSurf)
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);


	if (_needBlit)
		_vm->_draw->forceBlit();

	Graphics::CoktelVideo::State state = video.nextFrame();
	WRITE_VAR(11, frame);

	if (_needBlit)
		_vm->_draw->forceBlit(true);


	if (modifiedPal && (palCmd == 16)) {
		if (_backSurf)
			_vm->_draw->forceBlit();
		_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, -2, 0);
		_vm->_draw->_noInvalidated = true;
		_vm->_video->dirtyRectsAll();
	}

	if ((state.flags & Graphics::CoktelVideo::kStatePalette) && (palCmd > 1)) {
		copyPalette(cVideo, palStart, palEnd);

		if (!_backSurf)
			_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
		else
			_vm->_draw->_applyPal = true;
	}

	if (modifiedPal && (palCmd == 8) && _backSurf)
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);


	if (!_ownSurf) {
		if (_backSurf) {
			_vm->_draw->invalidateRect(state.left, state.top, state.right, state.bottom);
			_vm->_draw->blitInvalidated();
		} else
			_vm->_video->dirtyRectsAdd(state.left, state.top, state.right, state.bottom);
		_vm->_video->retrace();
	}


	if (modifiedPal && ((palCmd == 2) || (palCmd == 4)))
		_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, -2, 0);
}

bool VideoPlayer::doPlay(int16 frame, int16 breakKey,
		uint16 palCmd, int16 palStart, int16 palEnd,
		int16 palFrame, int16 endFrame) {

	playFrame(frame, breakKey, palCmd, palStart, palEnd, palFrame, endFrame);

	_vm->_util->processInput();

	if (_vm->shouldQuit()) {
		_primaryVideo->getVideo()->disableSound();
		return true;
	}

	if (breakKey != 0) {
		_vm->_util->getMouseState(&_vm->_global->_inter_mouseX,
				&_vm->_global->_inter_mouseY, &_vm->_game->_mouseButtons);

		_vm->_inter->storeKey(_vm->_util->checkKey());
		if (VAR(0) == (unsigned) breakKey) {
			_primaryVideo->getVideo()->disableSound();
			return true;
		}
	}

	return false;
}

void VideoPlayer::copyPalette(Graphics::CoktelVideo &video, int16 palStart, int16 palEnd) {
	if (!(video.getFeatures() & Graphics::CoktelVideo::kFeaturesPalette))
		return;

	if (palStart < 0)
		palStart = 0;
	if (palEnd < 0)
		palEnd = 255;

	memcpy(((char *) (_vm->_global->_pPaletteDesc->vgaPal)) + palStart * 3,
			video.getPalette() + palStart * 3,
			(palEnd - palStart + 1) * 3);
}

void VideoPlayer::writeVideoInfo(const char *videoFile, int16 varX, int16 varY,
		int16 varFrames, int16 varWidth, int16 varHeight) {

	if (primaryOpen(videoFile)) {
		int16 x, y, width, height;

		x = _primaryVideo->getVideo()->getX();
		y = _primaryVideo->getVideo()->getY();
		width = _primaryVideo->getVideo()->getWidth();
		height = _primaryVideo->getVideo()->getHeight();

		if (VAR_OFFSET(varX) == 0xFFFFFFFF)
			_primaryVideo->getVideo()->getAnchor(1, 2, x, y, width, height);

		WRITE_VAR_OFFSET(varX, x);
		WRITE_VAR_OFFSET(varY, y);
		WRITE_VAR_OFFSET(varFrames, _primaryVideo->getVideo()->getFramesCount());
		WRITE_VAR_OFFSET(varWidth, width);
		WRITE_VAR_OFFSET(varHeight, height);

		primaryClose();
	} else {
		WRITE_VAR_OFFSET(varX, (uint32) -1);
		WRITE_VAR_OFFSET(varY, (uint32) -1);
		WRITE_VAR_OFFSET(varFrames, (uint32) -1);
		WRITE_VAR_OFFSET(varWidth, (uint32) -1);
		WRITE_VAR_OFFSET(varHeight, (uint32) -1);
	}
}

void VideoPlayer::evalBgShading(Graphics::CoktelVideo &video) {
	if (video.isSoundPlaying())
		_vm->_sound->bgShade();
	else
		_vm->_sound->bgUnshade();
}

void VideoPlayer::notifyPaused(uint32 duration) {
	if (_primaryVideo->isOpen())
		_primaryVideo->getVideo()->notifyPaused(duration);

	for (uint i = 0; i < _videoSlots.size(); i++)
		if (_videoSlots[i] && _videoSlots[i]->isOpen())
			_videoSlots[i]->getVideo()->notifyPaused(duration);
}

} // End of namespace Gob
