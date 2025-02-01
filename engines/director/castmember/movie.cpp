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
#include "director/movie.h"
#include "director/castmember/movie.h"
#include "director/lingo/lingo-the.h"

namespace Director {

MovieCastMember::MovieCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId, stream) {
	_type = kCastMovie;

	_initialRect = Movie::readRect(stream);
	_flags = stream.readUint32();

	_looping = !(_flags & 0x20);
	_enableScripts = _flags & 0x10;
	_enableSound = _flags & 0x08;
	_crop = !(_flags & 0x02);
	_center = _flags & 0x01;

	if (debugChannelSet(2, kDebugLoading))
		_initialRect.debugPrint(2, "MovieCastMember(): rect:");
	debugC(2, kDebugLoading, "MovieCastMember(): flags: (%d 0x%04x)", _flags, _flags);
	debugC(2, kDebugLoading, "_looping: %d, _enableScripts %d, _enableSound: %d, _crop %d, _center: %d",
			_looping, _enableScripts, _enableSound, _crop, _center);

}

MovieCastMember::MovieCastMember(Cast *cast, uint16 castId, MovieCastMember &source)
	: CastMember(cast, castId) {
	_type = kCastMovie;
	_loaded = source._loaded;

	_initialRect = source._initialRect;
	_boundingRect = source._boundingRect;
	_children = source._children;

	_flags = source._flags;
	_looping = source._looping;
	_enableScripts = source._enableScripts;
	_enableSound = source._enableSound;
	_crop = source._crop;
	_center = source._center;
}

bool MovieCastMember::hasField(int field) {
	switch (field) {
	case kTheCenter:
	case kTheIdleHandlerPeriod:
	case kThePaletteMapping:
	case kTheScoreSelection:
	case kTheScriptsEnabled:
	case kTheSound:
	case kTheUpdateLock:
		return true;
	default:
		break;
	}
	return CastMember::hasField(field);
}

Datum MovieCastMember::getField(int field) {
	Datum d;

	switch (field) {
	case kTheCenter:
		d = Datum((int)_center);
		break;
	case kTheIdleHandlerPeriod:
		warning("STUB: MovieCastMember::getField(): idleHandlerPeriod not implemented");
		break;
	case kThePaletteMapping:
		warning("STUB: MovieCastMember::getField(): paletteMapping not implemented");
		break;
	case kTheScoreSelection:
		warning("STUB: MovieCastMember::getField(): scoreSelection not implemented");
		break;
	case kTheScriptsEnabled:
		d = Datum(_enableScripts);
		break;
	case kTheSound:
		d = Datum(_enableSound);
		break;
	case kTheUpdateLock:
		warning("STUB: MovieCastMember::getField(): updateLock not implemented");
		break;
	default:
		d = CastMember::getField(field);
		break;
	}

	return d;
}

bool MovieCastMember::setField(int field, const Datum &d) {
	switch (field) {
	case kTheCenter:
		_center = (bool)d.asInt();
		return true;
	case kTheIdleHandlerPeriod:
		warning("STUB: MovieCastMember::setField(): idleHandlerPeriod not implemented");
		return false;
	case kThePaletteMapping:
		warning("STUB: MovieCastMember::setField(): paletteMapping not implemented");
		return false;
	case kTheScoreSelection:
		warning("STUB: MovieCastMember::setField(): scoreSelection not implemented");
		return false;
	case kTheScriptsEnabled:
		_enableScripts = (bool)d.asInt();
		return true;
	case kTheSound:
		_enableSound = (bool)d.asInt();
		return true;
	case kTheUpdateLock:
		warning("STUB: MovieCastMember::setField(): updateLock not implemented");
		return false;
	default:
		break;
	}

	return CastMember::setField(field, d);
}

Common::String MovieCastMember::formatInfo() {
	return Common::String::format(
		"initialRect: %dx%d@%d,%d, boundingRect: %dx%d@%d,%d, enableScripts: %d, enableSound: %d, looping: %d, crop: %d, center: %d",
		_initialRect.width(), _initialRect.height(),
		_initialRect.left, _initialRect.top,
		_boundingRect.width(), _boundingRect.height(),
		_boundingRect.left, _boundingRect.top,
		_enableScripts, _enableSound, _looping,
		_crop, _center
	);
}

} // End of namespace Director
