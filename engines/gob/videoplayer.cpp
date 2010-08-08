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
#include "gob/game.h"
#include "gob/palanim.h"
#include "gob/inter.h"
#include "gob/map.h"
#include "gob/sound/sound.h"

namespace Gob {

VideoPlayer::Properties::Properties() : type(kVideoTypeTry), sprite(Draw::kFrontSurface),
	x(-1), y(-1), width(-1), height(-1), flags(kFlagFrontSurface),
	startFrame(-1), lastFrame(-1), endFrame(-1), breakKey(kShortKeyEscape),
	palCmd(8), palStart(0), palEnd(255), palFrame(-1),
	fade(false), waitEndFrame(true), canceled(false) {

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
	surface.reset();
}


const char *VideoPlayer::_extensions[] = { "IMD", "IMD", "VMD", "RMD", "SMD" };

VideoPlayer::VideoPlayer(GobEngine *vm) : _vm(vm), _needBlit(false),
	_noCursorSwitch(false), _woodruffCohCottWorkaround(false) {
}

VideoPlayer::~VideoPlayer() {
	for (int i = 0; i < kVideoSlotCount; i++)
		_videoSlots[i].close();
}

void VideoPlayer::evaluateFlags(Properties &properties) {
	if        (properties.flags & kFlagFrontSurface) {
		properties.sprite = Draw::kFrontSurface;
	} else if (properties.flags & kFlagOtherSurface) {
		properties.sprite = properties.x;
		properties.x      = 0;
	} else if (properties.flags & kFlagScreenSurface) {
		properties.sprite = 0;
	} else {
		properties.sprite = Draw::kBackSurface;
	}
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

		// WORKAROUND: In some rare cases, the cursor should still be
		// displayed while a video is playing.
		_noCursorSwitch = false;
		if (primary && (_vm->getGameType() == kGameTypeLostInTime)) {
			if (!file.compareToIgnoreCase("PORTA03") ||
			    !file.compareToIgnoreCase("PORTA03A") ||
			    !file.compareToIgnoreCase("CALE1") ||
			    !file.compareToIgnoreCase("AMIL2") ||
			    !file.compareToIgnoreCase("AMIL3B") ||
			    !file.compareToIgnoreCase("DELB"))
				_noCursorSwitch = true;
		}

		// WORKAROUND: In Woodruff, Coh Cott vanished in one video on her party.
		// This is a bug in video, so we work around it.
		_woodruffCohCottWorkaround = false;
		if (primary && (_vm->getGameType() == kGameTypeWoodruff)) {
			if (!file.compareToIgnoreCase("SQ32-03"))
				_woodruffCohCottWorkaround = true;
		}

		if (!(properties.flags & kFlagNoVideo) && (properties.sprite >= 0)) {
			bool ownSurf    = (properties.sprite != Draw::kFrontSurface) && (properties.sprite != Draw::kBackSurface);
			bool screenSize = properties.flags & kFlagScreenSurface;

			if (ownSurf) {
				_vm->_draw->_spritesArray[properties.sprite] =
					_vm->_video->initSurfDesc(_vm->_global->_videoMode,
					                          screenSize ? _vm->_width  : video->decoder->getWidth(),
					                          screenSize ? _vm->_height : video->decoder->getHeight(), 0);
			}

			if (!_vm->_draw->_spritesArray[properties.sprite]) {
				properties.sprite = -1;
				video->surface.reset();
				video->decoder->setSurfaceMemory();
				video->decoder->setXY(0, 0);
			} else {
				video->surface = _vm->_draw->_spritesArray[properties.sprite];
				video->decoder->setSurfaceMemory(video->surface->getVidMem(),
						video->surface->getWidth(), video->surface->getHeight(), 1);

				if (!ownSurf || (ownSurf && screenSize)) {
					if ((properties.x >= 0) || (properties.y >= 0))
						video->decoder->setXY((properties.x < 0) ? 0xFFFF : properties.x,
						                      (properties.y < 0) ? 0xFFFF : properties.y);
					else
						video->decoder->setXY();
				} else
					video->decoder->setXY(0, 0);
			}

		} else {
			properties.sprite = -1;
			video->surface.reset();
			video->decoder->setSurfaceMemory();
			video->decoder->setXY(0, 0);
		}
	}

	if (primary)
		_needBlit = (properties.flags & kFlagUseBackSurfaceContent) && (properties.sprite == Draw::kFrontSurface);

	if (!video->decoder->hasSound())
		video->decoder->setFrameRate(_vm->_util->getFrameRate());

	WRITE_VAR(7, video->decoder->getFrameCount());

	return slot;
}

