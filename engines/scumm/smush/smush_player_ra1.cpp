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
				if (rebel1->getActiveGameOpcode() == 0x0B && _storedFobjWidth == _vm->_screenWidth) {
					left += _ra1ViewportOffsetX;
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
	_dst = _specialBuffer;
	return true;
}

} // End of namespace Scumm
