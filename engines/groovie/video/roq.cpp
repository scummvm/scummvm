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

// ROQ video player based on this specification by Dr. Tim Ferguson:
// https://multimedia.cx/mirror/idroq.txt

#include "groovie/video/roq.h"
#include "groovie/graphics.h"
#include "groovie/groovie.h"

#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/rect.h"
#include "common/substream.h"
#include "common/textconsole.h"

#include "image/jpeg.h"

// Required for the YUV to RGB conversion
#include "graphics/conversion.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"

#ifdef USE_MPEG2
#include "video/mpegps_decoder.h"
#endif

#include "common/file.h"
#ifdef USE_PNG
#include "image/png.h"
#else
#include "image/bmp.h"
#endif

/* copied from graphics/blit.h */
#ifdef SCUMM_LITTLE_ENDIAN
static const int kAIndex = 0;
static const int kBIndex = 1;
static const int kGIndex = 2;
static const int kRIndex = 3;

#else
static const int kAIndex = 3;
static const int kBIndex = 2;
static const int kGIndex = 1;
static const int kRIndex = 0;
#endif

namespace Groovie {

// Overwrites one pixel of destination regardless of the alpha value
static inline void copyPixel(byte *dst, const byte *src) {
	*(uint32 *)dst = *(const uint32 *)src;
}

// Overwrites one pixel of destination if the src pixel is visible
static inline void copyPixelIfAlpha(byte *dst, const byte *src) {
	if (src[kAIndex] > 0) {
		copyPixel(dst, src);
	}
}

// Overwrites one pixel if it's part of the mask
static inline void copyPixelIfMask(byte *dst, const byte *mask, const byte *src) {
	if (mask[kAIndex] > 0) {
		copyPixel(dst, src);
	}
}

// Copies one pixel to destination but respects the alpha value of the source
static inline void copyPixelWithA(byte *dst, const byte *src) {
	if (src[kAIndex] == 255) {
		copyPixel(dst, src);
	} else if (src[kAIndex] > 0) {
		dst[kAIndex] = MAX(src[kAIndex], dst[kAIndex]);
		dst[kRIndex] = ((src[kRIndex] * src[kAIndex]) + dst[kRIndex] * (255 - src[kAIndex])) >> 8;
		dst[kGIndex] = ((src[kGIndex] * src[kAIndex]) + dst[kGIndex] * (255 - src[kAIndex])) >> 8;
		dst[kBIndex] = ((src[kBIndex] * src[kAIndex]) + dst[kBIndex] * (255 - src[kAIndex])) >> 8;
	}
	// In case of alpha == 0 just do not copy
}


ROQPlayer::ROQPlayer(GroovieEngine *vm) :
	VideoPlayer(vm), _codingTypeCount(0),
	_bg(&_vm->_graphicsMan->_foreground),
	_screen(&_vm->_graphicsMan->_background),
	_firstFrame(true),
	_origX(0), _origY(0) {

	// Create the work surfaces
	_currBuf = new Graphics::Surface();
	_prevBuf = new Graphics::Surface();
	_overBuf = new Graphics::Surface();	// Overlay buffer. Objects move behind this layer

	// Allocate new buffers
	_currBuf->create(_bg->w, _bg->h, _vm->_pixelFormat);
	_prevBuf->create(_bg->w, _bg->h, _vm->_pixelFormat);
	_overBuf->create(_bg->w, _bg->h, _vm->_pixelFormat);
	_scaleX = MIN(_syst->getWidth() / _bg->w, 2);
	_scaleY = MIN(_syst->getHeight() / _bg->h, 2);

	_restoreArea = new Common::Rect();

	_videoDecoder = nullptr;
}

ROQPlayer::~ROQPlayer() {
	// Free the buffers
	_currBuf->free();
	delete _currBuf;
	_prevBuf->free();
	delete _prevBuf;
	_overBuf->free();
	delete _overBuf;
	delete _restoreArea;
}

void ROQPlayer::setOrigin(int16 x, int16 y) {
	_origX = x;
	_origY = y;
}

void ROQPlayer::stopAudioStream() {
	if (_audioStream) {
		g_system->getMixer()->stopHandle(_soundHandle);
	}
	_audioStream = nullptr;
}

uint16 ROQPlayer::loadInternal() {
	if (DebugMan.isDebugChannelEnabled(kDebugVideo)) {
		int8 i;
		debugCN(1, kDebugVideo, "Groovie::ROQ: Loading video. New ROQ: bitflags are ");
		for (i = 15; i >= 0; i--) {
			debugCN(1, kDebugVideo, "%d", _flags & (1 << i) ? 1 : 0);
			if (i % 4 == 0) {
				debugCN(1, kDebugVideo, " ");
			}
		}
		debugC(1, kDebugVideo, " <- 0 ");
	}

	// Flags:
	// - 2 For overlay videos, show the whole video
	// - 14 Manual flag indication alternate motion copy decoder
	bool oldOverlay = _flagOverlay;
	_flagNoPlay = ((_flags & (1 << 1)) != 0);
	_flagOverlay = ((_flags & (1 << 2)) != 0);
	_altMotionDecoder = ((_flags & (1 << 14)) != 0);
	_flagMasked = ((_flags & (1 << 10)) != 0);
	bool flagBricks = ((_flags & 1) != 0);

	if (gDebugLevel >= 8 && DebugMan.isDebugChannelEnabled(kDebugVideo)) {
		dumpAllSurfaces("loadInternal");
	}

	if (!_flagOverlay && _flagNoPlay) {
		// Clandestiny's bricks puzzle needs this copy to bg
		if (oldOverlay && _overBuf->w && flagBricks)
			_bg->copyFrom(*_overBuf);
		clearOverlay();
	}

	// Read the file header
	ROQBlockHeader blockHeader;
	if (!readBlockHeader(blockHeader)) {
		return 0;
	}

	debugC(6, kDebugVideo, "Groovie::ROQ: First Block type = 0x%02X", blockHeader.type);
	debugC(6, kDebugVideo, "Groovie::ROQ: First Block size = 0x%08X", blockHeader.size);
	debugC(6, kDebugVideo, "Groovie::ROQ: First Block param = 0x%04X", blockHeader.param);

	// Verify the file signature
#ifdef USE_MPEG2
	if (blockHeader.type == 0) {
		_videoDecoder = new Video::MPEGPSDecoder();
		_videoDecoder->setSoundType(Audio::Mixer::kSFXSoundType);
		_videoDecoder->loadStream(_file);

		_videoDecoder->start();

		_isFileHandled = true;
		return 24;
	}

	delete _videoDecoder;
	_videoDecoder = nullptr;
	_isFileHandled = false;
#endif

	if (blockHeader.type != 0x1084) {
		return 0;
	}

	// Clear the dirty flag and restore area
	_dirty = false;
	_restoreArea->top = 9999;
	_restoreArea->left = 9999;
	_restoreArea->bottom = 0;
	_restoreArea->right = 0;

	// Reset the codebooks
	_num2blocks = 0;
	_num4blocks = 0;

	// Reset the first frame flag
	_firstFrame = true;

	if ((blockHeader.size == 0) && (blockHeader.param == 0)) {
		// Set the offset scaling to 2
		_offScale = 2;

		// Hardcoded FPS
		return 30;
	} else if (blockHeader.size == (uint32)-1 || blockHeader.size == 0) {
		// Set the offset scaling to 1
		_offScale = 1;

		// In this case the block parameter is the framerate
		return blockHeader.param;
	} else {
		warning("Groovie::ROQ: Invalid header with size=%d and param=%d", blockHeader.size, blockHeader.param);
		return 0;
	}
}

void ROQPlayer::waitFrame() {
#ifdef USE_MPEG2
	if (_videoDecoder) {
		uint32 wait = _videoDecoder->getTimeToNextFrame();
		_syst->delayMillis(wait);
	} else
#endif
		VideoPlayer::waitFrame();
}

void ROQPlayer::clearOverlay() {
	debugC(1, kDebugVideo, "Groovie::ROQ: Clear overlay buffer");
	if (gDebugLevel >= 8 && DebugMan.isDebugChannelEnabled(kDebugVideo)) {
		dumpAllSurfaces("clearOverlay");
	}
	if (_overBuf->w) {
		_overBuf->fillRect(Common::Rect(0, 0, _overBuf->w, _overBuf->h), _overBuf->format.ARGBToColor(0, 0, 0, 0));
	}
}

// Calculate the overlapping area for the rendered frame to the visible frame. The game can use an origin to
// place the rendered image at different places.
void ROQPlayer::calcStartStop(int &start, int &stop, int origin, int length) {
	if (origin >= 0) {
		start = origin;
		stop = length;
	} else {
		start = 0;
		stop = length + origin;
	}
}

void ROQPlayer::redrawRestoreArea(int screenOffset, bool force) {
	// Restore the background by data from the foreground. Only restore the area which was overwritten during the last frame
	// Therefore we have the _restoreArea which reduces the area for restoring. We also use the _prevBuf to only overwrite the
	// Pixels which have been written during the last frame. This means _restoreArea is just an optimization.
	if (force) {
		_restoreArea->top = 0;
		_restoreArea->left = 0;
		_restoreArea->bottom = _screen->h;
		_restoreArea->right = _screen->w;
	}
	if (_restoreArea->isEmpty())
		return;

	int width = _restoreArea->right - _restoreArea->left;
	Graphics::Surface *screen = _vm->_system->lockScreen();
	assert(screen->format == _bg->format);
	assert(screen->format.bytesPerPixel == 4);
	for (int line = _restoreArea->top; line < _restoreArea->bottom; line++) {
		byte *dst = (byte *)screen->getBasePtr(_restoreArea->left, line + screenOffset);
		byte *src = (byte *)_bg->getBasePtr(_restoreArea->left, line);
		byte *prv = (byte *)_prevBuf->getBasePtr((_restoreArea->left - _origX) / _scaleX, (line - _origY) / _scaleY);
		byte *ovr = (byte *)_overBuf->getBasePtr(_restoreArea->left, line);

		for (int i = 0; i < width; i++) {
			if (prv[kAIndex] != 0 || force) {
				copyPixel(dst, src);
				copyPixelWithA(dst, ovr);
			}
			src += _bg->format.bytesPerPixel;
			dst += _bg->format.bytesPerPixel;
			prv += _bg->format.bytesPerPixel;
			ovr += _bg->format.bytesPerPixel;
		}
	}
	_vm->_system->unlockScreen();

	// Reset _restoreArea for the next frame
	_restoreArea->top = 9999;
	_restoreArea->left = 9999;
	_restoreArea->bottom = 0;
	_restoreArea->right = 0;
}

void writeImage(const Common::String filename, Graphics::Surface &surface) {
	if (surface.h == 0 || surface.w == 0) {
		return;
	}
	Common::String tname = "img/" + filename;
#ifdef USE_PNG
	tname += ".png";
#else
	tname += ".bmp";
#endif

	Common::DumpFile out;
	if (!out.open(Common::Path(tname))) {
		warning("failed to write debug image to %s", tname.c_str());
		return;
	}

#ifdef USE_PNG
	Image::writePNG(out, surface);
#else
	Image::writeBMP(out, surface);
#endif
}

void ROQPlayer::dumpAllSurfaces(const Common::String funcname) {
	TimeDate date;
	int curMonth;
	g_system->getTimeAndDate(date, true);
	curMonth = date.tm_mon + 1; // month is base 0, we need base 1 (1 = january and so on)
	uint millis = g_system->getMillis();
	Common::String timestamp = Common::String::format("%d-%02d-%02d %02d-%02d-%02d %08u",
													  date.tm_year + 1900, curMonth, date.tm_mday,
													  date.tm_hour, date.tm_min, date.tm_sec, millis);

	debugC(kDebugVideo, "%s %s dumpAllSurfaces", timestamp.c_str(), funcname.c_str());

	writeImage(timestamp + " lockScreen " + funcname, *_vm->_system->lockScreen());
	_vm->_system->unlockScreen();
	writeImage(timestamp + " _bg " + funcname, *_bg);
	writeImage(timestamp + " _currBuf " + funcname, *_currBuf);
	writeImage(timestamp + " _overBuf " + funcname, *_overBuf);
	writeImage(timestamp + " _prevBuf " + funcname, *_prevBuf);
	writeImage(timestamp + " _screen " + funcname, *_screen);
	while (g_system->getMillis() == millis) {
		g_system->delayMillis(1); // make sure we get a new timestamp every time
	}
}

void ROQPlayer::buildShowBuf() {
	// Calculate screen offset for normal / fullscreen videos and images
	int screenOffset = 0;
	if (_screen->h != 480) {
		screenOffset = 80;
	}

	if (_alpha) {
		redrawRestoreArea(screenOffset, false);
	}


	// Select the destination buffer according to the given flags
	int destOffset = 0;
	Graphics::Surface *maskBuf = nullptr;
	Graphics::Surface *srcBuf = _currBuf;
	Graphics::Surface *destBuf = nullptr;
	if (_flagMasked) {
		srcBuf = _bg;
		maskBuf = _currBuf;
	}
	if (_flagNoPlay) {
		if (_flagOverlay) {
			destBuf = _overBuf;
		} else {
			destBuf = _bg;
		}
	} else {
		destBuf = _vm->_system->lockScreen();
		destOffset = screenOffset;
	}


	// _origY and _origX may be negative (11th hour uses this in the chapel puzzle against Stauf)
	int startX, startY, stopX, stopY;
	calcStartStop(startX, stopX, _origX, _screen->w);
	calcStartStop(startY, stopY, _origY, _screen->h);
	assert(destBuf->format == srcBuf->format);
	assert(destBuf->format == _overBuf->format);
	assert(destBuf->format.bytesPerPixel == 4);

	for (int line = startY; line < stopY; line++) {
		byte *in = (byte *)srcBuf->getBasePtr(MAX(0, -_origX) / _scaleX, (line - _origY) / _scaleY);
		byte *inOvr = (byte *)_overBuf->getBasePtr(startX, line);
		byte *out = (byte *)destBuf->getBasePtr(startX, line + destOffset);
		byte *mask = nullptr;
		if (_flagMasked) {
			mask = (byte *)maskBuf->getBasePtr(MAX(0, -_origX) / _scaleX, (line - _origY) / _scaleY);
		}


		for (int x = startX; x < stopX; x++) {
			if (_flagMasked) {
				copyPixelIfMask(out, mask, in);
			} else if (destBuf == _overBuf) {
				copyPixelIfAlpha(out, in);
			} else {
				copyPixelWithA(out, in);
			}

			if (_alpha && in[kAIndex] != 0 && destBuf != _overBuf) {
				_restoreArea->top = MIN(line, (int)_restoreArea->top);
				_restoreArea->left = MIN(x, (int)_restoreArea->left);
				_restoreArea->bottom = MAX(line + 1, (int)_restoreArea->bottom);
				_restoreArea->right = MAX(x + 1, (int)_restoreArea->right);

				copyPixelWithA(out, inOvr);
			}

			// Skip to the next pixel
			out += _screen->format.bytesPerPixel;
			inOvr += _screen->format.bytesPerPixel;
			if (!(x % _scaleX))
				in += _screen->format.bytesPerPixel;
			if (mask)
				mask += _screen->format.bytesPerPixel;
		}
	}

	if (!_flagNoPlay) {
		_vm->_system->unlockScreen();
		_vm->_system->updateScreen();
	}
	_dirty = false;

	if (gDebugLevel >= 9 && DebugMan.isDebugChannelEnabled(kDebugVideo)) {
		dumpAllSurfaces("buildShowBuf");
	}

	// On the first frame, copy from the current buffer to the prev buffer
	if (_firstFrame) {
		_prevBuf->copyFrom(*_currBuf);
		_firstFrame = false;
	}

	// Swap buffers
	SWAP(_prevBuf, _currBuf);
}

bool ROQPlayer::playFrameInternal() {
	debugC(5, kDebugVideo, "Groovie::ROQ: Playing frame");

#ifdef USE_MPEG2
	if (_videoDecoder) {
		if (!_videoDecoder->needsUpdate())
			return false;	// Video has not yet ended

		const Graphics::Surface *srcSurf = _videoDecoder->decodeNextFrame();
		_currBuf->free();
		delete _currBuf;
		_currBuf = new Graphics::Surface();
		if (srcSurf) {
			_currBuf->copyFrom(*srcSurf);
			buildShowBuf();
		}
		return _videoDecoder->endOfVideo();
	}
#endif

	// Process the needed blocks until the next video frame
	bool endframe = false;
	while (!endframe && !_file->eos()) {
		endframe = processBlock();
	}

	if (_dirty) {
		// Build the show buffer from the current buffer
		buildShowBuf();
	}

	// Wait until the current frame can be shown
	// Don't wait if we're just showing one frame
	if (!playFirstFrame())
		waitFrame();

	if (_dirty) {
		// TODO: Update the screen
		void *src = /*(_alpha && 0)*/ false ? _bg->getPixels() : _screen->getPixels();
		_syst->copyRectToScreen(src, _screen->pitch, 0, (_syst->getHeight() - _screen->h) / 2, _screen->w, _screen->h);
		_syst->updateScreen();

		// TODO: For overlay videos, set the background buffer when the video ends
		/*if (_alpha && (!_flagOverlay || _file->eos())) {
			_bg->copyFrom(*_screen);
		}*/

		// Clear the dirty flag
		_dirty = false;
	}

	// Report the end of the video if we reached the end of the file or if we
	// just wanted to play one frame. Also reset origin for next video / image
	if (_file->eos() || playFirstFrame()) {
		_origX = _origY = 0;
		return true;
	}
	return false;
}

bool ROQPlayer::readBlockHeader(ROQBlockHeader &blockHeader) {
	if (_file->eos()) {
		return false;
	} else {
		blockHeader.type = _file->readUint16LE();
		blockHeader.size = _file->readUint32LE();
		blockHeader.param = _file->readUint16LE();

		debugC(10, kDebugVideo, "Groovie::ROQ: Block type = 0x%02X", blockHeader.type);
		debugC(10, kDebugVideo, "Groovie::ROQ: Block size = 0x%08X", blockHeader.size);
		debugC(10, kDebugVideo, "Groovie::ROQ: Block param = 0x%04X", blockHeader.param);

		return true;
	}
}

bool ROQPlayer::processBlock() {
	// Read the header of the block
	ROQBlockHeader blockHeader;
	if (!readBlockHeader(blockHeader)) {
		return true;
	}

	// Calculate where the block should end
	int64 endpos = _file->pos() + blockHeader.size;

	// Detect the end of the video
	if (_file->eos()) {
		return false;
	}

	bool ok = true;
	bool endframe = false;
	switch (blockHeader.type) {
	case 0x1001: // Video info
		ok = processBlockInfo(blockHeader);
		break;
	case 0x1002: // Quad codebook definition
		ok = processBlockQuadCodebook(blockHeader);
		break;
	case 0x1011: // Quad vector quantised video frame
		ok = processBlockQuadVector(blockHeader);
		_dirty = true;
		endframe = true;
		debugC(3, kDebugVideo, "Groovie::ROQ:   Decoded Quad Vector frame.");
		break;
	case 0x1012: // Still image (JPEG)
		ok = processBlockStill(blockHeader);
		_dirty = true;
		endframe = true;
		debugC(3, kDebugVideo, "Groovie::ROQ:   Decoded Still image (JPG).");
		break;
	case 0x1013: // Hang
		assert(blockHeader.size == 0 && blockHeader.param == 0);
		endframe = true;
		break;
	case 0x1020: // Mono sound samples
		ok = processBlockSoundMono(blockHeader);
		break;
	case 0x1021: // Stereo sound samples
		ok = processBlockSoundStereo(blockHeader);
		break;
	case 0x1030: // Container Chunk -> Just skip the chunk header and continue
		endpos = _file->pos();
		ok = processBlockAudioContainer(blockHeader);
		break;
	default:
		warning("Groovie::ROQ: Unknown block type: 0x%04X", blockHeader.type);
		ok = false;
		_file->skip(blockHeader.size);
	}

	if (endpos != _file->pos() && !_file->eos()) {
		warning("Groovie::ROQ: BLOCK %04x Should have ended at %lld, and has ended at %lld", blockHeader.type, (long long)endpos, (long long)_file->pos());
		warning("Ensure you've copied the files correctly according to the wiki.");
		_file->seek(MIN(_file->pos(), endpos));
	}
	// End the frame when the graphics have been modified or when there's an error
	return endframe || !ok;
}

bool ROQPlayer::processBlockInfo(ROQBlockHeader &blockHeader) {
	debugC(5, kDebugVideo, "Groovie::ROQ: Processing info block");

	// Verify the block header
	if (blockHeader.type != 0x1001 || blockHeader.size != 8 || (blockHeader.param != 0 && blockHeader.param != 1)) {
		warning("Groovie::ROQ: BlockInfo size=%d param=%d", blockHeader.size, blockHeader.param);
		return false;
	}

	// Reset the first frame flag
	_firstFrame = true;

	// Save the alpha channel size
	_alpha = blockHeader.param;

	// Read the information
	uint16 width = _file->readUint16LE();
	uint16 height = _file->readUint16LE();
	uint16 unk1 = _file->readUint16LE();
	uint16 unk2 = _file->readUint16LE();
	if (unk1 != 8 || unk2 != 4) {
		warning("Groovie::ROQ: unk1 = %d, unk2 = %d", unk1, unk2);
		return false;
	}

	// If the size of the image has changed, resize the buffers
	if ((width != _currBuf->w) || (height != _currBuf->h)) {
		// Calculate the maximum scale that fits the screen
		_scaleX = MIN(_syst->getWidth() / width, 2);
		_scaleY = MIN(_syst->getHeight() / height, 2);

		// Free the previous surfaces
		_currBuf->free();
		_prevBuf->free();
		_overBuf->free();

		// Allocate new buffers
		_currBuf->create(width, height, _vm->_pixelFormat);
		_prevBuf->create(width, height, _vm->_pixelFormat);
		_overBuf->create(width, height, _vm->_pixelFormat);
	}

	// Hack: Detect a video with interlaced black lines, by checking its height compared to width
	_interlacedVideo = 0;
	if (height <= width / 3) {
		_interlacedVideo = 1;
		_offScale = 2;
	}
	debugC(2, kDebugVideo, "Groovie::ROQ: width=%d, height=%d, scaleX=%d, scaleY=%d, _offScale=%d, interl.=%d, _alpha=%d", width, height, _scaleX, _scaleY, _interlacedVideo, _offScale, _alpha);

	// Switch from/to fullscreen, if needed
	if (_screen->h != 480 && height * _scaleY == 480)
		_vm->_graphicsMan->switchToFullScreen(true);
	else if (_screen->h == 480 && height * _scaleY != 480)
		_vm->_graphicsMan->switchToFullScreen(false);

	// TODO: Clear the buffers with black
	if (!_alpha && 0) {
		_currBuf->fillRect(Common::Rect(width, height), _vm->_pixelFormat.RGBToColor(0, 0, 0));
		_prevBuf->fillRect(Common::Rect(width, height), _vm->_pixelFormat.RGBToColor(0, 0, 0));
	}

	return true;
}

bool ROQPlayer::processBlockQuadCodebook(ROQBlockHeader &blockHeader) {
	debugC(5, kDebugVideo, "Groovie::ROQ: Processing quad codebook block");

	// Get the number of 2x2 pixel blocks to read
	int newNum2blocks = blockHeader.param >> 8;
	if (newNum2blocks == 0) {
		newNum2blocks = 256;
	}
	if (newNum2blocks > _num2blocks)
		_num2blocks = newNum2blocks;

	// Get the number of 4x4 pixel blocks
	_num4blocks = blockHeader.param & 0xFF;
	if ((_num4blocks == 0) && (blockHeader.size > (uint32)_num2blocks * (6 + _alpha * 4))) {
		_num4blocks = 256;
	}

	// Read the 2x2 codebook
	uint32 *codebook = _codebook2;

	for (int i = 0; i < newNum2blocks; i++) {
		// Read the 4 Y components and their alpha channel
		byte y[4];
		byte a[4];

		for (int j = 0; j < 4; j++) {
			y[j] = _file->readByte();
			a[j] = _alpha ? _file->readByte() : 255;
		}

		// Read the subsampled Cb and Cr
		byte u = _file->readByte();
		byte v = _file->readByte();

		// Convert the codebook to RGB right here
		for (int j = 0; j < 4; j++) {
			byte r, g, b;
			Graphics::YUV2RGB(y[j], u, v, r, g, b);
			*codebook++ = _vm->_pixelFormat.ARGBToColor(a[j], r, g, b);
		}
	}

	// Read the 4x4 codebook
	_file->read(_codebook4, _num4blocks * 4);

	return true;
}

bool ROQPlayer::processBlockQuadVector(ROQBlockHeader &blockHeader) {
	debugC(5, kDebugVideo, "Groovie::ROQ: Processing quad vector block");

	// Get the mean motion vectors
	_motionOffX = blockHeader.param >> 8;
	_motionOffY = blockHeader.param & 0xFF;

	// Calculate where the block should end
	int64 endpos =_file->pos() + blockHeader.size;

	// Reset the coding types
	_codingTypeCount = 0;

	// Traverse the image in 16x16 macroblocks
	for (int macroY = 0; macroY < _currBuf->h; macroY += 16) {
		for (int macroX = 0; macroX < _currBuf->w; macroX += 16) {
			// Traverse the macroblock in 8x8 blocks
			for (int blockY = 0; blockY < 16; blockY += 8) {
				for (int blockX = 0; blockX < 16; blockX += 8) {
					processBlockQuadVectorBlock(macroX + blockX, macroY + blockY);
				}
			}
		}
	}

	// HACK: Skip the remaining bytes
	int64 skipBytes = endpos -_file->pos();
	if (skipBytes > 0) {
		if (_file->eos()) {
			return false;
		}
		_file->skip(skipBytes);
		if (skipBytes != 2) {
			warning("Groovie::ROQ: Skipped %lld bytes", (long long)skipBytes);
		}
	}
	return true;
}

void ROQPlayer::processBlockQuadVectorBlock(int baseX, int baseY) {
	uint16 codingType = getCodingType();
	switch (codingType) {
	case 0: // MOT: Skip block
		break;
	case 1: { // FCC: Copy an existing block
		byte argument = _file->readByte();
		int16 dx = 8 - (argument >> 4);
		int16 dy = 8 - (argument & 0x0F);
		copy(8, baseX, baseY, dx, dy);
		break;
	}
	case 2: // SLD: Quad vector quantisation
		// Upsample the 4x4 pixel block
		paint8(_file->readByte(), baseX, baseY);
		break;
	case 3: // CCC:
		// Traverse the block in 4x4 sub-blocks
		for (int subBlockY = 0; subBlockY < 8; subBlockY += 4) {
			for (int subBlockX = 0; subBlockX < 8; subBlockX += 4) {
				processBlockQuadVectorBlockSub(baseX + subBlockX, baseY + subBlockY);
			}
		}
		break;
	default:
		break;
	}
}

void ROQPlayer::processBlockQuadVectorBlockSub(int baseX, int baseY) {
	debugC(6, kDebugVideo, "Groovie::ROQ: Processing quad vector sub block");

	uint16 codingType = getCodingType();
	switch (codingType) {
	case 0: // MOT: Skip block
		break;
	case 1: { // FCC: Copy an existing block
		byte argument = _file->readByte();
		int16 dx = 8 - (argument >> 4);
		int16 dy = 8 - (argument & 0x0F);
		copy(4, baseX, baseY, dx, dy);
		break;
	}
	case 2: // SLD: Quad vector quantisation
		paint4(_file->readByte(), baseX, baseY);
		break;
	case 3:
		paint2(_file->readByte(), baseX    , baseY);
		paint2(_file->readByte(), baseX + 2, baseY);
		paint2(_file->readByte(), baseX    , baseY + 2);
		paint2(_file->readByte(), baseX + 2, baseY + 2);
		break;
	default:
		break;
	}
}

bool ROQPlayer::processBlockStill(ROQBlockHeader &blockHeader) {
	debugC(5, kDebugVideo, "Groovie::ROQ: Processing still (JPEG) block");

	Image::JPEGDecoder jpg;
	jpg.setOutputPixelFormat(_vm->_pixelFormat);

	uint32 startPos = _file->pos();
	Common::SeekableSubReadStream subStream(_file, startPos, startPos + blockHeader.size, DisposeAfterUse::NO);
	jpg.loadStream(subStream);

	const Graphics::Surface *srcSurf = jpg.getSurface();
	_currBuf->free();
	delete _currBuf;

	_currBuf = new Graphics::Surface();
	_currBuf->copyFrom(*srcSurf);

	_file->seek(startPos + blockHeader.size);
	return true;
}

bool ROQPlayer::processBlockSoundMono(ROQBlockHeader &blockHeader) {
	debugC(5, kDebugVideo, "Groovie::ROQ: Processing mono sound block");

	// Verify the block header
	if (blockHeader.type != 0x1020) {
		return false;
	}

	// Initialize the audio stream if needed
	if (!_audioStream && !playFirstFrame()) {
		createAudioStream(false);
	}

	// Create the audio buffer
	int16 *buffer = (int16 *)malloc(blockHeader.size * 2);

	// Initialize the prediction with the block parameter
	int16 prediction = blockHeader.param ^ 0x8000;

	// Process the data
	for (uint16 i = 0; i < blockHeader.size; i++) {
		int16 data = _file->readByte();
		if (data < 0x80) {
			prediction += data * data;
		} else {
			data -= 0x80;
			prediction -= data * data;
		}
		buffer[i] = prediction;
	}

	// Queue the read buffer
	byte flags = Audio::FLAG_16BITS;
#ifdef SCUMM_LITTLE_ENDIAN
	flags |= Audio::FLAG_LITTLE_ENDIAN;
#endif
	if (!playFirstFrame() && !isFastForwarding())
		_audioStream->queueBuffer((byte *)buffer, blockHeader.size * 2, DisposeAfterUse::YES, flags);
	else
		free(buffer);

	return true;
}

bool ROQPlayer::processBlockSoundStereo(ROQBlockHeader &blockHeader) {
	debugC(5, kDebugVideo, "Groovie::ROQ: Processing stereo sound block");

	// Verify the block header
	if (blockHeader.type != 0x1021) {
		return false;
	}

	// Initialize the audio stream if needed
	if (!_audioStream && !playFirstFrame()) {
		createAudioStream(true);
	}

	// Create the audio buffer
	int16 *buffer = (int16 *)malloc(blockHeader.size * 2);

	// Initialize the prediction with the block parameter
	int16 predictionLeft = (blockHeader.param & 0xFF00) ^ 0x8000;
	int16 predictionRight = (blockHeader.param << 8) ^ 0x8000;
	bool left = true;

	// Process the data
	for (uint16 i = 0; i < blockHeader.size; i++) {
		int16 data = _file->readByte();
		if (left) {
			if (data < 0x80) {
				predictionLeft += data * data;
			} else {
				data -= 0x80;
				predictionLeft -= data * data;
			}
			buffer[i] = predictionLeft;
		} else {
			if (data < 0x80) {
				predictionRight += data * data;
			} else {
				data -= 0x80;
				predictionRight -= data * data;
			}
			buffer[i] = predictionRight;
		}
		left = !left;
	}

	// Queue the read buffer
	byte flags = Audio::FLAG_16BITS | Audio::FLAG_STEREO;
#ifdef SCUMM_LITTLE_ENDIAN
	flags |= Audio::FLAG_LITTLE_ENDIAN;
#endif
	if (!playFirstFrame() && !isFastForwarding())
		_audioStream->queueBuffer((byte *)buffer, blockHeader.size * 2, DisposeAfterUse::YES, flags);
	else
		free(buffer);

	return true;
}

bool ROQPlayer::processBlockAudioContainer(ROQBlockHeader &blockHeader) {
	debugC(5, kDebugVideo, "Groovie::ROQ: Processing audio container block: 0x%04X", blockHeader.param);
	return true;
}

byte ROQPlayer::getCodingType() {
	_codingType <<= 2;
	if (!_codingTypeCount) {
		_codingType = _file->readUint16LE();
		_codingTypeCount = 8;
	}

	_codingTypeCount--;
	return (_codingType >> 14);
}

void ROQPlayer::paint2(byte i, int destx, int desty) {
	if (i > _num2blocks) {
		warning("Groovie::ROQ: Invalid 2x2 block %d (%d available)", i, _num2blocks);
		return;
	}

	uint32 *block = &_codebook2[i * 4];
	uint32 *ptr = (uint32 *)_currBuf->getBasePtr(destx, desty);
	uint32 pitch = _currBuf->pitch / 4;

	ptr[0] = block[0];
	ptr[1] = block[1];
	ptr[pitch] = block[2];
	ptr[pitch + 1] = block[3];
}

void ROQPlayer::paint4(byte i, int destx, int desty) {
	if (i > _num4blocks) {
		warning("Groovie::ROQ: Invalid 4x4 block %d (%d available)", i, _num4blocks);
		return;
	}

	byte *block4 = &_codebook4[i * 4];
	for (int origy = 0; origy < 4; origy += 2) {
		for (int origx = 0; origx < 4; origx += 2) {
			paint2(*block4, destx + origx, desty + origy);
			block4++;
		}
	}
}

void ROQPlayer::paint8(byte i, int destx, int desty) {
	if (i > _num4blocks) {
		warning("Groovie::ROQ: Invalid 4x4 block %d (%d available)", i, _num4blocks);
		return;
	}

	byte *block4 = &_codebook4[i * 4];
	for (int y4 = 0; y4 < 2; y4++) {
		for (int x4 = 0; x4 < 2; x4++) {
			uint32 *block2 = &_codebook2[*block4++ * 4];

			for (int y2 = 0; y2 < 2; y2++) {
				for (int x2 = 0; x2 < 2; x2++) {
					uint32 *ptr = (uint32 *)_currBuf->getBasePtr(destx + x4 * 4 + x2 * 2, desty + y4 * 4 + y2 * 2);
					uint32 pitch = _currBuf->pitch / 4;
					uint32 color = *block2++;
					ptr[0] = ptr[1] = ptr[pitch] = ptr[pitch + 1] = color;
				}
			}
		}
	}
}

