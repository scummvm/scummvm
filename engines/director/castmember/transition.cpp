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

Common::String TransitionCastMember::formatInfo() {
	return Common::String::format("transType: %d, durationMillis: %d, flags: %d, chunkSize: %d", _transType, _durationMillis, _flags, _chunkSize);
}

} // End of namespace Director
