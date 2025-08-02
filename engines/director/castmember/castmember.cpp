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

#include "common/events.h"
#include "common/substream.h"
#include "common/macresman.h"
#include "common/memstream.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/movie.h"
#include "director/castmember/castmember.h"
#include "director/lingo/lingo-the.h"

namespace Director {

CastMember::CastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream) : Object<CastMember>("CastMember") {
	_type = kCastTypeNull;
	_cast = cast;
	_castId = castId;
	_hilite = false;
	_purgePriority = 3;
	_size = stream.size();
	_flags1 = 0;

	_loaded = false;
	_modified = true;
	_isChanged = false;
	_needsReload = false;

	_objType = kCastMemberObj;

	_widget = nullptr;
	_erase = false;
}

CastMember::CastMember(Cast *cast, uint16 castId) : Object<CastMember>("CastMember") {
	_type = kCastTypeNull;
	_cast = cast;
	_castId = castId;
	_hilite = false;
	_purgePriority = 3;
	_size = 0;
	_flags1 = 0;

	_loaded = false;
	_modified = true;
	_isChanged = false;

	_objType = kCastMemberObj;

	_widget = nullptr;
	_erase = false;
}

CastMember *CastMember::duplicate(Cast *cast, uint16 castId) {
	warning("CastMember::duplicate(): unsupported cast type %s", castType2str(_type));
	return nullptr;
}

void CastMember::setModified(bool modified) {
	_modified = modified;
	if (modified)
		_isChanged = true;
}

Common::Rect CastMember::getBbox() {
	Common::Rect result(_initialRect);
	Common::Point offset = getRegistrationOffset();
	result.moveTo(-offset.x, -offset.y);
	return result;
}

Common::Rect CastMember::getBbox(int16 currentWidth, int16 currentHeight) {
	Common::Rect result(currentWidth, currentHeight);
	Common::Point offset = getRegistrationOffset(currentWidth, currentHeight);
	result.moveTo(-offset.x, -offset.y);
	return result;
}

bool CastMember::hasProp(const Common::String &propName) {
	Common::String fieldName = Common::String::format("%d%s", kTheCast, propName.c_str());
	return g_lingo->_theEntityFields.contains(fieldName) && hasField(g_lingo->_theEntityFields[fieldName]->field);
}

