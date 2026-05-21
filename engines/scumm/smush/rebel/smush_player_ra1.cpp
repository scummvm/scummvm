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

#include "common/endian.h"
#include "common/memstream.h"

#include "scumm/file.h"
#include "scumm/scumm_v7.h"
#include "scumm/smush/smush_font.h"
#include "scumm/smush/rebel/codec_ra2.h"
#include "scumm/smush/rebel/smush_player_ra1.h"

#include "scumm/insane/rebel1/rebel.h"

namespace Scumm {

enum {
	kRA1PresentationBorder = 4,
	kRA1PresentationScreenWidth = 320,
	kRA1PresentationScreenHeight = 200,
	kRA1PresentationWidth = kRA1PresentationScreenWidth - kRA1PresentationBorder * 2,
	kRA1PresentationHeight = kRA1PresentationScreenHeight - kRA1PresentationBorder * 2
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

SmushPlayerRebel1::SmushPlayerRebel1(ScummEngine_v7 *scumm, IMuseDigital *imuseDigital, Insane *insane)
	: SmushPlayer(scumm, imuseDigital, insane) {
	initGamePlayerFields();
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
				// 0x0B (asteroid/surface) and 0x19/0x1A (on-foot) use SetCameraOffset
				// directly — no projection-based FTCH placement. Level 4 phase 2
				// also stores a full-width screen-space patch (320x180) that DOS
				// restores without the centered 1/4 projection warp.
				if (fullWidthStoredPatch || gameOp == 0x19 || gameOp == 0x1A) {
					left += _ra1ViewportOffsetX;
					top += _ra1ViewportOffsetY;
				} else {
					ra1ApplyCenteredFetchPlacement(rebel1, _storedFobjWidth, _storedFobjHeight, left, top);
					// ScummVM currently emulates the RA1 camera with a source-window crop
					// for interactive scenes. FTCH placement from the original executable
					// is computed in fixed presentation space, so convert it back into the
					// cropped buffer space used by the current renderer.
					left += _ra1ViewportOffsetX;
					top += _ra1ViewportOffsetY;
				}
			}
		}

		debug("RA1 FTCH: frame=%d id=0x%08x pos=(%d,%d) using stored FOBJ codec=%d size=%dx%d",
			_frame, fetchId, left, top, storedCodec, _storedFobjWidth, _storedFobjHeight);
		decodeFrameObject(storedCodec, _storedFobjData, left, top,
			_storedFobjWidth, _storedFobjHeight, _storedFobjDataSize,
			storedParam, _storedFobjParm2);
	} else {
		debug("RA1 FTCH: frame=%d id=0x%08x with no stored FOBJ data", _frame, fetchId);
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
		debug("RA1 GOST: frame=%d ignored type=0x%08x pos=(%d,%d) (no current-frame FOBJ cached)",
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
		debug("RA1 GOST: frame=%d ignored unknown type=0x%08x pos=(%d,%d)",
			_frame, ghostType, ghostX, ghostY);
		return;
	}

	debug("RA1 GOST: frame=%d type=0x%08x flags=0x%04x pos=(%d,%d) size=%dx%d codec=%d",
		_frame, ghostType, priorityFlags, ghostX, ghostY,
		_lastFobjWidth, _lastFobjHeight, _lastFobjCodec);

	// DOS reuses the most recent FOBJ payload for RA1 GOST and places it at the
	// absolute BE32 coordinates stored in the chunk. Priority bits are identified
	// here but not yet modeled in the generic ScummVM decode path.
	decodeFrameObject(_lastFobjCodec, _lastFobjData, ghostX, ghostY,
		_lastFobjWidth, _lastFobjHeight, _lastFobjDataSize);
}

bool SmushPlayerRebel1::handleGameTextResource(uint32 subType, int32 subSize, Common::SeekableReadStream &b) {
	if (subType != MKTAG('T','E','X','T'))
		return false;

	ra1HandleText(subSize, b);
	return true;
}

SmushFont *SmushPlayerRebel1::getGameFont(int font) {
	return ra1GetFont(font);
}

void SmushPlayerRebel1::adjustGamePalette() {
	_pal[0] = _pal[1] = _pal[2] = 0;
}

