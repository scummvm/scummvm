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
#include "gob/dataio.h"
#include "gob/video.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/palanim.h"
#include "gob/inter.h"
#include "gob/map.h"
#include "gob/sound/sound.h"

namespace Gob {

const char *VideoPlayer::_extensions[] = { "IMD", "IMD", "VMD", "RMD", "SMD" };

VideoPlayer::VideoPlayer(GobEngine *vm) : _vm(vm), _primaryVideo(0),
	_ownSurf(false), _backSurf(false), _needBlit(false),
	_noCursorSwitch(false), _woodruffCohCottWorkaround(false) {

	for (int i = 0; i < kVideoSlotCount; i++)
		_videoSlots[i] = 0;
}

VideoPlayer::~VideoPlayer() {
	delete _primaryVideo;
	for (int i = 0; i < kVideoSlotCount; i++)
		delete _videoSlots[i];
}

bool VideoPlayer::findFile(char *fileName, Type &which) {
	char *extStart = strrchr(fileName, '.');
	// There's no empty extension, Or the filename with its current extension is not found
	if ((extStart) && ((extStart == (fileName + strlen(fileName) - 1)) || (!_vm->_dataIO->existData(fileName)))) {
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
				fileName[len] = '.';
				fileName[len + 1] = 0;
				strcat(fileName, _extensions[i]);

				if (_vm->_dataIO->existData(fileName)) {
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

Graphics::CoktelDecoder *VideoPlayer::openVideo(const char *fileName, Type which, uint16 width, uint16 height) {
	Common::SeekableReadStream *stream = _vm->_dataIO->getDataStream(fileName);
	if (!stream)
		return 0;

	Graphics::CoktelDecoder *video = 0;
	if (which == kVideoTypeIMD)
		warning("TODO: IMD");
		//_video = new Graphics::Imd();
	else if (which == kVideoTypePreIMD)
		video = new Graphics::PreIMDDecoder(width, height, *_vm->_mixer, Audio::Mixer::kSFXSoundType);
	else if (which == kVideoTypeVMD)
		warning("TODO: VMD");
		//_video = new Graphics::Vmd(_vm->_video->_palLUT);
	else if (which == kVideoTypeRMD)
		warning("TODO: RMD");
		//_video = new Graphics::Vmd(_vm->_video->_palLUT);
	else
		warning("Couldn't open video \"%s\": Invalid video Type", fileName);

	if (!video) {
		delete stream;
		return 0;
	}

	if (!video->load(*stream)) {
		delete video;
		return 0;
	}

	return video;
}

bool VideoPlayer::primaryOpen(const char *videoFile, int16 x, int16 y,
		int32 flags, Type which, int16 width, int16 height) {

	char fileName[256];

	strncpy0(fileName, videoFile, 250);

	if (!findFile(fileName, which))
		return false;

	if (scumm_strnicmp(_primaryFileName.c_str(), fileName, strlen(fileName))) {
		primaryClose();

		if (!(_primaryVideo = openVideo(fileName, which, (uint16) width, (uint16) height)))
			return false;

		_primaryFileName = fileName;

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

		// WORKAROUND: In Woodruff, Coh Cott vanished in one video on her party.
		// This is a bug in video, so we work around it.
		_woodruffCohCottWorkaround = false;
		if (_vm->getGameType() == kGameTypeWoodruff) {
			if (!scumm_stricmp(fileName, "SQ32-03.VMD"))
				_woodruffCohCottWorkaround = true;
		}

		_ownSurf = false;

		if (!(flags & kFlagNoVideo)) {
			SurfaceDescPtr surf;

			if (flags & kFlagOtherSurface) {
				_ownSurf = true;
				_backSurf = false;

				surf = _vm->_video->initSurfDesc(_vm->_global->_videoMode,
						_primaryVideo->getWidth(),
						_primaryVideo->getHeight(), 0);
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

			/*
			_primaryVideo->getVideo()->setVideoMemory(surf->getVidMem(),
					surf->getWidth(), surf->getHeight());
			*/

		} else
			;//_primaryVideo->getVideo()->setVideoMemory();

		_needBlit = ((flags & kFlagUseBackSurfaceContent) != 0) && ((flags & kFlagFrontSurface) != 0);
	}

	if (!_primaryVideo)
		return false;

	//_primaryVideo->getVideo()->setFrameRate(_vm->_util->getFrameRate());
	//_primaryVideo->getVideo()->setXY(x, y);

	WRITE_VAR(7, _primaryVideo->getFrameCount());

	return true;
}

bool VideoPlayer::primaryPlay(int16 startFrame, int16 lastFrame, int16 breakKey,
		uint16 palCmd, int16 palStart, int16 palEnd,
		int16 palFrame, int16 endFrame, bool fade, int16 reverseTo, bool forceSeek) {

	if (!_primaryVideo)
		return false;

	Graphics::CoktelDecoder &video = *_primaryVideo;

	breakKey = 27;
	if (startFrame < 0)
		startFrame = video.getCurFrame();
	if (lastFrame < 0)
		lastFrame = video.getFrameCount() - 1;
	if (palFrame < 0)
		palFrame = startFrame;
	if (endFrame < 0)
		endFrame = lastFrame;
	palCmd &= 0x3F;

	int16 realStartFrame = startFrame;
	/*
	if (video.getCurFrame() != startFrame) {
		if (!forceSeek && (video.getFeatures() & Graphics::CoktelDecoder::kFeaturesSound))
			startFrame = video.getCurFrame();
		else
			video.seek(startFrame);
	}
	*/ video.seek(startFrame);

	_vm->_draw->_showCursor = _noCursorSwitch ? 3 : 0;

	if (fade)
		_vm->_palAnim->fade(0, -2, 0);

	bool canceled = false;

	while (startFrame <= lastFrame) {
		if (doPlay(startFrame, breakKey,
					palCmd, palStart, palEnd, palFrame, endFrame, startFrame < realStartFrame)) {

			canceled = true;
			break;
		}

		evalBgShading(video);

		if (fade) {
			_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, -2, 0);
			fade = false;
		}

		/*if (!_noCursorSwitch)
			video.waitEndFrame();*/
		startFrame++;
	}

	evalBgShading(video);

	if (reverseTo >= 0) {
		int16 toFrame = video.getFrameCount() - reverseTo;
		for (int i = video.getCurFrame(); i >= toFrame; i--) {
			video.seek(i, SEEK_SET, true);

			bool b = doPlay(i, breakKey, 0, 0, 0, 0, 0);
			evalBgShading(video);

			if (b) {
				_vm->_palAnim->fade(0, -2, 0);
				memset((char *)_vm->_draw->_vgaPalette, 0, 768);
			}

			/*if (!_noCursorSwitch)
				video.waitEndFrame();*/
		}
	}

	evalBgShading(video);

	return canceled;
}

void VideoPlayer::primaryClose() {
	delete _primaryVideo;
	_primaryVideo = 0;

	_primaryFileName.clear();
}

int VideoPlayer::slotOpen(const char *videoFile, Type which, int16 width, int16 height) {
	int slot = getNextFreeSlot();
	if (slot == -1)
		return -1;

	Graphics::CoktelDecoder *&video = _videoSlots[slot];

	char fileName[256];

	strncpy0(fileName, videoFile, 250);

	if (!findFile(fileName, which)) {
		delete video;
		return -1;
	}

	if (!(video = openVideo(fileName, which, width, height)))
		return -1;

	//video->getVideo()->setVideoMemory();
	//video->getVideo()->enableSound(*_vm->_mixer);

	WRITE_VAR(7, video->getFrameCount());

	return slot;
}

int VideoPlayer::getNextFreeSlot() {
	for (int i = 0; i < kVideoSlotCount; i++)
		if (!_videoSlots[i])
			return i;

	warning("VideoPlayer::getNextFreeSlot(): No free video slot");
	return -1;
}

void VideoPlayer::slotPlay(int slot, int16 frame) {
	if ((slot < 0) || (slot >= kVideoSlotCount) || !_videoSlots[slot])
		return;

	Graphics::CoktelDecoder &video = *_videoSlots[slot];

	if (frame < 0)
		frame = video.getCurFrame();

	if (((uint16) frame) >= video.getFrameCount())
		return;

	if (video.getCurFrame() != frame)
		video.seek(frame);

	//_videoSlots[slot]->nextFrame();
	WRITE_VAR(11, frame);

	evalBgShading(video);
}

void VideoPlayer::slotClose(int slot) {
	if ((slot < 0) || (slot >= kVideoSlotCount) || !_videoSlots[slot])
		return;

	delete _videoSlots[slot];
	_videoSlots[slot] = 0;
}

void VideoPlayer::slotCopyFrame(int slot, byte *dest,
		uint16 left, uint16 top, uint16 width, uint16 height,
		uint16 x, uint16 y, uint16 pitch, int16 transp) {

	if ((slot < 0) || (slot >= kVideoSlotCount) || !_videoSlots[slot])
		return;

	/*_videoSlots[slot]->getVideo()->copyCurrentFrame(dest,
			left, top, width, height, x, y, pitch, transp);*/
}

void VideoPlayer::slotCopyPalette(int slot, int16 palStart, int16 palEnd) {
	if ((slot < 0) || (slot >= kVideoSlotCount) || !_videoSlots[slot])
		return;

	//copyPalette(*(_videoSlots[slot]->getVideo()), palStart, palEnd);
}

void VideoPlayer::slotWaitEndFrame(int slot, bool onlySound) {
	Graphics::CoktelDecoder *video = getVideoBySlot(slot);

	if (video) {
		/*
		Graphics::CoktelDecoder &cVideo = *video->getVideo();

		if (!onlySound || (cVideo.getFeatures() & Graphics::CoktelDecoder::kFeaturesSound))
			cVideo.waitEndFrame();
		*/
	}
}

bool VideoPlayer::slotIsOpen(int slot) const {
	if ((slot < 0) || (slot >= kVideoSlotCount) || !_videoSlots[slot])
		return true;

	return false;
}

void VideoPlayer::slotSetDoubleMode(int slot, bool doubleMode) {
	/*
	Graphics::CoktelDecoder *video = getVideoBySlot(slot);

	if (video)
		video->getVideo()->setDoubleMode(doubleMode);
	*/
}

const Graphics::CoktelDecoder *VideoPlayer::getVideoBySlot(int slot) const {
	if (slot < 0) {
		if (_primaryVideo)
			return _primaryVideo;
	} else if ((slot < kVideoSlotCount) && _videoSlots[slot])
		return _videoSlots[slot];

	return 0;
}

Graphics::CoktelDecoder *VideoPlayer::getVideoBySlot(int slot) {
	if (slot < 0) {
		if (_primaryVideo)
			return _primaryVideo;
	} else if ((slot < kVideoSlotCount) && _videoSlots[slot])
		return _videoSlots[slot];

	return 0;
}

const Common::String &VideoPlayer::getPrimaryFileName() const {
	return _primaryFileName;
}

uint16 VideoPlayer::getFlags(int slot) const {
	const Graphics::CoktelDecoder *video = getVideoBySlot(slot);

	if (video)
		return 0;//return video->getVideo()->getFlags();

	return 0;
}

int16 VideoPlayer::getFrameCount(int slot) const {
	const Graphics::CoktelDecoder *video = getVideoBySlot(slot);

	if (video)
		return video->getFrameCount();

	return 0;
}

int16 VideoPlayer::getCurrentFrame(int slot) const {
	const Graphics::CoktelDecoder *video = getVideoBySlot(slot);

	if (video)
		return video->getCurFrame();

	return 0;
}

int16 VideoPlayer::getWidth(int slot) const {
	const Graphics::CoktelDecoder *video = getVideoBySlot(slot);

	if (video)
		return video->getWidth();

	return 0;
}

int16 VideoPlayer::getHeight(int slot) const {
	const Graphics::CoktelDecoder *video = getVideoBySlot(slot);

	if (video)
		return video->getHeight();

	return 0;
}

int16 VideoPlayer::getDefaultX(int slot) const {
	const Graphics::CoktelDecoder *video = getVideoBySlot(slot);

	if (video)
		return 0;//return video->getDefaultX();

	return 0;
}

int16 VideoPlayer::getDefaultY(int slot) const {
	const Graphics::CoktelDecoder *video = getVideoBySlot(slot);

	if (video)
		return 0;//return video->getDefaultY();

	return 0;
}

uint32 VideoPlayer::getFeatures(int slot) const {
	const Graphics::CoktelDecoder *video = getVideoBySlot(slot);

	if (video)
		return 0;//return video->getFeatures();

	return 0;
}

void VideoPlayer::getState(int slot) const {
	/*
	const Video *video = getVideoBySlot(slot);
	Graphics::CoktelVideo::State state;

	if (video)
		state = video->getState();

	return state;
	*/
}

bool VideoPlayer::hasExtraData(const char *fileName, int slot) const {
	const Graphics::CoktelDecoder *video = getVideoBySlot(slot);

	if (video)
		return false;//return video->hasExtraData(fileName);

	return false;
}

Common::MemoryReadStream *VideoPlayer::getExtraData(const char *fileName, int slot) {
	Graphics::CoktelDecoder *video = getVideoBySlot(slot);

	if (video)
		return 0;//return video->getExtraData(fileName);

	return 0;
}

void VideoPlayer::playFrame(int16 frame, int16 breakKey,
		uint16 palCmd, int16 palStart, int16 palEnd,
		int16 palFrame, int16 endFrame, bool noRetrace) {

	if (!_primaryVideo)
		return;

	Graphics::CoktelDecoder &video = *_primaryVideo;

	if (video.getCurFrame() != frame)
		video.seek(frame);
	if (palFrame < 0)
		palFrame = 0;
	if (endFrame < 0)
		endFrame = video.getFrameCount() - 1;


	bool modifiedPal = false;

	if ((frame == palFrame) || ((frame == endFrame) && (palCmd == 8))) {
		modifiedPal = true;
		_vm->_draw->_applyPal = true;

		if (palCmd >= 4)
			copyPalette(video, palStart, palEnd);
	}

	if (modifiedPal && (palCmd == 8) && !_backSurf)
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);


	if (_needBlit)
		_vm->_draw->forceBlit();

	Graphics::CoktelDecoder::State state;// = video.nextFrame();
	WRITE_VAR(11, frame);

	if (_woodruffCohCottWorkaround && (frame == 32)) {
		// WORKAROUND: This frame mistakenly masks Coh Cott, making her vanish
		// To prevent that, we'll never draw that part
		state.left += 50;
	}

	if (_needBlit)
		_vm->_draw->forceBlit(true);


	if (modifiedPal && (palCmd == 16)) {
		if (_backSurf)
			_vm->_draw->forceBlit();
		_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, -2, 0);
		_vm->_draw->_noInvalidated = true;
		_vm->_video->dirtyRectsAll();
	}

	/*
	if ((state.flags & Graphics::CoktelDecoder::kStatePalette) && (palCmd > 1)) {
		copyPalette(video, palStart, palEnd);

		if (!_backSurf)
			_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
		else
			_vm->_draw->_applyPal = true;
	}
	*/

	if (modifiedPal && (palCmd == 8) && _backSurf)
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);


	if (!_ownSurf) {
		if (_backSurf) {
			_vm->_draw->invalidateRect(state.left, state.top, state.right, state.bottom);
			_vm->_draw->blitInvalidated();
		} else
			_vm->_video->dirtyRectsAdd(state.left, state.top, state.right, state.bottom);

		if (!noRetrace)
			_vm->_video->retrace();
	}

	/*
	// Subtitle
	if (state.flags & Graphics::CoktelDecoder::kStateSpeech)
		_vm->_draw->printTotText(state.speechId);
	*/

	if (modifiedPal && ((palCmd == 2) || (palCmd == 4)))
		_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, -2, 0);
}