Datum CastMember::getProp(const Common::String &propName) {
	Common::String fieldName = Common::String::format("%d%s", kTheCast, propName.c_str());
	if (g_lingo->_theEntityFields.contains(fieldName)) {
		return getField(g_lingo->_theEntityFields[fieldName]->field);
	}

	warning("CastMember::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

bool CastMember::setProp(const Common::String &propName, const Datum &value, bool force) {
	Common::String fieldName = Common::String::format("%d%s", kTheCast, propName.c_str());
	if (g_lingo->_theEntityFields.contains(fieldName)) {
		return setField(g_lingo->_theEntityFields[fieldName]->field, value);
	}

	warning("CastMember::setProp: unknown property '%s'", propName.c_str());
	return false;
}

bool CastMember::hasField(int field) {
	switch (field) {
	case kTheBackColor:
	case kTheCastLibNum:
	case kTheCastType:
	case kTheFileName:
	case kTheForeColor:
	case kTheHeight:
	case kTheHilite:
	case kTheLoaded:
	case kTheModified:
	case kTheMemberNum:
	case kTheName:
	case kTheNumber:
	case kTheRect:
	case kThePreLoad:
	case kThePurgePriority:
	case kTheScriptText:
	case kTheSize:
	case kTheType:
	case kTheWidth:
		return true;
	default:
		break;
	}
	return false;
}

Datum CastMember::getField(int field) {
	Datum d;

	CastMemberInfo *castInfo = _cast->getCastMemberInfo(_castId);
	if (!castInfo)
		warning("CastMember::getField(): CastMember info for %d not found", _castId);

	switch (field) {
	case kTheBackColor:
		d = (int)getBackColor();
		break;
	case kTheCastLibNum:
		d = _cast->_castLibID;
		break;
	case kTheCastType:
	case kTheType:
		d = Common::String(castType2str(_type));
		if (g_director->getVersion() >= 500 && _type == kCastText) {
			// D5 changes this from "text" to "field"
			d = Common::String("field");
		}
		d.type = SYMBOL;
		break;
	case kTheFileName:
		if (castInfo)
			d = Datum(castInfo->directory + g_director->_dirSeparator + castInfo->fileName);
		break;
	case kTheForeColor:
		d = (int)getForeColor();
		break;
	case kTheHeight:
		d = _cast->getCastMemberInitialRect(_castId).height();
		break;
	case kTheHilite:
		d = (int)_hilite;
		break;
	case kTheLoaded:
		d = 1; // Not loaded handled in Lingo::getTheCast
		break;
	case kTheModified:
		d = (int)_isChanged;
		break;
	case kTheName:
		if (castInfo)
			d = Datum(castInfo->name);
		break;
	case kTheMemberNum:
		d = _castId;
		break;
	case kTheNumber:
		if (g_director->getVersion() >= 500) {
			d = CastMemberID(_castId, _cast->_castLibID).toMultiplex();
		} else {
			d = _castId;
		}
		break;
	case kTheRect:
		// not sure get the initial rect would be fine to castmember
		d = Datum(_cast->getCastMember(_castId)->_initialRect);
		break;
	case kThePreLoad:
		d = 0; // ScummVM does not do preloading
		break;
	case kThePurgePriority:
		d = _purgePriority;
		break;
	case kTheScriptText:
		if (castInfo)
			d = Datum(castInfo->script);
		break;
	case kTheSize:
		d = (int)_size;
		break;
	case kTheWidth:
		d = _cast->getCastMemberInitialRect(_castId).width();
		break;
	default:
		warning("CastMember::getField(): Unprocessed getting field \"%s\" of cast %d", g_lingo->field2str(field), _castId);
	//TODO find out about String fields
	}

	return d;
}

bool CastMember::setField(int field, const Datum &d) {
	CastMemberInfo *castInfo = _cast->getCastMemberInfo(_castId);

	switch (field) {
	case kTheBackColor:
		_cast->getCastMember(_castId)->setBackColor(d.asInt());
		return true;
	case kTheCastType:
	case kTheType:
		warning("BUILDBOT: CastMember::setField(): Attempt to set read-only field %s of cast %d", g_lingo->entity2str(field), _castId);
		return false;
	case kTheFileName:
		if (!castInfo) {
			warning("CastMember::setField(): CastMember info for %d not found", _castId);
			return false;
		}
		castInfo->fileName = d.asString();
		_needsReload = true;
		return true;
	case kTheForeColor:
		_cast->getCastMember(_castId)->setForeColor(d.asInt());
		return true;
	case kTheHeight:
		warning("BUILDBOT: CastMember::setField(): Attempt to set read-only field \"%s\" of cast %d", g_lingo->field2str(field), _castId);
		return false;
	case kTheHilite:
		_hilite = (bool)d.asInt();
		_modified = true;
		return true;
	case kTheName:
		if (!castInfo) {
			warning("CastMember::setField(): CastMember info for %d not found", _castId);
			return false;
		}
		castInfo->name = d.asString();
		_cast->rebuildCastNameCache();
		return true;
	case kTheRect:
		warning("CastMember::setField(): Attempt to set read-only field \"%s\" of cast %d", g_lingo->field2str(field), _castId);
		return false;
	case kThePreLoad:
		warning("CastMember::setField(): Attempt to set preLoad field to %d but ScummVM does not do preloading", d.asInt());
		return false;
	case kThePurgePriority:
		_purgePriority = CLIP<int>(d.asInt(), 0, 3);
		return true;
	case kTheScriptText:
		if (!castInfo) {
			warning("CastMember::setField(): CastMember info for %d not found", _castId);
			return false;
		}
		_cast->_lingoArchive->replaceCode(*d.u.s, kCastScript, _castId);
		castInfo->script = d.asString();
		return true;
	case kTheWidth:
		warning("BUILDBOT: CastMember::setField(): Attempt to set read-only field \"%s\" of cast %d", g_lingo->field2str(field), _castId);
		return false;
	default:
		warning("CastMember::setField(): Unprocessed setting field \"%s\" of cast %d", g_lingo->field2str(field), _castId);
	}

	return false;
}

CastMemberInfo *CastMember::getInfo() {
	return _cast->getCastMemberInfo(_castId);
}

void CastMember::load() {
	if (_loaded)
		return;

	_loaded = true;
}

void CastMember::unload() {
	if (!_loaded)
		return;

	_loaded = false;
}

// Cast members have two types of "information", one is _data_ and the other is _info_
// _data_ is the actual "data" (e.g. the pixel image data of a bitmap, sound data of a sound cast member)
// Whereas _info_ is metadata (size, name, flags, etc.)
// Some cast members have their _data_ as well as _info_ in this very 'CASt' resource, e.g. TextCastMember
// Whereas some other have their _info_ in a 'CASt' resource and _data_ in a dedicated resource (e.g. PaletteCastMember has 'CLUT' resource)
uint32 CastMember::writeCAStResource(Common::MemoryWriteStream *writeStream, uint32 offset) {
	uint32 castResourceSize = getCastResourceSize();

	writeStream->writeUint32LE(MKTAG('C', 'A', 'S', 't'));
	writeStream->writeUint32LE(castResourceSize);		// this is excluding the 'CASt' header and the size itself (- 8 bytes)

	uint32 castDataToWrite = getCastDataSize();
	uint32 castInfoToWrite = getCastInfoSize();

	if (_cast->_version >= kFileVer400 && _cast->_version < kFileVer500) {
		writeStream->writeUint16BE(castDataToWrite);
		writeStream->writeUint32BE(castInfoToWrite);
		writeStream->writeByte((uint8)_type);
		castDataToWrite -= 1;

		if (_flags1 != 0xFF) {					// In case of TextCastMember, this should be true
			writeStream->writeByte(_flags1);
			castDataToWrite -= 1;
		}

		// For cast members with dedicated resources for data, the castDataToWrite is zero
		if (castDataToWrite) {
			writeCastData(writeStream);
		}

		if (castInfoToWrite) {
			_cast->writeCastInfo(writeStream, _castId);
		}
	} else if (_cast->_version >= kFileVer500 && _cast->_version < kFileVer600) {
		writeStream->writeUint32BE((uint32)_type);
		writeStream->writeUint32BE(castInfoToWrite);
		writeStream->writeUint32BE(castDataToWrite);

		if (castInfoToWrite) {
			_cast->writeCastInfo(writeStream, _castId);
		}

		// For cast members with dedicated resources for data, the castDataToWrite is zero
		if (castDataToWrite) {
			writeCastData(writeStream);
		}
	}
	return 0;
}

// This is the data that is inside the 'CASt' resource
// These functions (getCastDataSize() and writeCastData() default implementations, are not supposed to be called
// If the data is modified in the cast member, we implement a custom getCastDataSize() and writeCastData() for that member
// If it is not modified, then we write it as it is from the original source in the overridden
// writeCAStResource(Common::MemoryWriteStream, uint32, uint32) function which doesn't call these default functions
uint32 CastMember::getCastDataSize() {
	warning("CastMember::getDataSize(): Defualt implementation of 'CASt' resource data size");
	return _castDataSize;
}

void CastMember::writeCastData(Common::MemoryWriteStream *writeStream) {
	warning("CastMember::getDataSize(): Defualt implementation of 'CASt' resource data");

	if (_cast->_version >= kFileVer400 && _cast->_version < kFileVer500) {
		if (_flags1 != 0xFF) {
			writeStream->write(0, _castDataSize - 2);
		} else {
			writeStream->write(0, _castDataSize - 1);
		}
	} else {
		writeStream->write(0, _castDataSize);
	}
}

// This is the info that is inside the 'CASt' resource
uint32 CastMember::getCastInfoSize() {
	return _cast->getCastInfoSize(_castId);
}

// getCastResourceSize only returns the size of the resource without the header
uint32 CastMember::getCastResourceSize() {
	uint32 headerSize = 0;

	if (_cast->_version >= kFileVer400 && _cast->_version < kFileVer500) {
		// Header size for director version 4
		headerSize = 7;			// see Cast::loadCastData() for director version 4
		if (_flags1 != 0xFF) {
			headerSize += 1;
		}
	} else if (_cast->_version >= kFileVer500 && _cast->_version < kFileVer600) {
		// Header size for director version 5
		headerSize = 12;		// See Cast::loadCastData() for director version 5
	}

	return headerSize + getCastInfoSize() + getCastDataSize();
}

} // End of namespace Director
