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
#include "director/castmember/script.h"

namespace Director {

ScriptCastMember::ScriptCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId, stream) {
	_type = kCastLingoScript;
	_scriptType = kNoneScript;

	if (debugChannelSet(5, kDebugLoading)) {
		debugC(5, kDebugLoading, "ScriptCastMember::ScriptCastMember(): Contents");
		stream.hexdump(stream.size());
	}

	if (version < kFileVer400) {
		error("Unhandled Script cast");
	} else if (version >= kFileVer400 && version < kFileVer600) {
		byte unk1 = stream.readByte();
		byte type = stream.readByte();

		switch (type) {
		case 1:
			_scriptType = kScoreScript;
			break;
		case 3:
			_scriptType = kMovieScript;
			break;
		case 7:
			_scriptType = kParentScript;
			warning("Unhandled kParentScript %d", castId);
			break;
		default:
			error("ScriptCastMember: Unprocessed script type: %d", type);
		}

		debugC(3, kDebugLoading, "CASt: Script type: %s (%d), unk1: %d", scriptType2str(_scriptType), type, unk1);

		assert(stream.pos() == stream.size()); // There should be no more data
	} else {
		warning("STUB: ScriptCastMember::ScriptCastMember(): Scripts not yet supported for version %d", version);
	}
}

ScriptCastMember::ScriptCastMember(Cast *cast, uint16 castId, ScriptCastMember &source)
	: CastMember(cast, castId) {
	_type = kCastLingoScript;
	_scriptType = source._scriptType;
	warning("ScriptCastMember(): Duplicating source %d to target %d! This is unlikely to work properly, as the actual scripts aren't yet copied", source._castId, castId);
}

Common::String ScriptCastMember::formatInfo() {
	return Common::String::format(
		"scriptType: %s", scriptType2str(_scriptType)
	);
}

}
