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

// Default implementation to write the cast members as they are
void CastMember::writeToFile(Common::MemoryWriteStream *writeStream, uint32 offset, uint32 version, uint32 castID) {
	writeStream->seek(offset);
	writeStream->writeUint16LE(MKTAG('C', 'A', 'S', 't'));
	
	uint32 castDataToWrite = getDataSize();
	uint32 castInfoToWrite = getInfoSize();
	uint32 castDataOffset = 0;
	uint32 castInfoOffset = 0;
	Common::SeekableReadStreamEndian *stream = _cast->getResource(MKTAG('C', 'A', 'S', 't'), castID);

	if (version >= kFileVer400 && version < kFileVer500) {
		writeStream->writeUint16LE(castDataToWrite);
		writeStream->writeUint32LE(castInfoToWrite);
		castDataOffset += 6;

		writeStream->writeUint32LE(_castType);
		castDataToWrite -= 1;
		
		if (_flags1 != -1) {
			writeStream->writeByte(_flags1);
			castDataToWrite -= 1;
			castDataOffset += 1;
		}
	} else if (version >= kFileVer500 && version < kFileVer600) {
		writeStream->writeUint32LE(_castType);
		writeStream->writeUint32LE(getInfoSize());
		writeStream->writeUint32LE(getDataSize());
	}


	byte *data = (byte *)calloc(castDataToWrite, 1);
	byte *info = (byte *)calloc(castInfoToWrite, 1);
	stream->seek(castDataOffset, SEEK_CUR);
	stream->read(data, castDataToWrite);
	stream->read(info, castInfoToWrite);

	Common::MemoryReadStreamEndian *dataStream = new Common::MemoryReadStreamEndian(data, castDataToWrite, stream->isBE());
	Common::MemoryReadStreamEndian *infoStream = new Common::MemoryReadStreamEndian(info, castInfoToWrite, stream->isBE());
	
	if (version >= kFileVer400 && version < kFileVer500) {
		writeStream->writeStream(dataStream);
		writeStream->writeStream(infoStream);
	} else if (version >= kFileVer500 && version < kFileVer600) {
		writeStream->writeStream(infoStream);
		writeStream->writeStream(dataStream);
	}
}

uint32 CastMember::getDataSize() {
	debug("CastMember::getDataSize(): Defualt implementation of cast member data size, returning original size.");
	return _castDataSize;
}

uint32 CastMember::getInfoSize() {
	debug("CastMember::getInfoSize(): Defualt implementation of cast member info size, returning original size.");
	return _castInfoSize;
}

} // End of namespace Director
