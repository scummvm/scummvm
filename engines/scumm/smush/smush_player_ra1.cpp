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

#include "scumm/file.h"
#include "scumm/scumm_v7.h"
#include "scumm/smush/smush_font.h"
#include "scumm/smush/smush_player.h"

#include "scumm/insane/insane_rebel1.h"

namespace Scumm {

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

void SmushPlayerRebel1::destroyGamePlayerFields() {
	free(_ra1ObjOverlayData);
	_ra1ObjOverlayData = nullptr;
	_ra1ObjOverlayDataSize = 0;
}

void SmushPlayerRebel1::resetGameVideoState() {
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

void SmushPlayer::ra1HandleGost(int32 subSize, Common::SeekableReadStream &b) {
	if (subSize < 12) {
		warning("SmushPlayer::ra1HandleGost: chunk too small (%d bytes)", subSize);
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
void smushDecodeRA1Transparent(byte *dst, const byte *src, int left, int top, int width, int height, int pitch);
void smushDecodeRLEOpaque(byte *dst, const byte *src, int left, int top, int width, int height, int pitch);
void smushDecodeRA1SkipCopy(byte *dst, const byte *src, int left, int top, int width, int height, int pitch);
void smushDecodeRA1AdditiveLineUpdate(byte *dst, const byte *src, int left, int top, int width, int height,
	int pitch, int bufWidth, int bufHeight, uint8 param);
void smushDecodeRA1Scatter(byte *dst, const byte *src, int left, int top, int bufWidth, int bufHeight, int pitch, int dataSize);
void smushDecodeRA1Block(byte *dst, const byte *src, int left, int top, int width, int height, int pitch, int dataSize, uint8 param, uint16 parm2, int codec);

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
	// RA1 additive codec (SKIP_RLE) uses original coords, not adjusted
	if (codec == SMUSH_CODEC_SKIP_RLE)
		return false;
	ra2AdjustFrameCoords(left, top, width, height, pitch, srcSkipY);
	return true;
}

bool SmushPlayerRebel1::handleGameCodecDecode(int codec, const uint8 *src, int left, int top, int width, int height, int pitch, int dataSize) {
	// The base class passes clipped coords. For additive codec and scatter, we need original coords
	// which are stored in the origLeft/origTop locals of decodeFrameObject. Since we can't access those
	// from the override, the additive codec and scatter draw are special-cased.
	// For now, handle the codecs that have RA1-specific behavior.
	switch (codec) {
	case SMUSH_CODEC_RLE:
		smushDecodeRA1Transparent(_dst, src, left, top, width, height, pitch);
		return true;
	case SMUSH_CODEC_RLE_ALT:
		smushDecodeRLEOpaque(_dst, src, left, top, width, height, pitch);
		return true;
	case SMUSH_CODEC_RA1_SCATTER:
		smushDecodeRA1Scatter(_dst, src, left, top, _width, _height, pitch, dataSize);
		return true;
	case SMUSH_CODEC_RA1_DELTA:
	case SMUSH_CODEC_RA1_BLOCK:
		smushDecodeRA1Block(_dst, src, left, top, width, height, pitch, dataSize, 0, 0, codec);
		return true;
	case SMUSH_CODEC_LINE_UPDATE:
		smushDecodeRA1SkipCopy(_dst, src, left, top, width, height, pitch);
		return true;
	case SMUSH_CODEC_SKIP_RLE: {
		const int bufWidth = pitch;
		const int bufHeight = (_dst == _specialBuffer) ? _height : _vm->_screenHeight;
		smushDecodeRA1AdditiveLineUpdate(_dst, src, left, top, width, height,
			pitch, bufWidth, bufHeight, 0);
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
	ra2RememberLastFobj(codec, data, dataSize, left, top, width, height);
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

} // End of namespace Scumm