bool VideoPlayer::closeVideo(int slot) {
	Video *video = getVideoBySlot(slot);
	if (!video)
		return false;

	video->close();
	return true;
}

bool VideoPlayer::play(int slot, Properties &properties) {
	Video *video = getVideoBySlot(slot);
	if (!video)
		return false;

	bool primary = slot == 0;

	// NOTE: For testing (and comfort?) purposes, we enable aborting of all videos)
	properties.breakKey = kShortKeyEscape;

	if (properties.startFrame < 0)
		properties.startFrame = video->decoder->getCurFrame() + 1;
	if (properties.lastFrame  < 0)
		properties.lastFrame  = video->decoder->getFrameCount() - 1;
	if (properties.endFrame   < 0)
		properties.endFrame   = properties.lastFrame;
	if (properties.palFrame   < 0)
		properties.palFrame   = properties.startFrame;

	properties.startFrame--;
	properties.endFrame--;
	properties.palFrame--;

	if (primary) {
		_vm->_draw->_showCursor = _noCursorSwitch ? 3 : 0;

		if (properties.fade)
			_vm->_palAnim->fade(0, -2, 0);
	}

	bool backwards = properties.startFrame > properties.lastFrame;

	properties.canceled = false;

	while (properties.startFrame != properties.lastFrame) {
		playFrame(slot, properties);
		if (properties.canceled)
			break;

		properties.startFrame += backwards ? -1 : 1;

		evalBgShading(*video);

		if (primary && properties.fade) {
			_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, -2, 0);
			properties.fade = false;
		}

		if (!_noCursorSwitch && properties.waitEndFrame)
			waitEndFrame(slot);
	}

	evalBgShading(*video);

	return true;
}

void VideoPlayer::waitEndFrame(int slot) {
	Video *video = getVideoBySlot(slot);
	if (!video)
		return;

	_vm->_util->delay(video->decoder->getTimeToNextFrame());
}

bool VideoPlayer::playFrame(int slot, Properties &properties) {
	Video *video = getVideoBySlot(slot);
	if (!video)
		return false;

	bool primary = slot == 0;

	if (video->decoder->getCurFrame() != properties.startFrame) {
		video->decoder->disableSound();
		video->decoder->seek(properties.startFrame + 1, SEEK_SET, true);
		video->decoder->enableSound();
	}

	bool modifiedPal = false;

	if (primary) {
		// Pre-decoding palette and blitting, only for primary videos

		if ((properties.startFrame == properties.palFrame) ||
		    ((properties.startFrame == properties.endFrame) && (properties.palCmd == 8))) {

			modifiedPal = true;
			_vm->_draw->_applyPal = true;

			if (properties.palCmd >= 4)
				copyPalette(*video, properties.palStart, properties.palEnd);
		}

		if (modifiedPal && (properties.palCmd == 8) && (video->surface != _vm->_draw->_backSurface))
			_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);

		if (_needBlit)
			_vm->_draw->forceBlit();
	}


	Graphics::Surface *surface = video->decoder->decodeNextFrame();
	if (!surface)
		return false;

	WRITE_VAR(11, video->decoder->getCurFrame());

	if (_woodruffCohCottWorkaround && (properties.startFrame == 31)) {
		// WORKAROUND: This frame mistakenly masks Coh Cott, making her vanish
		// To prevent that, we'll never draw that part
		// state.left += 50;
	}

	if (primary) {
		// Post-decoding palette and blitting, only for primary videos

		if (_needBlit)
			_vm->_draw->forceBlit(true);

		if (modifiedPal && (properties.palCmd == 16)) {
			if (video->surface == _vm->_draw->_backSurface)
				_vm->_draw->forceBlit();
			_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, -2, 0);
			_vm->_draw->_noInvalidated = true;
			_vm->_video->dirtyRectsAll();
		}

		if (video->decoder->hasPalette() && (properties.palCmd > 1)) {
			copyPalette(*video, properties.palStart, properties.palEnd);

			if (video->surface != _vm->_draw->_backSurface)
				_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
			else
				_vm->_draw->_applyPal = true;
		}

		const Common::List<Common::Rect> &dirtyRects = video->decoder->getDirtyRects();

		if (modifiedPal && (properties.palCmd == 8) && (video->surface == _vm->_draw->_backSurface))
			_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);

		if (video->surface == _vm->_draw->_backSurface) {

			for (Common::List<Common::Rect>::const_iterator rect = dirtyRects.begin(); rect != dirtyRects.end(); ++rect)
				_vm->_draw->invalidateRect(rect->left, rect->top, rect->right - 1, rect->bottom - 1);
			_vm->_draw->blitInvalidated();

			// if (!noRetrace)
				_vm->_video->retrace();

		} else if (video->surface == _vm->_draw->_frontSurface) {
			for (Common::List<Common::Rect>::const_iterator rect = dirtyRects.begin(); rect != dirtyRects.end(); ++rect)
				_vm->_video->dirtyRectsAdd(rect->left, rect->top, rect->right - 1, rect->bottom - 1);

			// if (!noRetrace)
				_vm->_video->retrace();
		}

		/*
		// Subtitle
		if (state.flags & Graphics::CoktelDecoder::kStateSpeech)
			_vm->_draw->printTotText(state.speechId);
		*/

		if (modifiedPal && ((properties.palCmd == 2) || (properties.palCmd == 4)))
			_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, -2, 0);
	}

	if (primary && properties.waitEndFrame)
		checkAbort(*video, properties);

	return true;
}

