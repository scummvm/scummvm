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
#include "director/castmember/transition.h"
#include "director/lingo/lingo-the.h"

namespace Director {

TransitionCastMember::TransitionCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId, stream) {
	_type = kCastTransition;

	_transType = kTransNone;
	_durationMillis = 0;
	_chunkSize = 0;
	_area = false;

	if (debugChannelSet(5, kDebugLoading)) {
		stream.hexdump(stream.size());
	}
	if (_cast->_version < kFileVer600) {
		stream.readByte();
		_chunkSize = stream.readByte();
		_transType = static_cast<TransitionType>(stream.readByte());
		_flags = stream.readByte();
		_area = !(_flags & 1);
		_durationMillis = stream.readUint16BE();
		debugC(5, kDebugLoading, "TransitionCastMember::TransitionCastMember(): transType: %d, durationMillis: %d, flags: %d, chunkSize: %d", _transType, _durationMillis, _flags, _chunkSize);
	} else {
		warning("STUB: TransitionCastMember::TransitionCastMember(): Transitions not yet supported for version %d", _cast->_version);
	}
}

TransitionCastMember::TransitionCastMember(Cast *cast, uint16 castId, TransitionCastMember &source)
		: CastMember(cast, castId) {
	_transType = source._transType;
	_loaded = source._loaded;

	_durationMillis = source._durationMillis;
	_flags = source._flags;
	_chunkSize = source._chunkSize;
	_area = source._area;
}

bool TransitionCastMember::hasField(int field) {
	switch (field) {
	case kTheDuration:
		return true;
	default:
		break;
	}
	return CastMember::hasField(field);
}

Datum TransitionCastMember::getField(int field) {
	Datum d;

	switch (field) {
	case kTheDuration:
		d = Datum(_durationMillis);
		break;
	default:
		d = CastMember::getField(field);
		break;
	}

	return d;
}

bool TransitionCastMember::setField(int field, const Datum &d) {
	switch (field) {
	case kTheDuration:
		_durationMillis = (bool)d.asInt();
		return true;
	default:
		break;
	}

	return CastMember::setField(field, d);
}

Common::String TransitionCastMember::formatInfo() {
	return Common::String::format("transType: %d, durationMillis: %d, flags: %d, chunkSize: %d", _transType, _durationMillis, _flags, _chunkSize);
}

} // End of namespace Director