 void ROQPlayer::copy(byte size, int destx, int desty, int dx, int dy) {
	int offx = (dx - _motionOffX) * (_offScale / _scaleX);
	int offy = (dy - _motionOffY) * (_offScale / _scaleY);

	if (_altMotionDecoder) {
		offx *= 2;
		offy *= 2;
	}

	// Get the beginning of the first line
	byte *dst = (byte *)_currBuf->getBasePtr(destx, desty);
	byte *src = (byte *)_prevBuf->getBasePtr(destx + offx, desty + offy);

	for (int i = 0; i < size; i++) {
		// Copy the current line
		memcpy(dst, src, size * _currBuf->format.bytesPerPixel);

		// Move to the beginning of the next line
		dst += _currBuf->pitch;
		src += _prevBuf->pitch;
	}
}

void ROQPlayer::createAudioStream(bool stereo) {
	_audioStream = Audio::makeQueuingAudioStream(22050, stereo);
	g_system->getMixer()->playStream(Audio::Mixer::kSpeechSoundType, &_soundHandle, _audioStream);
}

void ROQPlayer::drawString(Graphics::Surface *surface, const Common::String text, int posx, int posy, uint32 color, bool blackBackground) {
	// TODO: fix redraw
#if 0
	int screenOffset = 0;
	if (_screen->h != 480) {
		screenOffset = 80;
	}
	Graphics::Surface *gamescreen = _vm->_system->lockScreen();
	Common::Rect rect(posx, posy - screenOffset, posx + _vm->_font->getMaxCharWidth()*15, posy + _vm->_font->getFontHeight()*2 - screenOffset);
	gamescreen->copyRectToSurface(*_bg, posx, posy, rect);
#endif

	if (blackBackground) {
		Common::Rect rect(posx - _vm->_font->getMaxCharWidth() * 0.3f, posy, posx + _vm->_font->getMaxCharWidth() * 15.3f, posy + _vm->_font->getFontHeight() * 1.3f);
		surface->fillRect(rect, surface->format.ARGBToColor(255, 0, 0, 0));
	}
	_vm->_font->drawString(surface, text.c_str(), posx, posy, surface->w, color, Graphics::kTextAlignLeft);
	_vm->_graphicsMan->change(); // Force Update screen after step
}

void ROQPlayer::copyfgtobg(uint8 arg) {
	// TODO: the arg isn't handled yet
	// but since we're doing a full redraw of all layers we might not need to care about the arg
	debugC(1, kDebugVideo, "Groovie::ROQ: copyfgtobg (0x%02X)", arg);

	redrawRestoreArea(_screen->h == 480 ? 0 : 80, true);
	_screen->copyFrom(*_bg);
	_vm->_system->updateScreen();
	clearOverlay();
}

ROQSoundPlayer::ROQSoundPlayer(GroovieEngine *vm) : ROQPlayer(vm) {
	// HACK: we set the pixel format here to prevent a crash because this never plays any videos
	// maybe we should just pre-create these buffers no matter what
	_overBuf->free();
	_overBuf->create(640, 480, _vm->_pixelFormat);
}

ROQSoundPlayer::~ROQSoundPlayer() {
}

void ROQSoundPlayer::createAudioStream(bool stereo) {
	_audioStream = Audio::makeQueuingAudioStream(22050, stereo);
	Audio::Mixer *mixer = g_system->getMixer();
	mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, _audioStream);
	mixer->setChannelVolume(_soundHandle, 100);
}

} // End of Groovie namespace
