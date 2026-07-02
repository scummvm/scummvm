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

	CastMemberInfo *ci = getInfo();

	if (ci && ci->isExternal) {
		warning("STUB: XtraCastMember::XtraCastMember(): External Xtra cast members not yet supported for version v%d (%d)", humanVersion(_cast->_version), _cast->_version);
	} else {
		uint32 symbolLen = stream.readUint32BE();
		_xtraSymbol = stream.readString(0, symbolLen);

		// TODO: Lookup synmbol

		uint32 xtraDataLen = stream.readUint32BE();
		xtraDataLen = MIN<int>(xtraDataLen, (int)(stream.size() - stream.pos()));
		_xtraData = Common::Array<byte>(xtraDataLen);
		stream.read(_xtraData.data(), xtraDataLen);

		debugC(3, kDebugLoading, "  XtraCastMember: xtraSymbol: '%s', xtraDataLen: %d", _xtraSymbol.c_str(), xtraDataLen);

		if (debugChannelSet(5, kDebugLoading)) {
			Common::hexdump(_xtraData.data(), xtraDataLen);
		}

		// TODO: Process data in the Xtra
	}

	// Director 7+ stores QuickTime videos as "quickTimeMedia" Xtra cast
	// members, rich text fields as "text" Xtra cast members, and embedded
	// fonts as "font" Xtra cast members (the Font Asset / Font Xtra, holding
	// a PFR1 TrueDoc font). QuickTime and text Xtras are promoted to dedicated
	// cast members in Cast::loadCastData(); font Xtras are handled gracefully
	// by the font subsystem (the embedded font is substituted, as ScummVM has
	// no PFR decoder). Don't warn about any of these being unsupported here.
	if (!isQuickTimeVideo() && !isTextXtra() && !isFontXtra())
		warning("STUB: XtraCastMember::XtraCastMember(): Xtra cast members not yet supported for version v%d (%d)", humanVersion(_cast->_version), _cast->_version);
}

XtraCastMember::XtraCastMember(Cast *cast, uint16 castId, XtraCastMember &source)
		: CastMember(cast, castId) {
}

bool XtraCastMember::isQuickTimeVideo() const {
	// QuickTime Asset Xtra ("QuickTime 3" / "QTASSET"). In D7+ this is how
	// linked .mov digital videos are represented in the cast.
	return _xtraSymbol.equalsIgnoreCase("quickTimeMedia");
}

bool XtraCastMember::isQuickTimeLooping() const {
	// The quickTimeMedia Asset Xtra payload carries the QuickTime member's
	// playback flags. Byte 7 holds a bitfield in which 0x40 is the "loop"
	// flag (verified empirically against Physikus: the looping logo sample
	// Maschine.mov has it set, the play-once Trailer.mov has it clear).
	// Short videos authored as ambient loops rely on this to keep repeating
	// while a longer concurrent sound plays and the score holds the frame.
	return isQuickTimeVideo() && _xtraData.size() > 7 && (_xtraData[7] & 0x40);
}

bool XtraCastMember::isTextXtra() const {
	// "Text" Asset Xtra. In D7+ this is how editable rich text fields are
	// represented in the cast; the displayed string is stored in an XMED child.
	return _xtraSymbol.equalsIgnoreCase("text");
}

bool XtraCastMember::isFontXtra() const {
	// "Font Asset" / "Font Xtra". In D7+ this is how embedded fonts are stored
	// in the cast; the payload wraps a PFR1 (Bitstream TrueDoc) font in an XMED
	// child. ScummVM has no PFR decoder, so the font itself is not rendered;
	// text that references it is drawn with a substitute font instead.
	return _xtraSymbol.equalsIgnoreCase("font");
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
