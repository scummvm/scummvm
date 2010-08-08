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

VideoPlayer::Properties::Properties() : type(kVideoTypeTry), sprite(20),
	x(-1), y(-1), width(-1), height(-1), flags(kFlagFrontSurface),
	startFrame(-1), lastFrame(-1), breakKey(kShortKeyEscape),
	palCmd(8), palStart(0), palEnd(255), palFrame(-1), fade(false) {

}


VideoPlayer::Video::Video() : decoder(0) {
}

bool VideoPlayer::Video::isEmpty() const {
	return decoder == 0;
}

void VideoPlayer::Video::close() {
	delete decoder;

	decoder = 0;
	fileName.clear();
}


const char *VideoPlayer::_extensions[] = { "IMD", "IMD", "VMD", "RMD", "SMD" };

VideoPlayer::VideoPlayer(GobEngine *vm) : _vm(vm),
	_ownSurf(false), _backSurf(false), _needBlit(false),
	_noCursorSwitch(false), _woodruffCohCottWorkaround(false) {
}

VideoPlayer::~VideoPlayer() {
	for (int i = 0; i < kVideoSlotCount; i++)
		_videoSlots[i].close();
}

int VideoPlayer::openVideo(bool primary, const Common::String &file, Properties &properties) {
	int slot = 0;

	Video *video = 0;
	if (!primary) {
		slot = getNextFreeSlot();
		if (slot < 0) {
			warning("VideoPlayer::openVideo(): Can't open video \"%s\": No free slot", file.c_str());
			return -1;
		}

		video = &_videoSlots[slot];
	} else
		video = &_videoSlots[0];

	// Different video already in the slot => close that video
	if (!video->isEmpty() && (video->fileName.compareToIgnoreCase(file) != 0))
		video->close();

	// No video => load the requested file
	if (video->isEmpty()) {
		// Open the video
		if (!(video->decoder = openVideo(file, properties)))
			return -1;

		// Set the filename
		video->fileName = file;
	}

	// TODO

	return -1; // slot
}

bool VideoPlayer::closeVideo(int slot) {
	Video *video = getVideoBySlot(slot);
	if (!video)
		return false;

	video->close();
	return true;
}

bool VideoPlayer::play(int slot, const Properties &properties) {
	Video *video = getVideoBySlot(slot);
	if (!video)
		return false;

	// TODO

	return false;
}

bool VideoPlayer::slotIsOpen(int slot) const {
	return getVideoBySlot(slot) != 0;
}

Common::String VideoPlayer::getFileName(int slot) const {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return "";

	return video->fileName;
}

uint16 VideoPlayer::getFlags(int slot) const {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return 0;

	return 0; // video->decoder->getFlags();
}

uint32 VideoPlayer::getFrameCount(int slot) const {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return 0;

	return video->decoder->getFrameCount();
}

uint32 VideoPlayer::getCurrentFrame(int slot) const {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return 0;

	return video->decoder->getCurFrame();
}

uint16 VideoPlayer::getWidth(int slot) const {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return 0;

	return video->decoder->getWidth();
}

uint16 VideoPlayer::getHeight(int slot) const {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return 0;

	return video->decoder->getHeight();
}

uint16 VideoPlayer::getDefaultX(int slot) const {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return 0;

	return 0; // video->decoder->getDefaultX();
}

uint16 VideoPlayer::getDefaultY(int slot) const {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return 0;

	return 0; // video->decoder->getDefaultY();
}

uint32 VideoPlayer::getFeatures(int slot) const {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return 0;

	return 0; // video->decoder->getFeatures();
}

void VideoPlayer::getState(int slot) const {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return;

	return; // video->decoder->getState();
}

bool VideoPlayer::hasExtraData(const Common::String &fileName, int slot) const {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return false;

	return false; // video->decoder->hasExtraData(fileName);
}

Common::MemoryReadStream *VideoPlayer::getExtraData(const Common::String &fileName, int slot) {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return 0;

	return 0; // video->decoder->getExtraData(fileName);
}

