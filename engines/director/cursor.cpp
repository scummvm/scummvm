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

#include "director/director.h"
#include "director/cursor.h"
#include "director/movie.h"
#include "director/castmember.h"

namespace Director {

Cursor::Cursor() {
	_cursorResId = 0;
	_cursorType = Graphics::kMacCursorArrow;

	_cursorCastId = 0;
	_cursorMaskId = 0;
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

	if (!cursorCast || !cursorCast->_widget || cursorCast->_type != kCastBitmap) {
		warning("Cursor::readFromCast: No bitmap cast for cursor");
		return;
	} else if (!maskCast || !maskCast->_widget || maskCast->_type != kCastBitmap) {
		warning("Cursor::readFromCast: No bitmap mask for cursor");
		return;
	}

	_cursorType = Graphics::kMacCursorCustom;
	_cursorResId = 0;
	_cursorCastId = cursorId;
	_cursorMaskId = maskId;

	clear();
	_surface = new byte[getWidth() * getHeight()];
	byte *dst = _surface;

	for (int y = 0; y < 16; y++) {
		const byte *cursor = nullptr, *mask = nullptr;

		if (y < cursorCast->_widget->getSurface()->h &&
				y < maskCast->_widget->getSurface()->h) {
			cursor = (const byte *)cursorCast->_widget->getSurface()->getBasePtr(0, y);
			mask = (const byte *)maskCast->_widget->getSurface()->getBasePtr(0, y);
		}

		for (int x = 0; x < 16; x++) {
			if (x >= cursorCast->_widget->getSurface()->w ||
					x >= maskCast->_widget->getSurface()->w) {
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
	_hotspotX = bc->_initialRect.left - bc->_regX;
	_hotspotY = bc->_initialRect.top - bc->_regY;
}

void Cursor::readFromResource(int resourceId) {
	if (resourceId == _cursorResId)
		return;

	clear();

	_cursorCastId = 0;
	_cursorMaskId = 0;

	_cursorResId = resourceId;

	switch(resourceId) {
	case -1:
		_cursorType = Graphics::kMacCursorArrow;
		break;
	case 1:
		_cursorType = Graphics::kMacCursorBeam;
		break;
	case 2:
		_cursorType = Graphics::kMacCursorCrossHair;
		break;
	case 3:
		_cursorType = Graphics::kMacCursorCrossBar;
		break;
	case 4:
		_cursorType = Graphics::kMacCursorWatch;
		break;
	case 200:
		_cursorType = Graphics::kMacCursorOff;
		break;
	default:
		_cursorType = Graphics::kMacCursorCustom;

		for (Common::HashMap<Common::String, Archive *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>::iterator it = g_director->_openResFiles.begin(); it != g_director->_openResFiles.end(); ++it) {
			Common::SeekableSubReadStreamEndian *cursorStream;

			cursorStream = ((MacArchive *)it->_value)->getResource(MKTAG('C', 'U', 'R', 'S'), resourceId);
			if (!cursorStream)
				cursorStream = ((MacArchive *)it->_value)->getResource(MKTAG('C', 'R', 'S', 'R'), resourceId);

			if (cursorStream) {
				readFromStream(*((Common::SeekableReadStream *)cursorStream), false, 0);
				break;
			}
		}
	}

}

} // end of namespace Director
