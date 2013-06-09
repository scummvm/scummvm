/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "fullpipe/fullpipe.h"

#include "common/file.h"

#include "fullpipe/utils.h"
#include "fullpipe/objects.h"

namespace Fullpipe {

char *MfcArchive::readPascalString(bool twoByte) {
	char *tmp;
	int len;

	if (twoByte)
		len = readUint16LE();
	else
		len = readByte();

	tmp = (char *)calloc(len + 1, 1);
	read(tmp, len);

	debug(0, "readPascalString: %d <%s>", len, tmp);

	return tmp;
}

int MfcArchive::readCount() {
	int count = readUint16LE();

	if (count == 0xffff)
		count = readUint32LE();

	return count;
}

enum {
	kNullObject = 0,
	kCInteraction = 1,
	kMessageQueue = 2,
	kExCommand = 3,
	kCObjstateCommand = 4,
	kCGameVar = 5
};

const struct {
	const char *name;
	int id;
} classMap[] = {
	{ "CInteraction",	kCInteraction },
	{ "MessageQueue",	kMessageQueue },
	{ "ExCommand",		kExCommand },
	{ "CObjstateCommand", kCObjstateCommand },
	{ "CGameVar",		kCGameVar },
	{ 0, 0 }
};

static const char *lookupObjectId(int id) {
	for (int i = 0; classMap[i].name; i++) {
		if (classMap[i].id == id)
			return classMap[i].name;
	}

	return "";
}


MfcArchive::MfcArchive() {
	for (int i = 0; classMap[i].name; i++) {
		_classMap[classMap[i].name] = classMap[i].id;
	}

	_lastIndex = 1;

	_objectMap.push_back(kNullObject);
}

CObject *MfcArchive::readClass() {
	CObject *res = parseClass();

	if (res)
		res->load(*this);

	return res;
}

CObject *MfcArchive::parseClass() {
	char *name;
	int objectId;

	uint obTag = readUint16LE();

	debug(0, "parseClass::obTag = %d (%04x)  at 0x%08x", obTag, obTag, pos() - 2);

	if (obTag == 0xffff) {
		int schema = readUint16LE();

		debug(0, "parseClass::schema = %d", schema);

		name = readPascalString(true);
		debug(0, "parseClass::class <%s>", name);

		if (!_classMap.contains(name)) {
			error("Unknown class in MfcArchive: <%s>", name);
		}

		objectId = _classMap[name];
		_objectMap.push_back(objectId);
		debug(0, "tag: %d (%x)", _objectMap.size() - 1, objectId);

		objectId = _classMap[name];
	} else if ((obTag & 0x8000) == 0) {
		objectId = _objectMap[obTag];
	} else {

		obTag &= ~0x8000;

		if (_objectMap.size() < obTag) {
			error("Object index too big: %d  at 0x%08x", obTag, pos() - 2);
		}

		debug(0, "parseClass::obTag <%s>", lookupObjectId(_objectMap[obTag]));

		objectId = _objectMap[obTag];
	}
	
	_objectMap.push_back(objectId);

	debug(0, "objectId: %d", objectId);

	switch (objectId) {
	case kNullObject:
		warning("parseClass: NULL object  at 0x%08x", pos() - 2);
		return 0;
	case kCInteraction:
		return new CInteraction();
	case kMessageQueue:
		return new MessageQueue();
	case kExCommand:
		return new ExCommand();
	case kCObjstateCommand:
		return new CObjstateCommand();
	case kCGameVar:
		return new CGameVar();
	default:
		error("Unknown objectId: %d", objectId);
	}

	return 0;
}

} // End of namespace Fullpipe
