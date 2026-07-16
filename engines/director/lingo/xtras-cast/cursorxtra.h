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

#ifndef DIRECTOR_LINGO_XTRAS_CAST_CURSORXTRA_H
#define DIRECTOR_LINGO_XTRAS_CAST_CURSORXTRA_H

#include "common/array.h"
#include "common/rect.h"

#include "director/castmember/castmember.h"

namespace Director {

class XtraCastMember;

namespace CursorXtra {

struct Info {
	Common::Point hotspot;
	uint32 sizePx = 0;
	uint32 intervalMs = 0;
	uint32 member = 0;		// cursor bitmap cast member
	uint32 mask = 0;		// mask bitmap cast member
	uint32 frameCount = 0;
};

CastMember *createCastMember(Cast *cast, uint16 castId, XtraCastMember *xtra);
bool parseXtraData(const Common::Array<byte> &data, Info &info);

} // End of namespace CursorXtra

// "Cursor" Asset Xtra cast member (D6+): a custom (possibly animated)
// cursor built from bitmap cast members.
class CursorXtraCastMember : public CastMember {
public:
	CursorXtraCastMember(Cast *cast, uint16 castId, XtraCastMember &source);
	CursorXtraCastMember(Cast *cast, uint16 castId, CursorXtraCastMember &source);

	CastMember *duplicate(Cast *cast, uint16 castId) override { return (CastMember *)(new CursorXtraCastMember(cast, castId, *this)); }

	bool getCursorInfo(CastMemberID &image, CastMemberID &mask);

	Common::String formatInfo() override;

	CursorXtra::Info _info;
	bool _valid = false;
};

} // End of namespace Director

#endif
