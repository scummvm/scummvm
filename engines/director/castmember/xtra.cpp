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

#include "director/director.h"
#include "director/cast.h"
#include "director/movie.h"
#include "director/castmember/xtra.h"
#include "director/lingo/lingo-the.h"

namespace Director {

XtraCastMember::XtraCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId, stream) {
	_type = kCastXtra;

	if (debugChannelSet(5, kDebugLoading)) {
		stream.hexdump(stream.size());
	}

	warning("STUB: XtraCastMember::XtraCastMember(): Xtra cast members not yet supported for version v%d (%d)", humanVersion(_cast->_version), _cast->_version);
}

XtraCastMember::XtraCastMember(Cast *cast, uint16 castId, XtraCastMember &source)
		: CastMember(cast, castId) {
}

bool XtraCastMember::hasField(int field) {
	switch (field) {
	case kTheCuePointNames:		// D6
	case kTheCuePointTimes:		// D6
	case kTheCurrentTime:		// D6
	case kTheMediaBusy:			// D6, undocumented
		return true;
	default:
		break;
	}
	return CastMember::hasField(field);
}

Datum XtraCastMember::getField(int field) {
	Datum d;

	switch (field) {
	default:
		d = CastMember::getField(field);
		break;
	}

	return d;
}

void XtraCastMember::setField(int field, const Datum &d) {
	switch (field) {
	default:
		break;
	}

	CastMember::setField(field, d);
}

Common::String XtraCastMember::formatInfo() {
	return Common::String::format("Xtra");
}

uint32 XtraCastMember::getCastDataSize() {
	warning("XtraCastMember()::getCastDataSize(): CastMember version invalid or not handled");
	return 0;
}

void XtraCastMember::writeCastData(Common::SeekableWriteStream *writeStream) {
	warning("XtraCastMember()::writeCastData(): CastMember version invalid or not handled");
}

} // End of namespace Director
