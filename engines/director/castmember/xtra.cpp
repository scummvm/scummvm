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

#include "common/hash-str.h"
#include "common/hashmap.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/movie.h"
#include "director/castmember/xtra.h"
#include "director/castmember/digitalvideo.h"
#include "director/lingo/lingo-the.h"
#include "director/lingo/xtras-cast/cursorxtra.h"
#include "director/lingo/xtras-cast/textxtra.h"

namespace Director {

struct XtraCastMemberProto {
	const char *symbol;
	CastMember *(*promote)(Cast *cast, uint16 castId, XtraCastMember *xtra);
};

static const XtraCastMemberProto xtraCastMemberProtos[] = {
	{ "cursor", CursorXtra::createCastMember },
	{ "quickTimeMedia", DigitalVideoCastMember::createFromXtra },
	{ "text", TextXtra::createCastMember },
	{ "font", nullptr },
	{ nullptr, nullptr },
};

static const XtraCastMemberProto *findXtraCastMemberProto(const Common::String &symbol) {
	static Common::HashMap<Common::String, const XtraCastMemberProto *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> map;

	if (map.empty()) {
		for (const XtraCastMemberProto *p = xtraCastMemberProtos; p->symbol; p++)
			map.setVal(p->symbol, p);
	}

	if (map.contains(symbol))
		return map.getVal(symbol);
	return nullptr;
}

XtraCastMember::XtraCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId, stream) {
	_type = kCastXtra;

	if (debugChannelSet(5, kDebugLoading)) {
		stream.hexdump(stream.size());
	}

	CastMemberInfo *ci = getInfo();

	if (ci && ci->isExternal) {
		warning("STUB: XtraCastMember::XtraCastMember(): External Xtra cast members not yet supported for version v%d (%d)", humanVersion(_cast->_version), _cast->_version);
	} else {
		uint32 symbolLen = stream.readUint32BE();
		_xtraSymbol = stream.readString(0, symbolLen);
		uint32 xtraDataLen = stream.readUint32BE();
		xtraDataLen = MIN<int>(xtraDataLen, (int)(stream.size() - stream.pos()));
		_xtraData = Common::Array<byte>(xtraDataLen);
		stream.read(_xtraData.data(), xtraDataLen);

		debugC(3, kDebugLoading, "  XtraCastMember: xtraSymbol: '%s', xtraDataLen: %d", _xtraSymbol.c_str(), xtraDataLen);

		if (debugChannelSet(5, kDebugLoading)) {
			Common::hexdump(_xtraData.data(), xtraDataLen);
		}
	}

	if (!findXtraCastMemberProto(_xtraSymbol))
		warning("STUB: XtraCastMember::XtraCastMember(): Xtra cast members not yet supported for version v%d (%d)", humanVersion(_cast->_version), _cast->_version);
}

XtraCastMember::XtraCastMember(Cast *cast, uint16 castId, XtraCastMember &source)
		: CastMember(cast, castId) {
}

CastMember *XtraCastMember::promote(Cast *cast, uint16 castId, XtraCastMember *xtra) {
	// Lookup the Xtra and instantiate it if registered
	const XtraCastMemberProto *p = findXtraCastMemberProto(xtra->_xtraSymbol);
	if (!p || !p->promote)
		return xtra;
	CastMember *promoted = p->promote(cast, castId, xtra);
	delete xtra;
	return promoted;
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
