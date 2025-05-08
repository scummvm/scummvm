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
 */

#include "image/image_decoder.h"

#include "graphics/wincursor.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/cursor.h"
#include "director/movie.h"
#include "director/castmember/bitmap.h"
#include "director/picture.h"
#include "director/lingo/lingo-code.h"

namespace Director {

Cursor::Cursor() {
	_keyColor = 0xff;

	_cursorResId = Datum(0);
	_cursorType = Graphics::kMacCursorArrow;

	_usePalette = false;
}

CursorRef Cursor::getRef() {
	CursorRef res;
	res._cursorType = _cursorType;
	res._cursorResId = _cursorResId;
	return res;
}

bool Cursor::operator==(const Cursor &c) {
	return _cursorType == c._cursorType &&
		c._cursorResId == _cursorResId;
}

bool Cursor::operator==(const CursorRef &c) {
	return _cursorType == c._cursorType &&
			c._cursorResId == _cursorResId;
}

void Cursor::readFromCast(Datum cursorCasts) {
	if (cursorCasts.type != ARRAY || cursorCasts.u.farr->arr.size() < 1) {
		warning("Cursor::readFromCast: Needs array of at least 1");
		return;
	}
	if (_cursorResId.type == ARRAY && LC::eqData(_cursorResId, cursorCasts).asInt())
		return;


	CastMemberID cursorId = cursorCasts.u.farr->arr[0].asMemberID();
	CastMember *cursorCast = g_director->getCurrentMovie()->getCastMember(cursorId);
	if (!cursorCast || cursorCast->_type != kCastBitmap) {
		warning("Cursor::readFromCast: No bitmap cast for cursor");
		return;
	}

	CastMember *maskCast = nullptr;
	CastMemberID maskId;
	if (cursorCasts.u.farr->arr.size() > 1) {
		maskId = cursorCasts.u.farr->arr[1].asMemberID();
		maskCast = g_director->getCurrentMovie()->getCastMember(maskId);
		if (!maskCast || maskCast->_type != kCastBitmap) {
			warning("Cursor::readFromCast: Invalid bitmap mask for cursor, ignoring");
			maskCast = nullptr;
		}
	}

	debugC(2, kDebugImages, "Cursor::readFromCast: setting cursor: %s, mask: %s", cursorId.asString().c_str(), maskId.asString().c_str());

	_usePalette = false;
	_keyColor = 3;

	Datum cursorRes;
	if (g_director->getVersion() < 500) {
		cursorRes = Datum(cursorId.member);
	} else {
		cursorRes = Datum((cursorId.castLib << 16) + cursorId.member);
	}
	resetCursor(Graphics::kMacCursorCustom, true, cursorRes);

	Graphics::Surface *cursorSurface = &((BitmapCastMember *)cursorCast)->_picture->_surface;
	Graphics::Surface *maskSurface = nullptr;
	if (maskCast) {
		maskSurface = &((BitmapCastMember *)maskCast)->_picture->_surface;
	}

	_surface = new byte[getWidth() * getHeight()];
	byte *dst = _surface;

	for (int y = 0; y < 16; y++) {
		const byte *cursor = nullptr, *mask = nullptr;

		if (y < cursorSurface->h &&
				(!maskSurface || y < maskSurface->h)) {
			cursor = (const byte *)cursorSurface->getBasePtr(0, y);
			if (maskSurface)
				mask = (const byte *)maskSurface->getBasePtr(0, y);
		}

		for (int x = 0; x < 16; x++) {
			if (x >= cursorSurface->w ||
					(!maskSurface || x >= maskSurface->w)) {
				cursor = mask = nullptr;
			}

			if (!cursor) {
				*dst = 3;
			} else {
				*dst = (!mask || *mask) ? (*cursor ? 0 : 1) : 3;
				cursor++;
				if (mask)
					mask++;
			}
			dst++;
		}
	}

	BitmapCastMember *bc = (BitmapCastMember *)(cursorCast);
	int offX = bc->_regX - bc->_initialRect.left;
	int offY = bc->_regY - bc->_initialRect.top;
	if ((offX < 0) || (offX >= 16) || (offY < 0) || (offY >= 16) ||
		(g_director->getVersion() < 500 && g_director->getPlatform() == Common::kPlatformWindows)) {
		// Hotspots that are outside the 16x16 crop will be recentered in the middle.
		// Pre-5 versions of Windows Director do not respect hotspots at all?
		offX = 8;
		offY = 8;
	}
	_hotspotX = (uint16)offX;
	_hotspotY = (uint16)offY;
}

void Cursor::readBuiltinType(Datum resourceId) {
	if (resourceId.equalTo(_cursorResId))
		return;

	if (resourceId.type != INT) {
		warning("readBuiltinType: failed to read cursor. It's not type int");
	}


	switch(resourceId.asInt()) {
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
		warning("Cursor::readBuiltinType failed to read cursor %d", resourceId.asInt());
		break;
	}
}

void Cursor::readFromResource(Datum resourceId) {
	if (resourceId == _cursorResId)
		return;

	if (resourceId.type != INT) {
		warning("Cursor:readFromResource is not of type INT");
		return;
	}
	switch(resourceId.asInt()) {
	case -1:
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 200:
		readBuiltinType(resourceId);
		break;
	default:
		bool readSuccessful = false;

		Cast *cast = g_director->getCurrentMovie()->getCast();
		if (cast && cast->getArchive()) {
			readSuccessful = readFromArchive(cast->getArchive(), resourceId.asInt());
			if (readSuccessful)
				break;
		}

		for (auto &it : g_director->_allOpenResFiles) {
			readSuccessful = readFromArchive(g_director->_allSeenResFiles[it], resourceId.asInt());
			if (readSuccessful)
				break;
		}

		// Cursors can be located in the main archive, which may not
		// be in _allOpenResFiles
		if (!readSuccessful && g_director->getPlatform() == Common::kPlatformMacintosh) {
			readSuccessful = readFromArchive(g_director->getMainArchive(), resourceId.asInt());
		}

		// TODO: figure out where to read custom cursor in windows platform
		// currently, let's just set arrow for default one.
		if (g_director->getPlatform() == Common::kPlatformWindows) {
			resetCursor(Graphics::kMacCursorArrow, true, resourceId);
			break;
		}

		// for win platform, try the cursor from exe
		if (!readSuccessful && g_director->getPlatform() == Common::kPlatformWindows) {
			// i'm not sure, in jman we have cursor id 2, 3, 4. and custom cursor id 128 129 130
			uint id = (resourceId.asInt() & 0x7f) + 2;
			for (uint i = 0; i < g_director->_winCursor.size(); i++) {
				for (uint j = 0; j < g_director->_winCursor[i]->cursors.size(); j++) {
					if (id == g_director->_winCursor[i]->cursors[j].id.getID()) {
						resetCursor(Graphics::kMacCursorCustom, false, Datum((int)id));
						readSuccessful = true;
					}
				}
			}
		}

		// fallback method. try to use builtin cursor by regarding resourceId as a single byte.
		if (!readSuccessful)
			readBuiltinType(resourceId.asInt() & 0x7f);
	}
}

bool Cursor::readFromArchive(Archive *archive, uint16 resourceId) {
	bool readSuccessful = false;
	Common::SeekableReadStreamEndian *cursorStream = nullptr;
	if (archive->hasResource(MKTAG('C', 'U', 'R', 'S'), resourceId))
		cursorStream = archive->getResource(MKTAG('C', 'U', 'R', 'S'), resourceId);

	if (!cursorStream && archive->hasResource(MKTAG('C', 'R', 'S', 'R'), resourceId))
		cursorStream = archive->getResource(MKTAG('C', 'R', 'S', 'R'), resourceId);

	if (cursorStream && readFromStream(*((Common::SeekableReadStream *)cursorStream), false, 0)) {
		_usePalette = true;
		_keyColor = 0xff;
		_cursorType = Graphics::kMacCursorCustom;
		_cursorResId = resourceId;
		readSuccessful = true;
	}
	delete cursorStream;
	return readSuccessful;
}

void Cursor::resetCursor(Graphics::MacCursorType type, bool shouldClear, Datum resId) {
	if (shouldClear)
		clear();

	_cursorType = type;
	if (_cursorType != Graphics::kMacCursorCustom)
		_usePalette = false;

	_cursorResId = resId;

	_hotspotX = 0;
	_hotspotY = 0;
}

CursorRef::CursorRef() {
	_cursorType = Graphics::kMacCursorArrow;
	_cursorResId = Datum(0);
}

bool CursorRef::operator==(const Cursor &c) {
	return _cursorType == c._cursorType &&
		c._cursorResId == _cursorResId;
}

bool CursorRef::operator==(const CursorRef &c) {
	return _cursorType == c._cursorType &&
		c._cursorResId == _cursorResId;
}

} // End of namespace Director