bool SmushPlayerRebel1::handleGameAnimHeader(byte *headerContent) {
	(void)headerContent;
	_width = 0;
	_height = 0;
	const int bufSize = 384 * 242;
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

// Forward declarations for RA1 codec functions (defined in smush_player.cpp and codec1.cpp)
/**
 * RA1 codec 1: RLE with transparency on pixel 0.
 * Same BOMP encoding as smushDecodeRLE but pixel value 0 is not written,
 * allowing the background (restored via FTCH) to show through.
 */
void smushDecodeRA1Transparent(byte *dst, const byte *src, int left, int top, int width, int height, int pitch) {
	dst += top * pitch;
	do {
		byte *rowDst = dst + left;
		const byte *lineData = src + 2;
		int remaining = width;

		while (remaining > 0) {
			byte code = *lineData++;
			byte num = (code >> 1) + 1;
			if (num > remaining)
				num = remaining;
			if (code & 1) {
				byte color = *lineData++;
				if (color != 0)
					memset(rowDst, color, num);
			} else {
				for (int j = 0; j < num; j++) {
					byte c = lineData[j];
					if (c != 0)
						rowDst[j] = c;
				}
				lineData += num;
			}
			rowDst += num;
			remaining -= num;
		}

		src += READ_LE_UINT16(src) + 2;
		dst += pitch;
	} while (--height);
}
/**
 * RA1 codec 21: Skip/copy line codec (FUN_10D41). Clip copy runs without
 * changing source X; stored cockpit patches can legitimately start offscreen.
 */
void smushDecodeRA1SkipCopy(byte *dst, const byte *src, int left, int top, int width, int height,
		int pitch, int bufWidth, int bufHeight) {
	for (int row = 0; row < height; row++) {
		const uint16 lineSize = READ_LE_UINT16(src);
		const byte *lineData = src + 2;
		const byte *lineEnd = lineData + lineSize;
		const int dstY = top + row;
		int srcX = 0;

		while (srcX < width && lineData < lineEnd) {
			if (lineData + 2 > lineEnd)
				break;
			const uint16 skip = READ_LE_UINT16(lineData);
			lineData += 2;
			srcX += skip;
			if (srcX >= width)
				break;

			if (lineData + 2 > lineEnd)
				break;
			const int copyLen = READ_LE_UINT16(lineData) + 1;
			lineData += 2;

			const int readableLen = MIN<int>(copyLen, (int)(lineEnd - lineData));
			const int dstStartX = left + srcX;
			const int dstEndX = dstStartX + readableLen;
			if (readableLen > 0 && dstY >= 0 && dstY < bufHeight) {
				const int clippedStartX = MAX(dstStartX, 0);
				const int clippedEndX = MIN(dstEndX, bufWidth);
				if (clippedStartX < clippedEndX) {
					const int srcSkipX = clippedStartX - dstStartX;
					memcpy(dst + dstY * pitch + clippedStartX,
						lineData + srcSkipX, clippedEndX - clippedStartX);
				}
			}

			lineData += readableLen;
			srcX += copyLen;
			if (readableLen < copyLen)
				break;
		}
		src += lineSize + 2;
	}
}

/**
 * RA1 codec 23: Additive line-update overlay (FUN_10B40).
 */
void smushDecodeRA1AdditiveLineUpdate(byte *dst, const byte *src, int left, int top, int width, int height,
		int pitch, int bufWidth, int bufHeight, uint8 paletteBase) {
	const uint8 colorDelta = (uint8)(paletteBase - 0x30);
	for (int row = 0; row < height; row++) {
		const uint16 lineSize = READ_LE_UINT16(src);
		const byte *lineData = src + 2;
		const byte *lineEnd = lineData + lineSize;
		const int dstY = top + row;
		int srcX = 0;
		while (srcX < width && lineData < lineEnd) {
			const int skip = *lineData++; srcX += skip;
			if (srcX >= width || lineData >= lineEnd) break;
			const int runLength = (int)(*lineData++);
			const int dstStartX = left + srcX;
			const int dstEndX = dstStartX + runLength;
			if (dstY >= 0 && dstY < bufHeight) {
				const int clippedStartX = MAX(dstStartX, 0);
				const int clippedEndX = MIN(dstEndX, bufWidth);
				if (clippedStartX < clippedEndX) {
					byte *dstPixel = dst + dstY * pitch + clippedStartX;
					for (int x = clippedStartX; x < clippedEndX; x++, dstPixel++)
						*dstPixel = (byte)(*dstPixel + colorDelta);
				}
			}
			srcX += runLength;
		}
		src += lineSize + 2;
	}
}

/**
 * RA1 codec 2: Scatter/point draw (FUN_110D7).
 */
void smushDecodeRA1Scatter(byte *dst, const byte *src, int left, int top, int bufWidth, int bufHeight, int pitch, int dataSize) {
	int curX = left;
	int curY = top;
	while (dataSize >= 4) {
		int16 dx = (int16)READ_LE_UINT16(src);
		uint8 dy = src[2];
		uint8 pixel = src[3];
		src += 4; dataSize -= 4;
		curX += dx; curY += dy;
		if (curX >= 0 && curY >= 0 && curX < bufWidth && curY < bufHeight)
			dst[curY * pitch + curX] = pixel;
	}
}

// RA1 codec 4/5: block-based dithered codec with 4x4 tile lookup tables
static uint8 s_ra1C4Tbl[2][256][16];
static uint16 s_ra1C4Param = 0xFFFF;

static void ra1Codec4GenTiles(uint16 param1) {
	uint8 *dst = &s_ra1C4Tbl[0][0][0];
	for (int i = 1; i < 16; i += 2) {
		for (int k = 0; k < 16; k++) {
			int j = i + param1, l = k + param1;
			int m = (j + l) / 2, n = (j + m) / 2, o = (l + m) / 2;
			if (j == m || l == m) {
				*dst++ = l; *dst++ = j; *dst++ = l; *dst++ = j;
				*dst++ = j; *dst++ = l; *dst++ = j; *dst++ = j;
				*dst++ = l; *dst++ = j; *dst++ = l; *dst++ = j;
				*dst++ = l; *dst++ = l; *dst++ = j; *dst++ = l;
			} else {
				*dst++ = m; *dst++ = m; *dst++ = n; *dst++ = j;
				*dst++ = m; *dst++ = m; *dst++ = n; *dst++ = j;
				*dst++ = o; *dst++ = o; *dst++ = m; *dst++ = n;
				*dst++ = l; *dst++ = l; *dst++ = o; *dst++ = m;
			}
		}
	}
	for (int i = 0; i < 16; i += 2) {
		for (int k = 0; k < 16; k++) {
			int j = i + param1, l = k + param1;
			int m = (j + l) / 2, n = (j + m) / 2, o = (l + m) / 2;
			if (m == j || m == l) {
				*dst++ = j; *dst++ = j; *dst++ = l; *dst++ = j;
				*dst++ = j; *dst++ = j; *dst++ = j; *dst++ = l;
				*dst++ = l; *dst++ = j; *dst++ = l; *dst++ = l;
				*dst++ = j; *dst++ = l; *dst++ = j; *dst++ = l;
			} else {
				*dst++ = j; *dst++ = j; *dst++ = n; *dst++ = m;
				*dst++ = j; *dst++ = j; *dst++ = n; *dst++ = m;
				*dst++ = n; *dst++ = n; *dst++ = m; *dst++ = o;
				*dst++ = m; *dst++ = m; *dst++ = o; *dst++ = l;
			}
		}
	}
}

static bool ra1Codec4LoadTiles(const byte *&src, int &remaining, uint16 param2, uint8 clr) {
	uint8 *dst = &s_ra1C4Tbl[1][0][0];
	int loop = param2 * 8;
	if (param2 > 256 || remaining < loop)
		return false;
	for (int i = 0; i < loop; i++) {
		byte c = *src++;
		remaining--;
		*dst++ = (c >> 4) + clr;
		*dst++ = (c & 0xF) + clr;
	}
	return true;
}

void smushDecodeRA1Block(byte *dst, const byte *src, int left, int top, int width, int height,
						 int pitch, int bufHeight, int dataSize, uint8 param, uint16 parm2, int codec) {
	const int mx = pitch;
	const int my = bufHeight;
	if (s_ra1C4Param != param) {
		ra1Codec4GenTiles(param);
		s_ra1C4Param = param;
	}
	int remaining = dataSize;
	const byte *data = src;
	if (parm2 > 0) {
		if (!ra1Codec4LoadTiles(data, remaining, parm2, param)) {
			warning("smushDecodeRA1Block: not enough data for tile load (parm2=%d)", parm2);
			return;
		}
	}
	for (int j = 0; j < width; j += 4) {
		byte mask = 0, bits = 0;
		int x = left + j;
		for (int i = 0; i < height; i += 4) {
			int y = top + i;
			int bit = 0;
			if (parm2 > 0) {
				if (bits == 0) {
					if (remaining < 1) return;
					mask = *data++; remaining--; bits = 8;
				}
				bit = !!(mask & 0x80);
				mask <<= 1; bits--;
			}
			if (remaining < 1) return;
			byte idx = *data++; remaining--;
			if (bit == 0 && idx == 0x80 && codec != 5)
				continue;
			if (y >= my || (y + 4) < 0 || (x + 4) < 0 || x >= mx)
				continue;
			const byte *gs = &s_ra1C4Tbl[bit][idx][0];
			if (y >= 0 && x >= 0 && (y + 4) <= my && (x + 4) <= mx) {
				for (int k = 0; k < 4; k++, gs += 4)
					memcpy(dst + x + (y + k) * pitch, gs, 4);
			} else {
				for (int k = 0; k < 4; k++)
					for (int l = 0; l < 4; l++, gs++) {
						int yo = y + k, xo = x + l;
						if (yo >= 0 && yo < my && xo >= 0 && xo < mx)
							*(dst + yo * pitch + xo) = *gs;
					}
			}
		}
	}
}

bool SmushPlayerRebel1::handleGameFrameBufferSelect(int codec, int width, int height) {
	// RA1 sub-fullscreen frames render into _specialBuffer at their (left, top) offset position.
	int bufSize = 384 * 242;
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
		if (_width == 0 || _height == 0) {
			_width = 384;
			_height = 242;
		}
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
		smushDecodeRA1Transparent(_dst, src, left, top, width, height, pitch);
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
			pitch, (_dst == _specialBuffer) ? _height : _vm->_screenHeight);
		return true;
	case SMUSH_CODEC_SKIP_RLE: {
		const int bufWidth = pitch;
		const int bufHeight = (_dst == _specialBuffer) ? _height : _vm->_screenHeight;
		// Codec 23 uses the high byte of the FOBJ codec word as the palette
		// band for its additive delta. The event-mask path may subtract 0x10
		// from this value before decoding, which Level 8 uses for the walker
		// armor layers.
		smushDecodeRA1AdditiveLineUpdate(_dst, src, left, top, width, height,
			pitch, bufWidth, bufHeight, param);
		return true;
	}
	default:
		debugC(DEBUG_SMUSH, "SmushPlayer::decodeFrameObject: Skipping unknown codec %d (left=%d, top=%d, %dx%d)",
			codec, left, top, width, height);
		return true;
	}
}