void VideoPlayer::checkAbort(Video &video, Properties &properties) {
	_vm->_util->processInput();

	if (_vm->shouldQuit()) {
		video.decoder->disableSound();

		properties.canceled = true;
		return;
	}

	if (properties.breakKey != 0) {
		_vm->_util->getMouseState(&_vm->_global->_inter_mouseX,
				&_vm->_global->_inter_mouseY, &_vm->_game->_mouseButtons);

		_vm->_inter->storeKey(_vm->_util->checkKey());
		if (VAR(0) == (unsigned) properties.breakKey) {
			video.decoder->disableSound();

			// Seek to the last frame. Some scripts depend on that.
			video.decoder->seek(properties.endFrame + 1, SEEK_SET, true);

			properties.canceled = true;
		}
	}
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

	return video->decoder->getDefaultX();
}

uint16 VideoPlayer::getDefaultY(int slot) const {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return 0;

	return video->decoder->getDefaultY();
}

const Common::List<Common::Rect> *VideoPlayer::getDirtyRects(int slot) const {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return 0;

	return &video->decoder->getDirtyRects();
}

bool VideoPlayer::hasEmbeddedFile(const Common::String &fileName, int slot) const {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return false;

	return video->decoder->hasEmbeddedFile(fileName);
}

Common::MemoryReadStream *VideoPlayer::getEmbeddedFile(const Common::String &fileName, int slot) {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return 0;

	return video->decoder->getEmbeddedFile(fileName);
}

void VideoPlayer::writeVideoInfo(const Common::String &file, int16 varX, int16 varY,
		int16 varFrames, int16 varWidth, int16 varHeight) {

	Properties properties;

	int slot = openVideo(false, file, properties);
	if (slot >= 0) {
		Video &video = _videoSlots[slot];

		int16 x = -1, y = -1, width = -1, height = -1;

		x     = video.decoder->getDefaultX();
		y     = video.decoder->getDefaultY();
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

void VideoPlayer::evalBgShading(Video &video) {
	if (video.decoder->isSoundPlaying())
		_vm->_sound->bgShade();
	else
		_vm->_sound->bgUnshade();
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
		video = new Graphics::IMDDecoder(*_vm->_mixer, Audio::Mixer::kSFXSoundType);
	else if (properties.type == kVideoTypePreIMD)
		video = new Graphics::PreIMDDecoder(properties.width, properties.height, *_vm->_mixer, Audio::Mixer::kSFXSoundType);
	else if (properties.type == kVideoTypeVMD)
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

	properties.width  = video->getWidth();
	properties.height = video->getHeight();

	return video;
}




// Obsolete, to be deleted

bool VideoPlayer::primaryOpen(const char *videoFile, int16 x, int16 y,
		int32 flags, Type which, int16 width, int16 height) {

	return false;
}

bool VideoPlayer::primaryPlay(int16 startFrame, int16 lastFrame, int16 breakKey,
		uint16 palCmd, int16 palStart, int16 palEnd,
		int16 palFrame, int16 endFrame, bool fade, int16 reverseTo, bool forceSeek) {

	return false;
}

void VideoPlayer::primaryClose() {
}

int VideoPlayer::slotOpen(const char *videoFile, Type which, int16 width, int16 height) {
	return -1;
}

void VideoPlayer::slotPlay(int slot, int16 frame) {
}

void VideoPlayer::slotClose(int slot) {
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

void VideoPlayer::copyPalette(const Video &video, int16 palStart, int16 palEnd) {
	if (!video.decoder->hasPalette())
		return;

	if (palStart < 0)
		palStart = 0;
	if (palEnd < 0)
		palEnd = 255;

	memcpy(((char *)(_vm->_global->_pPaletteDesc->vgaPal)) + palStart * 3,
			video.decoder->getPalette() + palStart * 3, (palEnd - palStart + 1) * 3);
}

} // End of namespace Gob
