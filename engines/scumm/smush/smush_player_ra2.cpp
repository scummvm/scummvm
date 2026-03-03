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

// Rebel Assault 2 specific SmushPlayer methods
//
// These are methods of the SmushPlayer class that contain RA2-specific logic.
// Keeping them in a separate file minimizes the diff on smush_player.cpp and
// reduces the risk of regressions in Full Throttle / The Dig / CMI.

#include "common/endian.h"
#include "common/rect.h"
#include "common/system.h"

#include "scumm/scumm.h"
#include "scumm/scumm_v7.h"
#include "scumm/smush/smush_font.h"
#include "scumm/smush/smush_multi_font.h"
#include "scumm/smush/smush_player.h"

#include "scumm/insane/insane.h"
#include "scumm/insane/insane_rebel.h"

namespace Scumm {

bool SmushPlayer::isRA2() const {
	return _vm->_game.id == GID_REBEL2;
}

// Forward declarations for RA2 codec functions (defined in codec_ra2.cpp)
void smushDecodeLineUpdate(byte *dst, const byte *src, int left, int top, int width, int height, int pitch);
void smushDecodeSkipRLE(byte *dst, const byte *src, int left, int top, int width, int height, int pitch);
void smushDecodeRA2Bomp(byte *dst, const byte *src, int left, int top, int width, int height, int pitch, int dataSize);

/**
 * Initialize RA2-specific fields in the SmushPlayer constructor.
 */
void SmushPlayer::ra2InitFields() {
	_multiFont = nullptr;
	_storedFobjData = nullptr;
	_storedFobjDataSize = 0;
	_storedFobjCodec = 0;
	_storedFobjLeft = 0;
	_storedFobjTop = 0;
	_storedFobjWidth = 0;
	_storedFobjHeight = 0;
	_skipNext = false;
	_ra2FastForwarding = false;
	_fobjOffsetX = 0;
	_fobjOffsetY = 0;
	_storeFrame = false;
	_loadBuffer = nullptr;
	_loadBufferSize = 0;
	_loadBufferOffset = 0;
	_loadReadOffset = 8;  // Original starts reading at offset 8 (skips header)
	_lastLoadChunkIdx = -1;
	_totalLoadChunks = 0;
	_scrollX = 0;
	_scrollY = 0;
}

/**
 * Free RA2-specific resources in the SmushPlayer destructor.
 */
void SmushPlayer::ra2DestroyFields() {
	delete _multiFont;
	_multiFont = nullptr;
	free(_storedFobjData);
	_storedFobjData = nullptr;
	free(_loadBuffer);
	_loadBuffer = nullptr;
}

/**
 * RA2-specific initialization in SmushPlayer::init().
 * Re-pushes the SMUSH palette (videos without NPAL inherit from previous),
 * and handles background preservation between cinematic and gameplay videos.
 */
void SmushPlayer::ra2InitVideo() {
	// Re-push the SMUSH palette to the system. Videos like O_LEVEL.SAN
	// have no NPAL chunk and inherit the palette from the previous video.
	// Since play() resets _palDirtyMin/Max, the palette would never be pushed otherwise.
	setDirtyColors(0, 255);

	// Handle background preservation between videos:
	// - Cinematic videos (flags 0x20) clear the buffer for a fresh start
	// - Gameplay videos (flags 0x28) preserve the existing screen content
	if (_dst != nullptr) {
		VirtScreen *vs = &_vm->_virtscr[kMainVirtScreen];
		if ((_curVideoFlags & 0x08) == 0) {
			// Cinematic mode (flags 0x20) - clear buffer for fresh video
			memset(_dst, 0, vs->w * vs->h);
		} else {
			// Gameplay mode (flags 0x28) - do nothing, preserve existing screen content
			int nonZero = 0;
			for (int i = 0; i < vs->w * vs->h; i++) {
				if (_dst[i] != 0) nonZero++;
			}
			debug("SmushPlayer::init: Preserving screen for gameplay video (%dx%d, %d%% non-zero)",
				vs->w, vs->h, (nonZero * 100) / (vs->w * vs->h));
		}
	}
}

/**
 * RA2-specific cleanup in SmushPlayer::release().
 * Frees stored FOBJ data but preserves _frameBuffer across videos.
 */
void SmushPlayer::ra2ReleaseVideo() {
	free(_storedFobjData);
	_storedFobjData = nullptr;
	_storedFobjDataSize = 0;
	// Preserve _frameBuffer across videos so that gameplay videos (which have no
	// background FOBJ) can use the stored background from the previous BEG video.
}

/**
 * RA2-specific FTCH handling.
 * For Handler 25, skips FTCH to preserve overlays.
 * For other handlers, re-decodes stored FOBJ with current offsets.
 */
void SmushPlayer::ra2HandleFetch(Common::SeekableReadStream &b) {
	int16 ftchUnknown = b.readSint16LE();
	int16 ftchX = b.readSint16LE();
	int16 ftchY = b.readSint16LE();

	debug("SmushPlayer::handleFetch: frame=%d unknown=%d x=%d y=%d",
		_frame, ftchUnknown, ftchX, ftchY);

	// For Handler 25, skip FTCH because the frame buffer only contains the
	// par4=5 base background without the overlays (par4=4, 6, 7) that were drawn
	// immediately in frame 0. Restoring would erase those overlays.
	if (_insane != nullptr) {
		InsaneRebel2 *rebel2 = static_cast<InsaneRebel2 *>(_insane);
		int handler = rebel2->getHandler();
		if (handler == 25) {
			debug("SmushPlayer::handleFetch: Skipping FTCH for Handler 25 - preserving overlays");
			return;
		}
	}

	// Re-decode stored FOBJ data with current offsets (matching original FUN_004246d0).
	if (_storedFobjData != nullptr) {
		decodeFrameObject(_storedFobjCodec, _storedFobjData,
			_storedFobjLeft, _storedFobjTop,
			_storedFobjWidth, _storedFobjHeight,
			_storedFobjDataSize);
	}
}

/**
 * Handle LOAD chunk for Rebel Assault 2.
 *
 * LOAD chunks stream embedded resource data across multiple frames.
 * The data is accumulated in a buffer and consumed by the audio system.
 */
void SmushPlayer::handleLoad(int32 subSize, Common::SeekableReadStream &b) {
	debugC(DEBUG_SMUSH, "SmushPlayer::handleLoad()");

	if (subSize < 10) {
		warning("SmushPlayer::handleLoad: chunk too small (%d bytes)", subSize);
		return;
	}

	int16 totalChunks = b.readUint16LE();
	int16 chunkIndex = b.readUint16LE();
	b.skip(6);  // Unknown/padding

	int32 dataSize = subSize - 10;

	debugC(DEBUG_SMUSH, "SmushPlayer::handleLoad: chunk %d/%d, dataSize=%d, bufferOffset=%d",
		chunkIndex, totalChunks, dataSize, _loadBufferOffset);

	// First chunk in sequence - reset buffer state
	if (chunkIndex == 0) {
		_loadBufferOffset = 0;
		_loadReadOffset = 8;
		_lastLoadChunkIdx = -1;
		_totalLoadChunks = totalChunks;

		int32 estimatedSize = totalChunks * 600;
		if (_loadBuffer == nullptr || _loadBufferSize < estimatedSize) {
			free(_loadBuffer);
			_loadBufferSize = estimatedSize;
			_loadBuffer = (byte *)malloc(_loadBufferSize);
			if (_loadBuffer == nullptr) {
				warning("SmushPlayer::handleLoad: Failed to allocate %d bytes for LOAD buffer",
					_loadBufferSize);
				_loadBufferSize = 0;
				return;
			}
			debugC(DEBUG_SMUSH, "SmushPlayer::handleLoad: Allocated %d bytes for LOAD buffer",
				_loadBufferSize);
		}
	}

	// Check sequential order
	if (_lastLoadChunkIdx + 1 != chunkIndex) {
		debugC(DEBUG_SMUSH, "SmushPlayer::handleLoad: Non-sequential chunk %d (expected %d), skipping",
			chunkIndex, _lastLoadChunkIdx + 1);
		return;
	}

	// Check buffer capacity
	if (_loadBuffer == nullptr || _loadBufferOffset + dataSize >= _loadBufferSize) {
		warning("SmushPlayer::handleLoad: Buffer overflow - offset=%d size=%d limit=%d",
			_loadBufferOffset, dataSize, _loadBufferSize);
		return;
	}

	// Copy data to buffer
	b.read(_loadBuffer + _loadBufferOffset, dataSize);
	_loadBufferOffset += dataSize;
	_lastLoadChunkIdx = chunkIndex;

	debugC(DEBUG_SMUSH, "SmushPlayer::handleLoad: Accumulated %d bytes total", _loadBufferOffset);

	if (chunkIndex == totalChunks - 1) {
		debugC(DEBUG_SMUSH, "SmushPlayer::handleLoad: Sequence complete - %d chunks, %d bytes total",
			totalChunks, _loadBufferOffset);
	}
}

/**
 * RA2-specific text rendering using SmushMultiFont for inline font switching.
 */
void SmushPlayer::ra2HandleTextResource(const char *str, int fontId, int color,
										int pos_x, int pos_y, int left, int top,
										int width, int height, TextStyleFlags flg) {
	// Create multi-font renderer on first use
	if (!_multiFont) {
		_multiFont = new SmushMultiFont(_vm, this, true);
	}
	_multiFont->setDefaultFont(fontId);

	debug("SmushPlayer::handleTextResource: RA2 TRES frame=%d fontId=%d color=%d flags=0x%x flg=%d pos=(%d,%d) clip=(%d,%d,%d,%d) str=\"%.40s\"",
		  _frame, fontId, color, (int)flg, (int)flg, pos_x, pos_y, left, top, width, height, str);

	if (flg & kStyleWordWrap) {
		Common::Rect clipRect(MAX<int>(0, left), MAX<int>(0, top), MIN<int>(left + width, _width), MIN<int>(top + height, _height));
		_multiFont->drawStringWrap(str, _dst, clipRect, pos_x, pos_y, color, flg);
	} else {
		Common::Rect clipRect(0, 0, _width, _height);
		_multiFont->drawString(str, _dst, clipRect, pos_x, pos_y, color, flg);
	}
}

/**
 * RA2-specific buffer selection for non-standard FOBJ dimensions.
 * Returns the destination buffer to use and updates _dst, _width, _height.
 */
void SmushPlayer::ra2SelectFrameBuffer(int width, int height) {
	// Rebel2 uses a special buffer for all non-matching frames.
	// Level 1: First frame is 424x260 (background), small sprites reuse same buffer
	// Level 2: Uses virtual screen directly (handled below when _specialBuffer stays null)
	int bufSize = width * height;
	if (bufSize > _vm->_screenWidth * _vm->_screenHeight) {
		// Frame is larger than screen - need special buffer
		if (_specialBuffer == nullptr || bufSize > _specialBufferSize) {
			free(_specialBuffer);
			_specialBuffer = (byte *)malloc(bufSize);
			_specialBufferSize = bufSize;
			_width = width;
			_height = height;
		}
	}

	if (bufSize > _vm->_screenWidth * _vm->_screenHeight &&
	    _specialBuffer != nullptr && _specialBufferSize >= bufSize) {
		_dst = _specialBuffer;
		debug("SmushPlayer: Using _specialBuffer for oversized FOBJ %dx%d", width, height);
	} else {
		if (_specialBuffer == nullptr) {
			VirtScreen *vs = &_vm->_virtscr[kMainVirtScreen];
			_dst = vs->getPixels(0, 0);
			debug("SmushPlayer: Reset _dst to virtual screen for FOBJ %dx%d at (%d,%d) _dst=%p",
				width, height, 0, 0, (void*)_dst);
		} else {
			// Large frame was in this video, use _specialBuffer for compositing
			_dst = _specialBuffer;
			debug("SmushPlayer: Using _specialBuffer for small FOBJ %dx%d (compositing with large frame)",
				width, height);
		}
	}
}

/**
 * Apply RA2 FOBJ position offsets and clamp to buffer bounds.
 * Modifies left, top, width, height in place.
 * When srcSkipY is non-null, outputs the number of source rows to skip
 * when top is clipped from negative (for codecs with row-size prefixes).
 */
void SmushPlayer::ra2AdjustFrameCoords(int &left, int &top, int &width, int &height, int pitch, int *srcSkipY) {
	left += _fobjOffsetX;
	top += _fobjOffsetY;

	int bufHeight = (_dst == _specialBuffer) ? _height : _vm->_screenHeight;
	if (top < 0) {
		if (srcSkipY)
			*srcSkipY = -top;
		height += top;
		top = 0;
	}
	if (left < 0) {
		width += left;
		left = 0;
	}
	if (top + height > bufHeight) {
		height = bufHeight - top;
	}
	if (left + width > pitch) {
		width = pitch - left;
	}
}

/**
 * Dispatch to RA2-specific codec functions.
 * Returns true if the codec was handled, false for standard codecs.
 */
bool SmushPlayer::ra2DecodeCodec(int codec, const uint8 *src, int left, int top,
								 int width, int height, int pitch, int dataSize) {
	switch (codec) {
	case SMUSH_CODEC_LINE_UPDATE:
	case SMUSH_CODEC_LINE_UPDATE2:
		smushDecodeLineUpdate(_dst, src, left, top, width, height, pitch);
		return true;
	case SMUSH_CODEC_SKIP_RLE:
		smushDecodeSkipRLE(_dst, src, left, top, width, height, pitch);
		return true;
	case SMUSH_CODEC_RA2_BOMP:
		smushDecodeRA2Bomp(_dst, src, left, top, width, height, pitch, dataSize);
		return true;
	default:
		return false;
	}
}

/**
 * Save raw FOBJ data when STOR is pending (for later re-decoding by FTCH).
 */
void SmushPlayer::ra2StoreFobjData(int codec, const byte *data, int32 dataSize,
								   int left, int top, int width, int height) {
	free(_storedFobjData);
	_storedFobjData = (byte *)malloc(dataSize);
	memcpy(_storedFobjData, data, dataSize);
	_storedFobjDataSize = dataSize;
	_storedFobjCodec = codec;
	_storedFobjLeft = left;
	_storedFobjTop = top;
	_storedFobjWidth = width;
	_storedFobjHeight = height;
	_storeFrame = false;
}

/**
 * Reset XPAL delta palette from the current base palette.
 * Prevents stale delta values from a previous video corrupting the palette.
 */
void SmushPlayer::ra2ResetDeltaPalette() {
	for (int j = 0; j < 768; ++j) {
		_shiftedDeltaPal[j] = _pal[j] << 7;
	}
	memset(_deltaPal, 0, sizeof(_deltaPal));
}

/**
 * RA2 font path table.
 */
SmushFont *SmushPlayer::ra2GetFont(int font) {
	const char *ra2_fonts[] = {
		"SYSTM/TALKFONT.NUT",
		"SYSTM/DIHIFONT.NUT",
		"SYSTM/TITLFONT.NUT",
		"SYSTM/SMALFONT.NUT"
	};
	int numFonts = ARRAYSIZE(ra2_fonts);
	if (font >= 0 && font < numFonts) {
		_sf[font] = new SmushFont(_vm, ra2_fonts[font], true);
	} else {
		debugC(DEBUG_SMUSH, "SmushPlayer::getFont: RA2 unknown font %d, using TALKFONT", font);
		_sf[font] = new SmushFont(_vm, ra2_fonts[0], true);
	}
	return _sf[font];
}

/**
 * RA2 per-frame audio processing (called from parseNextFrame).
 */
void SmushPlayer::ra2ParseNextFrame() {
	// Call processDispatches directly since RA2 has no iMUSE
	// 11025 Hz / 12 fps = ~918 samples per frame
	processDispatches(_smushAudioSampleRate / 12);
}

/**
 * RA2-specific handleAnimHeader fixup: when AHDR reports 0x0 dimensions,
 * use screen dimensions instead.
 */
void SmushPlayer::ra2FixupAnimHeader() {
	if (_width == 0 && _height == 0) {
		_width = _vm->_screenWidth;   // 320
		_height = _vm->_screenHeight; // 200
		debug("SmushPlayer::handleAnimHeader: RA2 AHDR has 0x0 dims - using screen size %dx%d", _width, _height);
	}
}

// Masked region management — used by InsaneRebel2

void SmushPlayer::addMaskedRegion(const Common::Rect &rect) {
	for (Common::List<Common::Rect>::iterator it = _maskedRegions.begin(); it != _maskedRegions.end(); ++it) {
		if (*it == rect) {
			return; // Already exists
		}
	}
	_maskedRegions.push_back(rect);
}

void SmushPlayer::removeMaskedRegion(const Common::Rect &rect) {
	for (Common::List<Common::Rect>::iterator it = _maskedRegions.begin(); it != _maskedRegions.end(); ++it) {
		if (*it == rect) {
			_maskedRegions.erase(it);
			return;
		}
	}
}

void SmushPlayer::clearMaskedRegions() {
	_maskedRegions.clear();
}

void SmushPlayer::setScrollOffset(int x, int y) {
	_scrollX = x;
	_scrollY = y;
}

} // End of namespace Scumm