bool VideoPlayer::doPlay(int16 frame, int16 breakKey,
		uint16 palCmd, int16 palStart, int16 palEnd,
		int16 palFrame, int16 endFrame, bool noRetrace) {

	playFrame(frame, breakKey, palCmd, palStart, palEnd, palFrame, endFrame, noRetrace);

	_vm->_util->processInput();

	if (_vm->shouldQuit()) {
		//_primaryVideo->getVideo()->disableSound();
		return true;
	}

	if (breakKey != 0) {
		_vm->_util->getMouseState(&_vm->_global->_inter_mouseX,
				&_vm->_global->_inter_mouseY, &_vm->_game->_mouseButtons);

		_vm->_inter->storeKey(_vm->_util->checkKey());
		if (VAR(0) == (unsigned) breakKey) {
			//_primaryVideo->getVideo()->disableSound();
			// Seek to the last frame. Some scripts depend on that.
			_primaryVideo->seek(endFrame, SEEK_SET, true);
			return true;
		}
	}

	return false;
}

void VideoPlayer::copyPalette(Graphics::CoktelDecoder &video, int16 palStart, int16 palEnd) {
	/*
	if (!(video.getFeatures() & Graphics::CoktelDecoder::kFeaturesPalette))
		return;
	*/

	if (palStart < 0)
		palStart = 0;
	if (palEnd < 0)
		palEnd = 255;

	/*
	memcpy(((char *)(_vm->_global->_pPaletteDesc->vgaPal)) + palStart * 3,
			video.getPalette() + palStart * 3,
			(palEnd - palStart + 1) * 3);
	*/
}

void VideoPlayer::writeVideoInfo(const char *videoFile, int16 varX, int16 varY,
		int16 varFrames, int16 varWidth, int16 varHeight) {

	if (primaryOpen(videoFile)) {
		int16 x = -1, y = -1, width = -1, height = -1;

		/*
		x = _primaryVideo->getVideo()->getX();
		y = _primaryVideo->getVideo()->getY();
		width = _primaryVideo->getVideo()->getWidth();
		height = _primaryVideo->getVideo()->getHeight();
		*/

		/*
		if (VAR_OFFSET(varX) == 0xFFFFFFFF)
			_primaryVideo->getVideo()->getFrameCoords(1, x, y, width, height);
		*/

		WRITE_VAR_OFFSET(varX, x);
		WRITE_VAR_OFFSET(varY, y);
		WRITE_VAR_OFFSET(varFrames, _primaryVideo->getFrameCount());
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

void VideoPlayer::evalBgShading(Graphics::CoktelDecoder &video) {
	/*
	if (video.isSoundPlaying())
		_vm->_sound->bgShade();
	else
		_vm->_sound->bgUnshade();
	*/
}

} // End of namespace Gob