const VideoPlayer::Video *VideoPlayer::getVideoBySlot(int slot) const {
	if ((slot < 0) || (slot >= kVideoSlotCount))
		return 0;

	if (_videoSlots[slot].isEmpty())
		return 0;

	return &_videoSlots[slot];
}

VideoPlayer::Video *VideoPlayer::getVideoBySlot(int slot) {
	if ((slot < 0) || (slot >= kVideoSlotCount))
		return 0;

	if (_videoSlots[slot].isEmpty())
		return 0;

	return &_videoSlots[slot];
}

int VideoPlayer::getNextFreeSlot() {
	// Starting with 1, since 0 is reserved for the "primary" video
	for (int i = 1; i < kVideoSlotCount; i++)
		if (_videoSlots[i].isEmpty())
			return i;

	return -1;
}

Common::String VideoPlayer::findFile(const Common::String &file, Properties &properties) {

	bool hasExtension = false;

	Common::String base     = file;
	Common::String fileName = file;

	const char *posDot = strrchr(base.c_str(), '.');
	if (posDot) {
		hasExtension = true;
		base = Common::String(base.c_str(), posDot);
		posDot++;
	}

	if (hasExtension) {
		int i;
		for (i = 0; i < ARRAYSIZE(_extensions); i++) {
			if (!scumm_stricmp(posDot, _extensions[i])) {
				if ((properties.type != kVideoTypeTry) && (properties.type == ((Type) i))) {
					warning("Attempted to open video \"%s\", but requested a different type", fileName.c_str());
					return "";
				}
				properties.type = (Type) i;
				break;
			}
		}
		if (i >= ARRAYSIZE(_extensions))
			hasExtension = false;
	}

	if (!hasExtension) {
		// No or unrecognized extension. Probing.

		int i;
		for (i = 0; i < ARRAYSIZE(_extensions); i++) {
			if ((properties.type == kVideoTypeTry) || (properties.type == ((Type) i))) {
				fileName = base + "." + _extensions[i];

				if (_vm->_dataIO->existData(fileName.c_str())) {
					properties.type = (Type) i;
					break;
				}
			}
		}
		if ((i >= ARRAYSIZE(_extensions)) || (properties.type == kVideoTypeTry)) {
			warning("Couldn't open video \"%s\"", file.c_str());
			return "";
		}

	}

	return fileName;
}

