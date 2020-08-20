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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "image/image_decoder.h"

#include "director/director.h"
#include "director/cursor.h"
#include "director/movie.h"
#include "director/castmember.h"

namespace Director {

Cursor::Cursor() {
	_keyColor = 0xff;

	_cursorResId = 0;
	_cursorType = Graphics::kMacCursorArrow;

	_cursorCastId = 0;
	_cursorMaskId = 0;

	_usePalette = false;
}

bool Cursor::operator==(const Cursor &c) {
	return _cursorType == c._cursorType &&
		_cursorResId == c._cursorResId &&
		_cursorCastId == c._cursorCastId &&
		_cursorMaskId == c._cursorMaskId;
}

void Cursor::readFromCast(uint cursorId, uint maskId) {
	if (cursorId == _cursorCastId && maskId == _cursorMaskId)
		return;

	CastMember *cursorCast = g_director->getCurrentMovie()->getCastMember(cursorId);
	CastMember *maskCast = g_director->getCurrentMovie()->getCastMember(maskId);

	if (!cursorCast || cursorCast->_type != kCastBitmap) {
		warning("Cursor::readFromCast: No bitmap cast for cursor");
		return;
	} else if (!maskCast || maskCast->_type != kCastBitmap) {
		warning("Cursor::readFromCast: No bitmap mask for cursor");
		return;
	}

	_usePalette = false;
	_keyColor = 3;

	resetCursor(Graphics::kMacCursorCustom, true, 0, cursorId, maskId);

	BitmapCastMember *cursorBitmap = (BitmapCastMember *)cursorCast;
	BitmapCastMember *maskBitmap = (BitmapCastMember *)maskCast;

	_surface = new byte[getWidth() * getHeight()];
	byte *dst = _surface;

	for (int y = 0; y < 16; y++) {
		const byte *cursor = nullptr, *mask = nullptr;

		if (y < cursorBitmap->_img->getSurface()->h &&
				y < maskBitmap->_img->getSurface()->h) {
			cursor = (const byte *)cursorBitmap->_img->getSurface()->getBasePtr(0, y);
			mask = (const byte *)maskBitmap->_img->getSurface()->getBasePtr(0, y);
		}

		for (int x = 0; x < 16; x++) {
			if (x >= cursorBitmap->_img->getSurface()->w ||
					x >= maskBitmap->_img->getSurface()->w) {
				cursor = mask = nullptr;
			}

			if (!cursor) {
				*dst = 3;
			} else {
				*dst = *mask ? 3 : (*cursor ? 1 : 0);
				cursor++;
				mask++;
			}
			dst++;
		}
	}

	BitmapCastMember *bc = (BitmapCastMember *)(cursorCast);
	_hotspotX = bc->_regX - bc->_initialRect.left;
	_hotspotY = bc->_regY - bc->_initialRect.top;
}

void Cursor::readFromResource(int resourceId) {
	if (resourceId == _cursorResId)
		return;

	switch(resourceId) {
	case -1:
	case 0:
		resetCursor(Graphics::kMacCursorArrow, true, resourceId);
		break;
	case 1:
		resetCursor(Graphics::kMacCursorBeam, true, resourceId);
		break;
	case 2:
		resetCursor(Graphics::kMacCursorCrossHair, true, resourceId);
		break;
	case 3:
		resetCursor(Graphics::kMacCursorCrossBar, true, resourceId);
		break;
	case 4:
		resetCursor(Graphics::kMacCursorWatch, true, resourceId);
		break;
	case 200:
		resetCursor(Graphics::kMacCursorOff, true, resourceId);
		break;
	default:
		_usePalette = true;
		_keyColor = 0xff;

		for (Common::HashMap<Common::String, Archive *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>::iterator it = g_director->_openResFiles.begin(); it != g_director->_openResFiles.end(); ++it) {
			Common::SeekableReadStreamEndian *cursorStream;

			cursorStream = ((MacArchive *)it->_value)->getResource(MKTAG('C', 'U', 'R', 'S'), resourceId);
			if (!cursorStream)
				cursorStream = ((MacArchive *)it->_value)->getResource(MKTAG('C', 'R', 'S', 'R'), resourceId);

			if (cursorStream && readFromStream(*((Common::SeekableReadStream *)cursorStream), false, 0)) {
				resetCursor(Graphics::kMacCursorCustom, false, resourceId);
				break;
			}
		}
	}
}

void Cursor::resetCursor(Graphics::MacCursorType type, bool shouldClear, int resId, uint castId, uint maskId) {
	if (shouldClear)
		clear();

	_cursorType = type;
	if (_cursorType != Graphics::kMacCursorCustom)
		_usePalette = false;

	_cursorResId = resId;

	_cursorCastId = castId;
	_cursorMaskId = maskId;

	_hotspotX = 0;
	_hotspotY = 0;
}

} // End of namespace Director
