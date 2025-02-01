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
#include "director/lingo/lingo-the.h"

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

bool ScriptCastMember::hasField(int field) {
	switch (field) {
	case kTheScriptType:
		return true;
	default:
		break;
	}
	return CastMember::hasField(field);
}

Datum ScriptCastMember::getField(int field) {
	Datum d;

	switch (field) {
	case kTheScriptType:
		switch (_scriptType) {
		case kMovieScript:
			d = Common::String("movie");
			d.type = SYMBOL;
			break;
		case kScoreScript:
			d = Common::String("score");
			d.type = SYMBOL;
			break;
		case kParentScript:
			d = Common::String("parent");
			d.type = SYMBOL;
			break;
		default:
			break;
		}
		break;
	default:
		d = CastMember::getField(field);
		break;
	}

	return d;
}

bool ScriptCastMember::setField(int field, const Datum &d) {
	switch (field) {
	case kTheScriptType:
		warning("ScriptCastMember::setField(): setting scriptType! This probably isn't going to work as it doesn't recategorize the script.");
		if (d.type == SYMBOL) {
			if (d.u.s->equalsIgnoreCase("movie")) {
				_scriptType = kMovieScript;
			} else if (d.u.s->equalsIgnoreCase("score")) {
				_scriptType = kScoreScript;
			} else if (d.u.s->equalsIgnoreCase("parent")) {
				_scriptType = kParentScript;
			}
		}
		return true;
		break;
	default:
		break;
	}

	return CastMember::setField(field, d);
}

Common::String ScriptCastMember::formatInfo() {
	return Common::String::format(
		"scriptType: %s", scriptType2str(_scriptType)
	);
}

}