Graphics::CoktelDecoder *VideoPlayer::openVideo(const Common::String &file, Properties &properties) {
	Common::String fileName = findFile(file, properties);
	if (fileName.empty())
		return 0;

	Common::SeekableReadStream *stream = _vm->_dataIO->getDataStream(fileName.c_str());
	if (!stream)
		return 0;

	Graphics::CoktelDecoder *video = 0;
	if (properties.type == kVideoTypeIMD)
		warning("TODO: IMD");
		//_video = new Graphics::Imd();
	else if (properties.type == kVideoTypePreIMD) {
		warning("PreIMDDecoder \"%s\" %dx%d", fileName.c_str(), properties.width, properties.height);
		video = new Graphics::PreIMDDecoder(properties.width, properties.height, *_vm->_mixer, Audio::Mixer::kSFXSoundType);
	} else if (properties.type == kVideoTypeVMD)
		warning("TODO: VMD");
		//_video = new Graphics::Vmd(_vm->_video->_palLUT);
	else if (properties.type == kVideoTypeRMD)
		warning("TODO: RMD");
		//_video = new Graphics::Vmd(_vm->_video->_palLUT);
	else
		warning("Couldn't open video \"%s\": Invalid video Type", fileName.c_str());

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




// Obsolete, to be deleted

bool VideoPlayer::primaryOpen(const char *videoFile, int16 x, int16 y,
		int32 flags, Type which, int16 width, int16 height) {

	return false;

#if 0
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

#endif

}

bool VideoPlayer::primaryPlay(int16 startFrame, int16 lastFrame, int16 breakKey,
		uint16 palCmd, int16 palStart, int16 palEnd,
		int16 palFrame, int16 endFrame, bool fade, int16 reverseTo, bool forceSeek) {

	return false;

#if 0

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

#endif
}

void VideoPlayer::primaryClose() {
#if 0
	delete _primaryVideo;
	_primaryVideo = 0;

	_primaryFileName.clear();
#endif
}

int VideoPlayer::slotOpen(const char *videoFile, Type which, int16 width, int16 height) {
	return -1;

#if 0

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
#endif
}

void VideoPlayer::slotPlay(int slot, int16 frame) {
#if 0
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
#endif
}

void VideoPlayer::slotClose(int slot) {
#if 0
	if ((slot < 0) || (slot >= kVideoSlotCount) || !_videoSlots[slot])
		return;

	delete _videoSlots[slot];
	_videoSlots[slot] = 0;
#endif
}

void VideoPlayer::slotCopyFrame(int slot, byte *dest,
		uint16 left, uint16 top, uint16 width, uint16 height,
		uint16 x, uint16 y, uint16 pitch, int16 transp) {

#if 0
	if ((slot < 0) || (slot >= kVideoSlotCount) || !_videoSlots[slot])
		return;

	/*_videoSlots[slot]->getVideo()->copyCurrentFrame(dest,
			left, top, width, height, x, y, pitch, transp);*/
#endif
}

void VideoPlayer::slotCopyPalette(int slot, int16 palStart, int16 palEnd) {
#if 0
	if ((slot < 0) || (slot >= kVideoSlotCount) || !_videoSlots[slot])
		return;

	//copyPalette(*(_videoSlots[slot]->getVideo()), palStart, palEnd);
#endif
}

void VideoPlayer::slotWaitEndFrame(int slot, bool onlySound) {
#if 0
	Graphics::CoktelDecoder *video = getVideoBySlot(slot);

	if (video) {
		/*
		Graphics::CoktelDecoder &cVideo = *video->getVideo();

		if (!onlySound || (cVideo.getFeatures() & Graphics::CoktelDecoder::kFeaturesSound))
			cVideo.waitEndFrame();
		*/
	}
#endif
}

void VideoPlayer::slotSetDoubleMode(int slot, bool doubleMode) {
}




void VideoPlayer::playFrame(int16 frame, int16 breakKey,
		uint16 palCmd, int16 palStart, int16 palEnd,
		int16 palFrame, int16 endFrame, bool noRetrace) {

#if 0
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
#endif
}

bool VideoPlayer::doPlay(int16 frame, int16 breakKey,
		uint16 palCmd, int16 palStart, int16 palEnd,
		int16 palFrame, int16 endFrame, bool noRetrace) {

	return false;
#if 0

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
#endif
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

void VideoPlayer::writeVideoInfo(const Common::String &file, int16 varX, int16 varY,
		int16 varFrames, int16 varWidth, int16 varHeight) {

	Properties properties;

	int slot = openVideo(false, file, properties);
	if (slot >= 0) {
		Video &video = _videoSlots[slot];

		int16 x = -1, y = -1, width = -1, height = -1;

		// x     = video.decoder->getX();
		// y     = video.decoder->getY();
		width = video.decoder->getWidth();
		width = video.decoder->getHeight();

		/*
		if (VAR_OFFSET(varX) == 0xFFFFFFFF)
			video.decoder->getFrameCoords(1, x, y, width, height);
		*/

		WRITE_VAR_OFFSET(varX     , x);
		WRITE_VAR_OFFSET(varY     , y);
		WRITE_VAR_OFFSET(varFrames, video.decoder->getFrameCount());
		WRITE_VAR_OFFSET(varWidth , width);
		WRITE_VAR_OFFSET(varHeight, height);

	} else {
		WRITE_VAR_OFFSET(varX     , (uint32) -1);
		WRITE_VAR_OFFSET(varY     , (uint32) -1);
		WRITE_VAR_OFFSET(varFrames, (uint32) -1);
		WRITE_VAR_OFFSET(varWidth , (uint32) -1);
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