bool SmushPlayerRebel1::handleGameStoreFrame() {
	// RA1 handles STOR via handleGameFrameObjectPost
	return true;
}

void SmushPlayerRebel1::handleGameFrameObjectPre(int codec, int left, int top, int width, int height, int dataSize) {
	debug("RA1 FOBJ: frame=%d codec=%d pos=(%d,%d) size=%dx%d dataSize=%d storeFrame=%d",
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
			free(chunk_buffer);
			return;
		}
	}

	decodeFrameObject(codec, chunk_buffer, left, top, width, height, chunk_size, ra1Param, ra1Parm2);
	free(chunk_buffer);
}

// ---------------------------------------------------------------------------
// handleFrame override — RA1 frame parsing with alignment, OBJ chunks, clean frame
// ---------------------------------------------------------------------------

static bool ra1FrameHasGameChunk(Common::SeekableReadStream &b, int32 frameSize) {
	const int64 frameStart = b.pos();
	int32 remaining = frameSize;

	while (remaining > 1) {
		if ((b.pos() & 1) && remaining > 0) {
			const byte pad = b.readByte();
			if (pad == 0) {
				remaining--;
			} else {
				b.seek(-1, SEEK_CUR);
			}
		}
		if (remaining < 8)
			break;

		const uint32 subType = b.readUint32BE();
		const int32 subSize = b.readUint32BE();
		const int64 subDataPos = b.pos();

		if (subType == MKTAG('F', 'R', 'M', 'E'))
			break;
		if (subType == MKTAG('G', 'A', 'M', 'E')) {
			b.seek(frameStart, SEEK_SET);
			return true;
		}

		remaining -= subSize + 8;
		b.seek(subDataPos + subSize, SEEK_SET);
	}

	b.seek(frameStart, SEEK_SET);
	return false;
}

