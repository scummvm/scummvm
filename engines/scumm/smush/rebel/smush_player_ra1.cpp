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

// Rebel Assault 1 specific SmushPlayer methods.
//
// Keep these in a dedicated file so the shared smush_player.cpp stays close
// to upstream while RA1 behavior is isolated in one place.

#include "common/config-manager.h"
#include "common/endian.h"
#include "common/memstream.h"

#include "scumm/file.h"
#include "scumm/scumm_v7.h"
#include "scumm/smush/rebel/anim_ra1.h"
#include "scumm/smush/rebel/codec_ra1.h"
#include "scumm/smush/smush_font.h"
#include "scumm/smush/rebel/codec_ra2.h"
#include "scumm/smush/rebel/smush_player_ra1.h"

#include "scumm/insane/rebel1/rebel.h"

namespace Scumm {

enum {
	kRA1DecodeWidth = 384,
	kRA1DecodeHeight = 242,
	kRA1PresentationBorder = 4,
	kRA1PresentationScreenWidth = 320,
	kRA1PresentationScreenHeight = 200,
	kRA1PresentationWidth = kRA1PresentationScreenWidth - kRA1PresentationBorder * 2,
	kRA1PresentationHeight = kRA1PresentationScreenHeight - kRA1PresentationBorder * 2,
	kRebel1LongAudioTrackSize = 500000
};

static void ra1ApplyCenteredFetchPlacement(InsaneRebel1 *rebel1, int width, int height, int &left, int &top) {
	int16 centerX = (int16)(left + (width >> 1));
	int16 centerY = (int16)(top + (height >> 1));

	rebel1->projectGameplayPoint(centerX, centerY);

	const int projectedLeft = (int)centerX - (width >> 1);
	const int projectedTop = (int)centerY - (height >> 1);

	// RestoreStoredFramePatch routes FTCH through DispatchFobjCodec with flag 0x800.
	// That path applies ProjectPointToScreen() to the center point, then only moves
	// a quarter of the projected delta before decoding the stored FOBJ.
	left -= ((projectedLeft - left) >> 2);
	top -= ((projectedTop - top) >> 2);
}

static bool ra1EnsureBuffer(byte *&buffer, int32 &bufferSize, int32 neededSize) {
	if (neededSize <= 0)
		return false;
	if (buffer != nullptr && bufferSize >= neededSize)
		return true;

	byte *newBuffer = (byte *)realloc(buffer, neededSize);
	if (newBuffer == nullptr)
		return false;

	buffer = newBuffer;
	bufferSize = neededSize;
	return true;
}

static void ra1CopyFadeRun(byte *dst, const byte *src, int srcPitch, int width, int height,
		int dstPos, int srcPos, int count) {
	const int frameSize = width * height;
	if (dstPos < 0 || srcPos < 0 || count <= 0 || dstPos >= frameSize || srcPos >= frameSize)
		return;

	count = MIN(count, frameSize - dstPos);
	count = MIN(count, frameSize - srcPos);
	while (count > 0) {
		const int srcX = srcPos % width;
		const int dstX = dstPos % width;
		const int run = MIN(count, width - srcX);
		const int rowRun = MIN(run, width - dstX);
		memcpy(dst + dstPos, src + (srcPos / width) * srcPitch + srcX, rowRun);
		srcPos += rowRun;
		dstPos += rowRun;
		count -= rowRun;
	}
}

static void ra1RememberDisplayedFrame(byte *&buffer, int32 &bufferSize, int &storedWidth,
		int &storedHeight, bool &valid, const byte *src, int pitch, int width, int height) {
	if (src == nullptr || width <= 0 || height <= 0)
		return;

	const int32 neededSize = width * height;
	if (!ra1EnsureBuffer(buffer, bufferSize, neededSize))
		return;

	if (buffer == src && pitch == width) {
		// Already displaying the retained FADE buffer.
	} else if (pitch == width) {
		memcpy(buffer, src, neededSize);
	} else {
		for (int y = 0; y < height; ++y)
			memcpy(buffer + y * width, src + y * pitch, width);
	}

	storedWidth = width;
	storedHeight = height;
	valid = true;
}

SmushPlayerRebel1::SmushPlayerRebel1(ScummEngine_v7 *scumm, IMuseDigital *imuseDigital, Insane *insane)
	: SmushPlayer(scumm, imuseDigital, insane) {
	initGamePlayerFields();
	ra1InitAudioTrackSizes();
}

SmushPlayerRebel1::~SmushPlayerRebel1() {
	destroyGamePlayerFields();
}

void SmushPlayerRebel1::initGamePlayerFields() {
	_ra1CleanFrame = nullptr;
	_ra1CleanFrameSize = 0;
	_ra1HasCleanFrame = false;
	_ra1PresentationBuffer = nullptr;
	_ra1PresentationBufferSize = 0;
	_ra1ObjOverlayData = nullptr;
	_ra1ObjOverlayDataSize = 0;
	_ra1ObjOverlayCodec = 0;
	_ra1ObjOverlayLeft = 0;
	_ra1ObjOverlayTop = 0;
	_ra1ObjOverlayWidth = 0;
	_ra1ObjOverlayHeight = 0;
	_ra1ViewportOffsetX = 0;
	_ra1ViewportOffsetY = 0;
	_ra1FrameSourceSkipY = 0;
	_ra1LastFrameObjectVisible = true;
	_ra1FadeFrame = nullptr;
	_ra1FadeFrameSize = 0;
	_ra1FadeFrameWidth = 0;
	_ra1FadeFrameHeight = 0;
	_ra1FadeFrameValid = false;
	_ra1UseFadeFrame = false;
}

void SmushPlayerRebel1::destroyGamePlayerFields() {
	free(_ra1ObjOverlayData);
	_ra1ObjOverlayData = nullptr;
	_ra1ObjOverlayDataSize = 0;
	free(_ra1CleanFrame);
	_ra1CleanFrame = nullptr;
	_ra1CleanFrameSize = 0;
	free(_ra1PresentationBuffer);
	_ra1PresentationBuffer = nullptr;
	_ra1PresentationBufferSize = 0;
	free(_ra1FadeFrame);
	_ra1FadeFrame = nullptr;
	_ra1FadeFrameSize = 0;
}

void SmushPlayerRebel1::ra1InitAudioTrackSizes() {
	const int longAudioTrack = SMUSH_MAX_TRACKS - 1;
	if (_smushNumTracks <= longAudioTrack || _smushTracks[longAudioTrack].blockSize >= kRebel1LongAudioTrackSize)
		return;

	uint8 *blockPtr = (uint8 *)realloc(_smushTracks[longAudioTrack].blockPtr, kRebel1LongAudioTrackSize);
	if (!blockPtr) {
		warning("SmushPlayerRebel1::ra1InitAudioTrackSizes: failed to allocate %d-byte long audio track",
			kRebel1LongAudioTrackSize);
		return;
	}

	_smushTracks[longAudioTrack].blockPtr = blockPtr;
	_smushTracks[longAudioTrack].blockSize = kRebel1LongAudioTrackSize;
	memset(_smushTracks[longAudioTrack].blockPtr, 127, _smushTracks[longAudioTrack].blockSize);
}

void SmushPlayerRebel1::resetGameVideoState() {
	_ra1HasCleanFrame = false;
	free(_ra1ObjOverlayData);
	_ra1ObjOverlayData = nullptr;
	_ra1ObjOverlayDataSize = 0;
	_ra1ObjOverlayCodec = 0;
	_ra1ObjOverlayLeft = 0;
	_ra1ObjOverlayTop = 0;
	_ra1ObjOverlayWidth = 0;
	_ra1ObjOverlayHeight = 0;
	_ra1ViewportOffsetX = 0;
	_ra1ViewportOffsetY = 0;
	_ra1LastFrameObjectVisible = true;
	_ra1UseFadeFrame = false;
}

void SmushPlayerRebel1::initGameVideoState() {
	// Some RA1 ANMs inherit the current SMUSH palette. SmushPlayer::play()
	// clears the dirty range before init(), so re-push the palette that was
	// restored or inherited before this video starts.
	setDirtyColors(0, 255);
}

void SmushPlayerRebel1::releaseGameVideoState() {
	free(_storedFobjData);
	_storedFobjData = nullptr;
	_storedFobjDataSize = 0;
	_storedFobjCodec = 0;
	_storedFobjParm2 = 0;
	_storedFobjLeft = 0;
	_storedFobjTop = 0;
	_storedFobjWidth = 0;
	_storedFobjHeight = 0;

	free(_ra1ObjOverlayData);
	_ra1ObjOverlayData = nullptr;
	_ra1ObjOverlayDataSize = 0;

	free(_ra1CleanFrame);
	_ra1CleanFrame = nullptr;
	_ra1CleanFrameSize = 0;
	_ra1HasCleanFrame = false;

	free(_ra1PresentationBuffer);
	_ra1PresentationBuffer = nullptr;
	_ra1PresentationBufferSize = 0;

	free(_ra1FadeFrame);
	_ra1FadeFrame = nullptr;
	_ra1FadeFrameSize = 0;
	_ra1FadeFrameWidth = 0;
	_ra1FadeFrameHeight = 0;
	_ra1FadeFrameValid = false;
	_ra1UseFadeFrame = false;
}

bool SmushPlayerRebel1::handleGameFetch(int32 subSize, Common::SeekableReadStream &b) {
	if (subSize < 4)
		return false;

	uint32 fetchId = b.readUint32BE();
	int32 fetchX = 0;
	int32 fetchY = 0;
	if (subSize >= 12) {
		fetchX = b.readSint32BE();
		fetchY = b.readSint32BE();
	}

	if (_storedFobjData != nullptr && _storedFobjDataSize > 0) {
		const int storedCodec = _storedFobjCodec & 0xFF;
		const uint8 storedParam = (uint8)((_storedFobjCodec >> 8) & 0xFF);
		int left = _storedFobjLeft + fetchX;
		int top = _storedFobjTop + fetchY;

		if (_insane) {
			InsaneRebel1 *rebel1 = static_cast<InsaneRebel1 *>(_insane);
			if (rebel1->isInteractiveVideoActive()) {
				const uint16 gameOp = rebel1->getActiveGameOpcode();
				const bool fullWidthStoredPatch = (_storedFobjWidth == _vm->_screenWidth);
				const bool projectedCockpitPatch = (gameOp == 0x0B &&
					((rebel1->getCurrentLevel() == 4 && rebel1->getLevelGameplayPhase() == 2) ||
						rebel1->getCurrentLevel() == 7));
				// Keep the direct viewport placement used by the existing 0x0B
				// compatibility path, except for the Level 5 part 2 and Level 8
				// cockpit patches.
				// DOS FTCH goes through FUN_28D0A, which sets DispatchFobjCodec
				// flag 0x800; these cockpit patches depend on that quarter-projection
				// so the cockpit and flow-script indicators move together.
				if (fullWidthStoredPatch || (gameOp == 0x0B && !projectedCockpitPatch) ||
						gameOp == 0x19 || gameOp == 0x1A) {
					left += _ra1ViewportOffsetX;
					top += _ra1ViewportOffsetY;
				} else {
					ra1ApplyCenteredFetchPlacement(rebel1, _storedFobjWidth, _storedFobjHeight, left, top);
					// RA1 camera emulation currently uses a source-window crop
					// for interactive scenes. FTCH placement from the original executable
					// is computed in fixed presentation space, so convert it back into the
					// cropped buffer space used by the current renderer.
					left += _ra1ViewportOffsetX;
					top += _ra1ViewportOffsetY;
				}
			}
		}

		debugC(DEBUG_SMUSH, "FTCH: frame=%d id=0x%08x pos=(%d,%d) using stored FOBJ codec=%d size=%dx%d",
			_frame, fetchId, left, top, storedCodec, _storedFobjWidth, _storedFobjHeight);
		decodeFrameObject(storedCodec, _storedFobjData, left, top,
			_storedFobjWidth, _storedFobjHeight, _storedFobjDataSize,
			storedParam, _storedFobjParm2);
	} else {
		debugC(DEBUG_SMUSH, "FTCH: frame=%d id=0x%08x with no stored FOBJ data", _frame, fetchId);
	}

	return true;
}

void SmushPlayerRebel1::ra1HandleGost(int32 subSize, Common::SeekableReadStream &b) {
	if (subSize < 12) {
		warning("SmushPlayerRebel1::ra1HandleGost: chunk too small (%d bytes)", subSize);
		return;
	}

	const uint32 ghostType = b.readUint32BE();
	const int32 ghostX = b.readSint32BE();
	const int32 ghostY = b.readSint32BE();

	if (!_hasFrameFobjForGost || _lastFobjData == nullptr || _lastFobjDataSize <= 0) {
		debugC(DEBUG_SMUSH, "GOST: frame=%d ignored type=0x%08x pos=(%d,%d) (no current-frame FOBJ cached)",
			_frame, ghostType, ghostX, ghostY);
		return;
	}

	uint16 priorityFlags = 0;
	switch (ghostType) {
	case 0x1C:
		priorityFlags = 0x2000;
		break;
	case 0x1D:
		priorityFlags = 0x4000;
		break;
	case 0x1E:
		priorityFlags = 0x6000;
		break;
	default:
		debugC(DEBUG_SMUSH, "GOST: frame=%d ignored unknown type=0x%08x pos=(%d,%d)",
			_frame, ghostType, ghostX, ghostY);
		return;
	}

	debugC(DEBUG_SMUSH, "GOST: frame=%d type=0x%08x flags=0x%04x pos=(%d,%d) size=%dx%d codec=%d",
		_frame, ghostType, priorityFlags, ghostX, ghostY,
		_lastFobjWidth, _lastFobjHeight, _lastFobjCodec);

	// DOS reuses the most recent FOBJ payload for RA1 GOST and places it at the
	// absolute BE32 coordinates stored in the chunk. Priority bits are identified
	// here but not yet modeled in the generic decode path.
	decodeFrameObject(_lastFobjCodec, _lastFobjData, ghostX, ghostY,
		_lastFobjWidth, _lastFobjHeight, _lastFobjDataSize);
}

bool SmushPlayerRebel1::handleGameTextResource(uint32 subType, int32 subSize, Common::SeekableReadStream &b) {
	if (subType != MKTAG('T','E','X','T'))
		return false;

	bool forceText = false;
	if (subSize > 8) {
		const int64 textStart = b.pos();
		b.seek(textStart + 8, SEEK_SET);
		forceText = (b.readByte() == '.');
		b.seek(textStart, SEEK_SET);
	}

	// Original FUN_1FDBC draws RA1 TEXT when the text starts with '.' regardless
	// of the DIALOGUE TEXT option. O1OPEN uses that path for the opening lines.
	if (forceText || ConfMan.getBool("subtitles"))
		ra1HandleText(subSize, b);
	return true;
}

SmushFont *SmushPlayerRebel1::getGameFont(int font) {
	return ra1GetFont(font);
}

void SmushPlayerRebel1::adjustGamePalette() {
	for (int i = 0; i < ARRAYSIZE(_pal); ++i)
		_shiftedDeltaPal[i] = _pal[i] << 7;
	memset(_deltaPal, 0, sizeof(_deltaPal));
	_pal[0] = _pal[1] = _pal[2] = 0;
}

void SmushPlayerRebel1::ra1HandleDeltaPalette(int32 subSize, Common::SeekableReadStream &b) {
	if (subSize < 4) {
		b.skip(subSize);
		return;
	}

	const uint32 command = b.readUint32BE();
	const int32 payloadBytes = subSize - 4;

	if (command == 0 || command == 2) {
		_deltaPal[0] = 0;
		_shiftedDeltaPal[0] = 0;
		int32 remaining = payloadBytes;
		if (remaining >= 2) {
			// The original loop starts at palette component 1, leaving component
			// 0 black and ignoring the first delta word in the XPAL payload.
			b.skip(2);
			remaining -= 2;
		}

		for (int i = 1; i < ARRAYSIZE(_pal); ++i) {
			_shiftedDeltaPal[i] = _pal[i] << 7;
			if (remaining >= 2) {
				_deltaPal[i] = b.readSint16LE();
				remaining -= 2;
			} else {
				_deltaPal[i] = 0;
			}
		}

		if (remaining > 0)
			b.skip(remaining);

		// Command 2 in the DOS dispatcher first restores the palette state before
		// loading a new delta table. The active palette lives in _pal, so
		// marking it dirty is the corresponding visible-side effect.
		if (command == 2)
			setDirtyColors(0, 255);
		return;
	}

	if (payloadBytes > 0)
		b.skip(payloadBytes);

	for (int i = 1; i < ARRAYSIZE(_pal); ++i) {
		_shiftedDeltaPal[i] += _deltaPal[i];
		_pal[i] = CLIP<int32>(_shiftedDeltaPal[i] >> 7, 0, 255);
	}
	_pal[0] = _pal[1] = _pal[2] = 0;
	setDirtyColors(0, 255);
}

void SmushPlayerRebel1::ra1HandleFade(int32 subSize, Common::SeekableReadStream &b) {
	if (subSize <= 24 || _dst == nullptr || _width <= 0 || _height <= 0) {
		b.skip(subSize);
		return;
	}

	byte *fadeData = (byte *)malloc(subSize);
	if (fadeData == nullptr) {
		b.skip(subSize);
		return;
	}
	b.read(fadeData, subSize);

	int fadeWidth = kRA1PresentationScreenWidth;
	int fadeHeight = kRA1PresentationScreenHeight;
	if (subSize >= 16 && READ_BE_UINT32(fadeData) == MKTAG('F','D','H','D')) {
		const int headerWidth = READ_LE_UINT16(fadeData + 12);
		const int headerHeight = READ_LE_UINT16(fadeData + 14);
		if (headerWidth > 0 && headerHeight > 0) {
			fadeWidth = headerWidth;
			fadeHeight = headerHeight;
		}
	}

	fadeWidth = MIN(fadeWidth, MIN(_vm->_screenWidth, _width - _scrollX));
	fadeHeight = MIN(fadeHeight, MIN(_vm->_screenHeight, _height - _scrollY));
	if (fadeWidth <= 0 || fadeHeight <= 0) {
		free(fadeData);
		return;
	}

	const int32 fadeFrameSize = fadeWidth * fadeHeight;
	if (!ra1EnsureBuffer(_ra1FadeFrame, _ra1FadeFrameSize, fadeFrameSize)) {
		free(fadeData);
		return;
	}

	if (!_ra1FadeFrameValid ||
			_ra1FadeFrameWidth != fadeWidth || _ra1FadeFrameHeight != fadeHeight) {
		memset(_ra1FadeFrame, 0, fadeFrameSize);
		_ra1FadeFrameValid = true;
	}
	_ra1FadeFrameWidth = fadeWidth;
	_ra1FadeFrameHeight = fadeHeight;

	const byte *control = fadeData + 24;
	int32 remaining = subSize - 24;
	const byte *src = _dst + _scrollY * _width + _scrollX;
	int srcPos = 0;
	int dstPos = 0;

	while (remaining > 0 && dstPos < fadeFrameSize && srcPos < fadeFrameSize) {
		byte op = *control++;
		remaining--;

		int count = op & 0x7F;
		if (count == 0) {
			if (remaining < 2)
				break;
			count = READ_LE_UINT16(control);
			control += 2;
			remaining -= 2;
		}

		if (op & 0x80) {
			srcPos += count;
			dstPos += count;
		} else {
			ra1CopyFadeRun(_ra1FadeFrame, src, _width, fadeWidth, fadeHeight, dstPos, srcPos, count);
			srcPos += count;
			dstPos += count;
		}
	}

	_ra1UseFadeFrame = true;
	free(fadeData);
}

bool SmushPlayerRebel1::handleGameAnimHeader(byte *headerContent) {
	(void)headerContent;
	_width = kRA1DecodeWidth;
	_height = kRA1DecodeHeight;
	const int bufSize = kRA1DecodeWidth * kRA1DecodeHeight;
	if (_specialBuffer == nullptr || bufSize > _specialBufferSize) {
		free(_specialBuffer);
		_specialBuffer = (byte *)calloc(bufSize, 1);
		_specialBufferSize = bufSize;
	}
	if (_specialBuffer != nullptr)
		memset(_specialBuffer, 0, bufSize);
	_dst = _specialBuffer;
	return true;
}

void SmushPlayerRebel1::handleGameParseNextFrame() {
	processDispatches(_smushAudioSampleRate / _speed);
}

bool SmushPlayerRebel1::handleGameFrameBufferSelect(int codec, int width, int height) {
	// RA1 sub-fullscreen frames render into _specialBuffer at their (left, top) offset position.
	int bufSize = kRA1DecodeWidth * kRA1DecodeHeight;
	if (_specialBuffer == nullptr || bufSize > _specialBufferSize) {
		free(_specialBuffer);
		_specialBuffer = (byte *)calloc(bufSize, 1);
		_specialBufferSize = bufSize;
	}
	_dst = _specialBuffer;
	return true;
}

bool SmushPlayerRebel1::handleGameDimensionOverride(int codec, int width, int height) {
	if (_dst == _specialBuffer) {
		// RA1: sub-fullscreen FOBJs should not override the 384x242 dimensions.
		_width = kRA1DecodeWidth;
		_height = kRA1DecodeHeight;
		return true;
	}
	return false;
}

bool SmushPlayerRebel1::handleGameAdjustCoords(int codec, int &left, int &top, int &width, int &height, int pitch, int *srcSkipY) {
	_ra1FrameSourceSkipY = 0;

	// RA1 additive codec (SKIP_RLE) and scatter (RA1_SCATTER) use absolute
	// positions — they must NOT be clipped/adjusted.
	if (codec == SMUSH_CODEC_SKIP_RLE || codec == SMUSH_CODEC_RA1_SCATTER)
		return false;

	// RA1 block codecs are column-major tile streams, not row-prefixed RLE
	// streams. Preserve the source data and let smushDecodeRA1Block() consume
	// offscreen tiles while clipping destination pixels.
	if (codec == SMUSH_CODEC_RA1_DELTA || codec == SMUSH_CODEC_RA1_BLOCK) {
		left += _fobjOffsetX;
		top += _fobjOffsetY;
		return false;
	}

	// RA1 codec 21 is source-X sensitive: generic left clipping would reduce
	// the destination width without skipping the corresponding source columns.
	// Keep only the global FOBJ offset here and let the codec clip each run.
	if (codec == SMUSH_CODEC_LINE_UPDATE) {
		left += _fobjOffsetX;
		top += _fobjOffsetY;
		return false;
	}

	int sourceSkipY = 0;
	adjustFrameCoords(left, top, width, height, pitch, &sourceSkipY);
	if (codec == SMUSH_CODEC_RLE_ALT) {
		_ra1FrameSourceSkipY = sourceSkipY;
		if (srcSkipY)
			*srcSkipY = 0;
	} else if (srcSkipY) {
		*srcSkipY = sourceSkipY;
	}
	return true;
}

bool SmushPlayerRebel1::handleGameCodecDecode(int codec, const uint8 *src, int left, int top, int width, int height, int pitch, int dataSize, uint8 param, uint16 parm2) {
	switch (codec) {
	case SMUSH_CODEC_RLE:
		smushDecodeRA1Transparent(_dst, src, left, top, width, height, pitch, dataSize);
		return true;
	case SMUSH_CODEC_RLE_ALT:
		src = smushSkipRLELines(src, dataSize, _ra1FrameSourceSkipY);
		smushDecodeRLEOpaque(_dst, src, left, top, width, height, pitch, dataSize);
		return true;
	case SMUSH_CODEC_RA1_SCATTER:
		smushDecodeRA1Scatter(_dst, src, left, top, _width, _height, pitch, dataSize);
		return true;
	case SMUSH_CODEC_RA1_DELTA:
	case SMUSH_CODEC_RA1_BLOCK:
		smushDecodeRA1Block(_dst, src, left, top, width, height, pitch,
			(_dst == _specialBuffer) ? _height : _vm->_screenHeight,
			dataSize, param, parm2, codec);
		return true;
	case SMUSH_CODEC_LINE_UPDATE:
		smushDecodeRA1SkipCopy(_dst, src, left, top, width, height, pitch,
			pitch, (_dst == _specialBuffer) ? _height : _vm->_screenHeight, dataSize);
		return true;
	case SMUSH_CODEC_SKIP_RLE: {
		const int bufWidth = pitch;
		const int bufHeight = (_dst == _specialBuffer) ? _height : _vm->_screenHeight;
		// Codec 23 uses the high byte of the FOBJ codec word as the palette
		// band for its additive delta. The event-mask path may subtract 0x10
		// from this value before decoding, which Level 8 uses for the walker
		// armor layers.
		smushDecodeRA1AdditiveLineUpdate(_dst, src, left, top, width, height,
			pitch, bufWidth, bufHeight, param, dataSize);
		return true;
	}
	default:
		debugC(DEBUG_SMUSH, "SmushPlayerRebel1::handleGameCodecDecode: Skipping unknown codec %d (left=%d, top=%d, %dx%d)",
			codec, left, top, width, height);
		return true;
	}
}

bool SmushPlayerRebel1::handleGameStoreFrame() {
	// RA1 handles STOR via handleGameFrameObjectPost
	return true;
}

void SmushPlayerRebel1::handleGameFrameObjectPre(int codec, int left, int top, int width, int height, int dataSize) {
	debugC(DEBUG_SMUSH, "FOBJ: frame=%d codec=%d pos=(%d,%d) size=%dx%d dataSize=%d storeFrame=%d",
		_frame, codec, left, top, width, height, dataSize, _storeFrame);
}

void SmushPlayerRebel1::handleGameFrameObjectPost(int codec, const byte *data, int32 dataSize, int left, int top, int width, int height) {
	rememberLastFobj(codec, data, dataSize, left, top, width, height);
	// RA1 STOR handling remains in handleFrameObject (needs ra1Param/rawLeft/rawTop)
}

void SmushPlayerRebel1::handleGameFrameStart() {
	_hasFrameFobjForGost = false;
}

void SmushPlayerRebel1::handleGameGost(int32 subSize, Common::SeekableReadStream &b) {
	ra1HandleGost(subSize, b);
}

void SmushPlayerRebel1::handleGameProcessAudio(int16 feedSize) {
	if (_insane) {
		InsaneRebel1 *rebel1 = static_cast<InsaneRebel1 *>(_insane);
		rebel1->processAudioFrame(feedSize);
	}
}

// ---------------------------------------------------------------------------
// handleFrameObject override — RA1 FOBJ has extra fields in the codec word
// ---------------------------------------------------------------------------

void SmushPlayerRebel1::handleFrameObject(int32 subSize, Common::SeekableReadStream &b) {
	assert(subSize >= 14);
	if (_skipNext) {
		_skipNext = false;
		_ra1LastFrameObjectVisible = false;
		return;
	}

	int codec = b.readUint16LE();
	uint8 ra1Param = (codec >> 8) & 0xFF;
	codec &= 0xFF;

	int left = (int)b.readSint16LE();
	int top = (int)b.readSint16LE();
	const int rawLeft = left;
	const int rawTop = top;
	int width = b.readUint16LE();
	int height = b.readUint16LE();

	uint16 ra1ObjectId = b.readUint16LE();
	uint16 ra1Parm2 = b.readUint16LE();

	handleGameFrameObjectPre(codec, left, top, width, height, subSize - 14);

	int32 chunk_size = subSize - 14;
	byte *chunk_buffer = (byte *)malloc(chunk_size);
	assert(chunk_buffer);
	b.read(chunk_buffer, chunk_size);

	handleGameFrameObjectPost(codec, chunk_buffer, chunk_size, left, top, width, height);

	// RA1 STOR: save raw FOBJ with original (pre-clipped) coords and full codec byte
	if (_storeFrame) {
		free(_storedFobjData);
		_storedFobjData = (byte *)malloc(chunk_size);
		if (_storedFobjData != nullptr) {
			memcpy(_storedFobjData, chunk_buffer, chunk_size);
			_storedFobjDataSize = chunk_size;
			_storedFobjCodec = codec | ((int)ra1Param << 8);
			_storedFobjParm2 = ra1Parm2;
			_storedFobjLeft = rawLeft;
			_storedFobjTop = rawTop;
			_storedFobjWidth = width;
			_storedFobjHeight = height;
		} else {
			_storedFobjDataSize = 0;
		}
		_storeFrame = false;
	}

	// RA1 target check — Insane can reject certain FOBJs
	if (_insane) {
		InsaneRebel1 *rebel1 = static_cast<InsaneRebel1 *>(_insane);
		if (!rebel1->handleFrameObjectTarget((int16)ra1ObjectId, (int16)rawLeft, (int16)rawTop,
				(int16)width, (int16)height, codec, ra1Param)) {
			_ra1LastFrameObjectVisible = false;
			free(chunk_buffer);
			return;
		}
	}

	_ra1LastFrameObjectVisible = true;
	decodeFrameObject(codec, chunk_buffer, left, top, width, height, chunk_size, ra1Param, ra1Parm2);
	free(chunk_buffer);
}

// ---------------------------------------------------------------------------
// handleFrame override — RA1 frame parsing with alignment, OBJ chunks, clean frame
// ---------------------------------------------------------------------------

static bool ra1ScanFrameGameChunks(Common::SeekableReadStream &b, int32 frameSize, uint32 &opcodeMask) {
	opcodeMask = 0;
	const int64 frameStart = b.pos();
	int32 remaining = frameSize;
	RA1FrameChunkIterator chunks(b, remaining);
	RA1AnimChunk chunk;
	bool hasGameChunk = false;
	while (chunks.next(chunk)) {
		if (chunk.tag == MKTAG('F', 'R', 'M', 'E'))
			break;
		if (chunk.tag == MKTAG('G', 'A', 'M', 'E') ||
				chunk.tag == MKTAG('G', 'A', 'M', '2')) {
			hasGameChunk = true;
			if (chunks.fits(chunk) && chunk.size >= 4) {
				const uint32 opcode = b.readUint32BE();
				if (opcode < 32)
					opcodeMask |= (1u << opcode);
			}
		}

		if (!chunks.fits(chunk))
			break;
		chunks.skip(chunk);
	}

	b.seek(frameStart, SEEK_SET);
	return hasGameChunk;
}

static int16 getRA1AudioChunkTypeFlags(uint32 subType) {
	switch (subType) {
	case MKTAG('P','V','O','C'):
		return IS_SPEECH;
	case MKTAG('P','S','A','D'):
		return IS_BKG_MUSIC;
	case MKTAG('P','S','D','2'):
	case MKTAG('S','A','U','D'):
	default:
		return IS_SFX;
	}
}

void SmushPlayerRebel1::ra1HandleFrameAudioChunk(uint32 subType, int32 subSize, Common::SeekableReadStream &b) {
	if (_compressedFileMode || isFastForwardingCurrentFrame())
		return;
	if (subSize <= 0)
		return;

	uint8 *audioChunk = (uint8 *)malloc(subSize + 8);
	if (audioChunk == nullptr)
		return;

	WRITE_BE_UINT32(audioChunk, subType);
	WRITE_BE_UINT32(audioChunk + 4, subSize);
	b.read(audioChunk + 8, subSize);
	ra1FeedAudio(subType, audioChunk, 0, 127, 0, 0);
	free(audioChunk);
}

void SmushPlayerRebel1::ra1FeedAudio(uint32 subType, uint8 *srcBuf, int groupId, int volume, int pan, int16 flags) {
	if (!_smushAudioInitialized)
		return;

	const uint32 chunkSize = READ_BE_UINT32(&srcBuf[4]);
	const int16 typeFlags = getRA1AudioChunkTypeFlags(subType);

	if (chunkSize >= 12 &&
			srcBuf[8] == 0 && srcBuf[9] == 0 && srcBuf[12] == 0 &&
			srcBuf[13] == 0 && srcBuf[16] == 0 && srcBuf[17] == 0) {
		const uint16 trkId = READ_BE_INT16(&srcBuf[10]);
		const uint16 index = READ_BE_INT16(&srcBuf[14]);
		const int32 maxFrames = READ_BE_INT16(&srcBuf[18]);
		flags = (flags & ~TRK_TYPE_MASK) | typeFlags;

		handleSAUDChunk(
			srcBuf + 20,
			chunkSize - 12,
			groupId,
			volume,
			pan,
			flags,
			trkId,
			index,
			maxFrames);
	} else if (chunkSize >= 10) {
		const uint16 trkId = READ_LE_INT16(&srcBuf[8]);
		const uint16 index = READ_LE_INT16(&srcBuf[10]);
		const int32 maxFrames = READ_LE_INT16(&srcBuf[12]);
		flags |= READ_LE_INT16(&srcBuf[14]);
		flags = (flags & ~TRK_TYPE_MASK) | typeFlags;
		volume = (volume * srcBuf[16]) >> 7;

		const int panDelta = (int8)srcBuf[17];
		const int effPan = (panDelta == -128) ? 128 : pan + panDelta;

		handleSAUDChunk(
			srcBuf + 18,
			chunkSize - 10,
			groupId,
			volume,
			effPan,
			flags,
			trkId,
			index,
			maxFrames);
	}
}

void SmushPlayerRebel1::ra1HandleGameFrameChunk(int32 subSize, Common::SeekableReadStream &b, bool fastForwarding) {
	if (!fastForwarding && _insane) {
		InsaneRebel1 *rebel1 = (InsaneRebel1 *)_insane;
		rebel1->handleGameChunk(subSize, b, _dst, _width, _height);
	}
}

void SmushPlayerRebel1::ra1HandleObjOverlayFrameChunk(int32 objDataSize, Common::SeekableReadStream &b, bool fastForwarding) {
	if (objDataSize <= 0)
		return;

	byte *objBuf = (byte *)malloc(objDataSize);
	if (objBuf == nullptr)
		return;

	b.read(objBuf, objDataSize);

	int32 objPos = 0;
	while (objPos + 8 < objDataSize) {
		uint32 embTag = READ_BE_UINT32(objBuf + objPos);
		uint32 embSize = READ_BE_UINT32(objBuf + objPos + 4);
		int32 embRemaining = objDataSize - objPos - 8;

		bool recognized = (embTag == MKTAG('F','O','B','J') ||
		                   embTag == MKTAG('G','A','M','E') ||
		                   embTag == MKTAG('G','A','M','2') ||
		                   embTag == MKTAG('P','S','A','D'));

		if (!recognized || embSize > (uint32)embRemaining) {
			objPos++;
			continue;
		}

		if (embTag == MKTAG('F','O','B','J') && embSize >= 14) {
			Common::MemoryReadStream embStream(objBuf + objPos + 8, embSize);
			handleFrameObject(embSize, embStream);

			if (_ra1ObjOverlayData == nullptr ||
			    (int32)embSize > _ra1ObjOverlayDataSize) {
				free(_ra1ObjOverlayData);
				_ra1ObjOverlayDataSize = embSize;
				_ra1ObjOverlayData = (byte *)malloc(embSize);
				memcpy(_ra1ObjOverlayData, objBuf + objPos + 8, embSize);
				_ra1ObjOverlayCodec = objBuf[objPos + 8] & 0xFF;
				_ra1ObjOverlayLeft = (int16)READ_LE_UINT16(objBuf + objPos + 10);
				_ra1ObjOverlayTop = (int16)READ_LE_UINT16(objBuf + objPos + 12);
				_ra1ObjOverlayWidth = READ_LE_UINT16(objBuf + objPos + 14);
				_ra1ObjOverlayHeight = READ_LE_UINT16(objBuf + objPos + 16);
			}
		} else if (embTag == MKTAG('G','A','M','E') || embTag == MKTAG('G','A','M','2')) {
			Common::MemoryReadStream embStream(objBuf + objPos + 8, embSize);
			ra1HandleGameFrameChunk(embSize, embStream, fastForwarding);
		} else if (embTag == MKTAG('P','S','A','D')) {
			if (!_compressedFileMode && !isFastForwardingCurrentFrame()) {
				uint8 *audioBuf = (uint8 *)malloc(embSize + 8);
				if (audioBuf == nullptr)
					break;
				memcpy(audioBuf, objBuf + objPos, embSize + 8);
				ra1FeedAudio(embTag, audioBuf, 0, 127, 0, 0);
				free(audioBuf);
			}
		}

		objPos += 8 + embSize;
		if (embSize & 1)
			objPos++;
	}

	free(objBuf);
}

bool SmushPlayerRebel1::ra1HandleUnknownFrameChunk(uint32 subType, int32 subSize) {
	// Original FUN_1FDBC: unknown uppercase tag -> silently stop
	byte tb0 = (subType >> 24) & 0xFF, tb1 = (subType >> 16) & 0xFF;
	byte tb2 = (subType >> 8) & 0xFF, tb3 = subType & 0xFF;
	if (tb0 > 0x40 && tb0 < 0x5B && tb1 > 0x40 && tb1 < 0x5B &&
	    tb2 > 0x40 && tb2 < 0x5B && tb3 > 0x40 && tb3 < 0x5B) {
		debugC(DEBUG_SMUSH, "unknown uppercase tag %s at frame %d, stopping frame parse", tag2str(subType), _frame);
		return true;
	}

	error("Unknown frame subChunk found : %s, %d", tag2str(subType), subSize);
	return false;
}

bool SmushPlayerRebel1::ra1DispatchFrameChunk(uint32 subType, int32 subSize, int32 &frameSize,
		Common::SeekableReadStream &b, bool fastForwarding) {
	switch (subType) {
	case MKTAG('N','P','A','L'):
		handleNewPalette(subSize, b);
		break;
	case MKTAG('F','O','B','J'):
		handleFrameObject(subSize, b);
		break;
	case MKTAG('Z','F','O','B'):
		handleZlibFrameObject(subSize, b);
		break;
	case MKTAG('P','S','A','D'):
	case MKTAG('P','V','O','C'):
		ra1HandleFrameAudioChunk(subType, subSize, b);
		break;
	case MKTAG('P','S','D','2'):
		if (_ra1LastFrameObjectVisible)
			ra1HandleFrameAudioChunk(subType, subSize, b);
		break;
	case MKTAG('T','R','E','S'):
	case MKTAG('T','E','X','T'):
		handleTextResource(subType, subSize, b);
		break;
	case MKTAG('X','P','A','L'):
		ra1HandleDeltaPalette(subSize, b);
		break;
	case MKTAG('I','A','C','T'):
		handleIACT(subSize, b);
		break;
	case MKTAG('S','T','O','R'):
		handleStore(subSize, b);
		break;
	case MKTAG('F','T','C','H'):
		handleFetch(subSize, b);
		break;
	case MKTAG('S','K','I','P'):
		_insane->procSKIP(subSize, b);
		break;
	case MKTAG('G','O','S','T'):
		handleGameGost(subSize, b);
		break;
	case MKTAG('G','A','M','E'):
	case MKTAG('G','A','M','2'):
		ra1HandleGameFrameChunk(subSize, b, fastForwarding);
		break;
	case MKTAG('O','B','J','\0'):
		ra1HandleObjOverlayFrameChunk(frameSize - 8, b, fastForwarding);
		frameSize = 0;
		return true;
	case MKTAG('F','A','D','E'):
		ra1HandleFade(subSize, b);
		break;
	case MKTAG('S','E','G','A'):
	case MKTAG('A','D','L',' '):
	case MKTAG('A','D','L','2'):
	case MKTAG('S','B','L',' '):
	case MKTAG('S','B','L','2'):
		debugC(DEBUG_SMUSH, "SmushPlayerRebel1::handleFrame: skipping chunk %s (%d bytes)", tag2str(subType), subSize);
		break;
	default:
		if (ra1HandleUnknownFrameChunk(subType, subSize)) {
			frameSize = 0;
			return true;
		}
		break;
	}

	return false;
}

void SmushPlayerRebel1::handleFrame(int32 frameSize, Common::SeekableReadStream &b) {
	debugC(DEBUG_SMUSH, "SmushPlayerRebel1::handleFrame(%d)", _frame);
	_skipNext = false;
	_ra1LastFrameObjectVisible = true;
	handleGameFrameStart();
	const bool fastForwarding = isFastForwardingCurrentFrame();

	bool preserveFrameHistory = false;
	if (_insane) {
		InsaneRebel1 *rebel1 = static_cast<InsaneRebel1 *>(_insane);
		rebel1->setCurrentSmushFrame(_frame);
		bool interactive = rebel1->isInteractiveVideoActive();
		uint32 frameGameOpcodeHintMask = 0;
		const bool frameHasGameChunk = interactive &&
			ra1ScanFrameGameChunks(b, frameSize, frameGameOpcodeHintMask);
		rebel1->setFrameHasGameChunk(frameHasGameChunk);
		rebel1->setFrameGameOpcodeHintMask(frameGameOpcodeHintMask);
		const uint16 activeOpcode = rebel1->getActiveGameOpcode();
		bool forceClear = interactive &&
			(activeOpcode == 0x0B ||
			 (activeOpcode == 0 && rebel1->getCurrentLevel() == 1));

		preserveFrameHistory = interactive && !forceClear && frameHasGameChunk;
	}

	// Restore clean frame for delta source
	if (preserveFrameHistory &&
		_ra1HasCleanFrame && _ra1CleanFrame &&
		_dst && _width > 0 && _height > 0) {
		const int frameBytes = _width * _height;
		if (_ra1CleanFrameSize >= frameBytes)
			memcpy(_dst, _ra1CleanFrame, frameBytes);
	}

	if (_insanity)
		_insane->procPreRendering(_dst);

	// Clear buffer for non-interactive frames to avoid trails
	if (_dst && _width > 0 && _height > 0) {
		if (!preserveFrameHistory)
			memset(_dst, 0, _width * _height);
	}

	RA1FrameChunkIterator chunks(b, frameSize);
	RA1AnimChunk chunk;
	while (chunks.next(chunk)) {
		const int32 subSize = (int32)chunk.size;

		// Guard against consuming next frame marker
		if (chunk.tag == MKTAG('F','R','M','E')) {
			b.seek(chunk.offset, SEEK_SET);
			break;
		}

		if (ra1DispatchFrameChunk(chunk.tag, subSize, frameSize, b, fastForwarding))
			continue;

		chunks.skip(chunk);
		// RA1 uses top-of-loop alignment, not bottom-of-loop padding
	}

	// Re-render cockpit overlay
	if (_ra1ObjOverlayData != nullptr && _frame > 0) {
		Common::MemoryReadStream overlayStream(_ra1ObjOverlayData, _ra1ObjOverlayDataSize);
		handleFrameObject(_ra1ObjOverlayDataSize, overlayStream);
	}

	// Save clean frame for next delta
	if (preserveFrameHistory && _dst && _width > 0 && _height > 0) {
		const int frameBytes = _width * _height;
		byte *newClean = (byte *)realloc(_ra1CleanFrame, frameBytes);
		if (newClean != nullptr) {
			_ra1CleanFrame = newClean;
			_ra1CleanFrameSize = frameBytes;
			memcpy(_ra1CleanFrame, _dst, frameBytes);
			_ra1HasCleanFrame = true;
		} else {
			_ra1HasCleanFrame = false;
		}
	} else {
		_ra1HasCleanFrame = false;
	}

	if (_insanity && !fastForwarding)
		_insane->procPostRendering(_dst, 0, 0, 0, _frame, _nbframes-1);

	if (_width != 0 && _height != 0 && !fastForwarding)
		updateScreen();

	_frame++;
}

// ---------------------------------------------------------------------------
// handleGameUpdateScreen — RA1 viewport-aware screen blit
// ---------------------------------------------------------------------------

void SmushPlayerRebel1::handleGameUpdateScreen(const byte *src, int srcPitch, int width, int height) {
	if (_dst == nullptr || _width <= 0 || _height <= 0)
		return;

	const bool useFadeFrame = _ra1UseFadeFrame && _ra1FadeFrameValid && _ra1FadeFrame != nullptr;
	if (useFadeFrame) {
		src = _ra1FadeFrame;
		srcPitch = _ra1FadeFrameWidth;
		width = MIN(width, _ra1FadeFrameWidth);
		height = MIN(height, _ra1FadeFrameHeight);
	}

	if (!_insane || !static_cast<InsaneRebel1 *>(_insane)->isInteractiveVideoActive() ||
			_vm->_screenWidth != kRA1PresentationScreenWidth ||
			_vm->_screenHeight != kRA1PresentationScreenHeight) {
		SmushPlayer::handleGameUpdateScreen(src, srcPitch, width, height);
		ra1RememberDisplayedFrame(_ra1FadeFrame, _ra1FadeFrameSize,
			_ra1FadeFrameWidth, _ra1FadeFrameHeight, _ra1FadeFrameValid,
			src, srcPitch, width, height);
		_ra1UseFadeFrame = false;
		return;
	}

	int ra1ViewX = _ra1ViewportOffsetX;
	int ra1ViewY = _ra1ViewportOffsetY;

	const byte *sourceBase = useFadeFrame ? src : _dst;
	const int sourcePitch = useFadeFrame ? srcPitch : _width;
	const int sourceWidth = useFadeFrame ? width : _width;
	const int sourceHeight = useFadeFrame ? height : _height;
	const int srcX = useFadeFrame ? 0 : CLIP(_scrollX + ra1ViewX + kRA1PresentationBorder, 0, sourceWidth - 1);
	const int srcY = useFadeFrame ? 0 : CLIP(_scrollY + ra1ViewY + kRA1PresentationBorder, 0, sourceHeight - 1);

	int frameWidth = MIN<int>(sourceWidth - srcX, kRA1PresentationWidth);
	int frameHeight = MIN<int>(sourceHeight - srcY, kRA1PresentationHeight);
	if (frameWidth <= 0 || frameHeight <= 0)
		return;

	const int presentationSize = kRA1PresentationScreenWidth * kRA1PresentationScreenHeight;
	if (_ra1PresentationBuffer == nullptr || _ra1PresentationBufferSize < presentationSize) {
		byte *newPresentationBuffer = (byte *)realloc(_ra1PresentationBuffer, presentationSize);
		if (newPresentationBuffer == nullptr)
			return;
		_ra1PresentationBuffer = newPresentationBuffer;
		_ra1PresentationBufferSize = presentationSize;
	}
	memset(_ra1PresentationBuffer, 0, presentationSize);

	// ResetPlaybackViewport() (0x20A53) initializes the interactive draw window
	// to (4,4,312,192), leaving a black presentation frame around cockpit scenes.
	const byte *dst = sourceBase + srcY * sourcePitch + srcX;
	byte *presentationDst = _ra1PresentationBuffer +
		kRA1PresentationBorder * kRA1PresentationScreenWidth + kRA1PresentationBorder;
	for (int y = 0; y < frameHeight; y++) {
		memcpy(presentationDst + y * kRA1PresentationScreenWidth,
			dst + y * sourcePitch, frameWidth);
	}

	SmushPlayer::handleGameUpdateScreen(_ra1PresentationBuffer,
		kRA1PresentationScreenWidth, kRA1PresentationScreenWidth, kRA1PresentationScreenHeight);
	ra1RememberDisplayedFrame(_ra1FadeFrame, _ra1FadeFrameSize,
		_ra1FadeFrameWidth, _ra1FadeFrameHeight, _ra1FadeFrameValid,
		_ra1PresentationBuffer, kRA1PresentationScreenWidth,
		kRA1PresentationScreenWidth, kRA1PresentationScreenHeight);
	_ra1UseFadeFrame = false;
}

SmushFont *SmushPlayerRebel1::ra1GetFont(int font) {
	const char *ra1Fonts[] = {
		"SYS/TALKFONT.NUT",
		"SYS/TECHFONT.NUT",
		"SYS/TITLFONT.NUT",
		"SYS/DISPLAY.NUT"
	};
	const char *ra2FallbackFonts[] = {
		"SYSTM/TALKFONT.NUT",
		"SYSTM/SMALFONT.NUT",
		"SYSTM/TITLFONT.NUT",
		"SYSTM/SMALFONT.NUT"
	};

	int numFonts = ARRAYSIZE(ra1Fonts);
	if (font < 0 || font >= numFonts) {
		debugC(DEBUG_SMUSH, "SmushPlayerRebel1::ra1GetFont: unknown font %d, using TALKFONT", font);
		font = 0;
	}

	if (_sf[font])
		return _sf[font];

	const char *fontPath = ra1Fonts[font];
	ScummFile *testFile = _vm->instantiateScummFile();
	bool ok = _vm->openFile(*testFile, Common::Path(fontPath));
	if (ok)
		testFile->close();
	delete testFile;

	if (!ok)
		fontPath = ra2FallbackFonts[font];

	_sf[font] = new SmushFont(_vm, fontPath, true);
	return _sf[font];
}

void SmushPlayerRebel1::ra1HandleText(int32 subSize, Common::SeekableReadStream &b) {
	if (subSize < 8 || !_dst || _width <= 0 || _height <= 0)
		return;

	InsaneRebel1 *rebel1 = static_cast<InsaneRebel1 *>(_insane);
	if (!rebel1)
		return;

	const int textAnchorX = b.readSint32BE();
	int cursorY = b.readSint32BE();

	int textLen = subSize - 8;
	if (textLen <= 0)
		return;

	byte *textBuf = (byte *)malloc(textLen);
	if (!textBuf)
		return;
	b.read(textBuf, textLen);

	int start = 0;
	if (textLen > 0 && textBuf[0] == '.')
		start = 1;

	int remaining = textLen - start;
	while (remaining > 0) {
		int lineLen = 0;
		while (lineLen < remaining && textBuf[start + lineLen] != 0)
			lineLen++;

		if (lineLen > 0) {
			char *line = (char *)malloc(lineLen + 1);
			if (!line) {
				cursorY += 12;
			} else {
				memcpy(line, textBuf + start, lineLen);
				line[lineLen] = '\0';
				const int drawX = textAnchorX - (rebel1->getFontBankStringWidth(line) / 2);
				rebel1->drawFontBankString(_dst, _width, _width, _height, drawX, cursorY, line);
				cursorY += rebel1->getFontBankLineAdvance(line);
				free(line);
			}
		} else {
			cursorY += rebel1->getFontBankLineAdvance(nullptr);
		}

		int consumed = lineLen;
		if (consumed < remaining && textBuf[start + consumed] == 0)
			consumed++;
		start += consumed;
		remaining -= consumed;
	}

	free(textBuf);
}

} // End of namespace Scumm