void SmushPlayerRebel1::handleFrame(int32 frameSize, Common::SeekableReadStream &b) {
	debugC(DEBUG_SMUSH, "SmushPlayerRebel1::handleFrame(%d)", _frame);
	uint8 *audioChunk = nullptr;
	_skipNext = false;
	handleGameFrameStart();

	bool preserveFrameHistory = false;
	if (_insane) {
		InsaneRebel1 *rebel1 = static_cast<InsaneRebel1 *>(_insane);
		bool interactive = rebel1->isInteractiveVideoActive();
		const uint16 activeOpcode = rebel1->getActiveGameOpcode();
		bool forceClear = interactive &&
			(activeOpcode == 0x0B ||
			 (activeOpcode == 0 && rebel1->getCurrentLevel() == 1));

		preserveFrameHistory = interactive && !forceClear &&
			ra1FrameHasGameChunk(b, frameSize);
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

	while (frameSize > 0) {
		// RA1 exits when <=1 byte remains
		if (frameSize <= 1) {
			if (frameSize == 1)
				b.skip(1);
			break;
		}

		// RA1 top-of-loop alignment (FUN_1FDBC)
		if ((b.pos() & 1) && frameSize > 0) {
			byte peek = b.readByte();
			if (peek == 0) {
				frameSize--;
			} else {
				b.seek(-1, SEEK_CUR);
			}
		}

		if (frameSize < 8) {
			b.skip(frameSize);
			break;
		}

		uint32 subType = b.readUint32BE();
		int32 subSize = b.readUint32BE();
		int32 subOffset = b.pos();

		// Guard against consuming next frame marker
		if (subType == MKTAG('F','R','M','E')) {
			b.seek(-8, SEEK_CUR);
			break;
		}

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
			if (!_compressedFileMode && !isFastForwardingCurrentFrame()) {
				audioChunk = (uint8 *)malloc(subSize + 8);
				b.seek(-8, SEEK_CUR);
				b.read(audioChunk, subSize + 8);
				feedAudio(audioChunk, 0, 127, 0, 0);
				free(audioChunk);
				audioChunk = nullptr;
			}
			break;
		case MKTAG('T','R','E','S'):
		case MKTAG('T','E','X','T'):
			handleTextResource(subType, subSize, b);
			break;
		case MKTAG('X','P','A','L'):
			handleDeltaPalette(subSize, b);
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
		case MKTAG('G','A','M','E'): {
			InsaneRebel1 *rebel1 = (InsaneRebel1 *)_insane;
			rebel1->handleGameChunk(subSize, b);
			break;
		}
		case MKTAG('O','B','J','\0'): {
			// RA1 object overlay chunk: variable-size header + embedded FOBJ/GAME/PSAD.
			int32 objDataSize = frameSize - 8;
			if (objDataSize > 0) {
				byte *objBuf = (byte *)malloc(objDataSize);
				b.read(objBuf, objDataSize);

				int32 objPos = 0;
				while (objPos + 8 < objDataSize) {
					uint32 embTag = READ_BE_UINT32(objBuf + objPos);
					uint32 embSize = READ_BE_UINT32(objBuf + objPos + 4);
					int32 embRemaining = objDataSize - objPos - 8;

					bool recognized = (embTag == MKTAG('F','O','B','J') ||
					                   embTag == MKTAG('G','A','M','E') ||
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
					} else if (embTag == MKTAG('G','A','M','E')) {
						Common::MemoryReadStream embStream(objBuf + objPos + 8, embSize);
						InsaneRebel1 *rebel1 = (InsaneRebel1 *)_insane;
						rebel1->handleGameChunk(embSize, embStream);
					} else if (embTag == MKTAG('P','S','A','D')) {
						if (!_compressedFileMode && !isFastForwardingCurrentFrame()) {
							uint8 *audioBuf = (uint8 *)malloc(embSize + 8);
							memcpy(audioBuf, objBuf + objPos, embSize + 8);
							feedAudio(audioBuf, 0, 127, 0, 0);
							free(audioBuf);
						}
					}

					objPos += 8 + embSize;
					if (embSize & 1)
						objPos++;
				}
				free(objBuf);
			}
			frameSize = 0;
			continue;
		}
		case MKTAG('G','A','M','2'):
		case MKTAG('F','A','D','E'):
		case MKTAG('S','E','G','A'):
		case MKTAG('A','D','L',' '):
		case MKTAG('A','D','L','2'):
		case MKTAG('S','B','L',' '):
		case MKTAG('S','B','L','2'):
		case MKTAG('P','S','D','2'):
			debugC(DEBUG_SMUSH, "SmushPlayerRebel1::handleFrame: skipping chunk %s (%d bytes)", tag2str(subType), subSize);
			break;
		default: {
			// Original FUN_1FDBC: unknown uppercase tag → silently stop
			byte tb0 = (subType >> 24) & 0xFF, tb1 = (subType >> 16) & 0xFF;
			byte tb2 = (subType >> 8) & 0xFF, tb3 = subType & 0xFF;
			if (tb0 > 0x40 && tb0 < 0x5B && tb1 > 0x40 && tb1 < 0x5B &&
			    tb2 > 0x40 && tb2 < 0x5B && tb3 > 0x40 && tb3 < 0x5B) {
				debug(5, "RA1: unknown uppercase tag %s at frame %d, stopping frame parse", tag2str(subType), _frame);
				frameSize = 0;
				continue;
			}
			error("Unknown frame subChunk found : %s, %d", tag2str(subType), subSize);
		}
		}

		frameSize -= subSize + 8;
		b.seek(subOffset + subSize, SEEK_SET);
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

	if (_insanity)
		_insane->procPostRendering(_dst, 0, 0, 0, _frame, _nbframes-1);

	if (_width != 0 && _height != 0)
		updateScreen();

	_frame++;
}

// ---------------------------------------------------------------------------
// handleGameUpdateScreen — RA1 viewport-aware screen blit
// ---------------------------------------------------------------------------

void SmushPlayerRebel1::handleGameUpdateScreen(const byte *src, int srcPitch, int width, int height) {
	if (_dst == nullptr || _width <= 0 || _height <= 0)
		return;

	if (!_insane || !static_cast<InsaneRebel1 *>(_insane)->isInteractiveVideoActive() ||
			_vm->_screenWidth != kRA1PresentationScreenWidth ||
			_vm->_screenHeight != kRA1PresentationScreenHeight) {
		SmushPlayer::handleGameUpdateScreen(src, srcPitch, width, height);
		return;
	}

	int ra1ViewX = _ra1ViewportOffsetX;
	int ra1ViewY = _ra1ViewportOffsetY;

	const int srcX = CLIP(_scrollX + ra1ViewX + kRA1PresentationBorder, 0, _width - 1);
	const int srcY = CLIP(_scrollY + ra1ViewY + kRA1PresentationBorder, 0, _height - 1);

	int frameWidth = MIN<int>(_width - srcX, kRA1PresentationWidth);
	int frameHeight = MIN<int>(_height - srcY, kRA1PresentationHeight);
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
	const byte *dst = _dst + srcY * _width + srcX;
	byte *presentationDst = _ra1PresentationBuffer +
		kRA1PresentationBorder * kRA1PresentationScreenWidth + kRA1PresentationBorder;
	for (int y = 0; y < frameHeight; y++) {
		memcpy(presentationDst + y * kRA1PresentationScreenWidth,
			dst + y * _width, frameWidth);
	}

	SmushPlayer::handleGameUpdateScreen(_ra1PresentationBuffer,
		kRA1PresentationScreenWidth, kRA1PresentationScreenWidth, kRA1PresentationScreenHeight);
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
